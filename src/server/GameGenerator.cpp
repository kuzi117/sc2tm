#include "server/GameGenerator.h"

#include "common/config.h"

#include <algorithm>
#include <cassert>

sc2tm::GameGenerator::GameGenerator(const SHAFileMap &botMap, const SHAFileMap &mapMap) {
  for (auto it : botMap)
    bots.insert(it.second);

  for (auto it : mapMap)
    maps.insert(it.second);
}

sc2tm::GameGenerator::Matchup::Matchup(SHA256Hash::ptr b0, SHA256Hash::ptr b1) :
    bot0(SHA256Hash::compare(b0, b1) < 0 ? b0 : b1),
    bot1(SHA256Hash::compare(b0, b1) < 0 ? b1 : b0) { }

bool sc2tm::GameGenerator::Matchup::contains(const SHA256Hash::ptr bot) const {
  return SHA256Hash::compare(bot, bot0) == 0 || SHA256Hash::compare(bot, bot1) == 0;
}

uint8_t sc2tm::GameGenerator::Matchup::indexOf(const SHA256Hash::ptr bot) const {
  assert(contains(bot));
  if (SHA256Hash::compare(bot, bot0) == 0)
    return 0;
  return 1;
}


// TODO We need to lock this when multithreading happens
bool sc2tm::GameGenerator::generateGame(Game &game, HashSet cBots, HashSet cMaps){
  // Get the bots and maps that the client and us have in common
  // Use temporary scopes to destroy the extra hash sets we make during intersection and subtraction
  {
    // Bots
    HashSet botInter;
    std::set_intersection(bots.begin(), bots.end(),
                          cBots.begin(), cBots.end(),
                          std::inserter(botInter, botInter.end()),
                          CompareHashPtrFtor());

    HashSet unfinishedBots;
    std::set_difference(botInter.begin(), botInter.end(),
                        finishedBots.begin(), finishedBots.end(),
                        std::inserter(unfinishedBots, unfinishedBots.end()),
                        CompareHashPtrFtor());

    // Assign over the input set
    cBots = unfinishedBots;
  }

  {
    // Maps
    HashSet mapInter;
    std::set_intersection(maps.begin(), maps.end(),
                          cMaps.begin(), cMaps.end(),
                          std::inserter(mapInter, mapInter.end()),
                          CompareHashPtrFtor());

    // Assign over the input set
    cMaps = mapInter;
  }

  // If there's not enough bots for a matchup or a single map to play on then there's no games
  // to give out for this client.
  if (cBots.size() < 2 || cMaps.empty())
    return false;

  // Try to find a matchup in the active matches from our list of common bots
  if (generateActiveMap(game, cBots, cMaps))
    return true;

  // Well we didn't find an already active matchup that this client could participate in, so we'll
  // try scheduling a new map for an existing matchup.
  if (generateActiveMatchup(game, cBots, cMaps))
    return true;

  // Couldn't find an existing matchup and new map, time to just see what sticks and generate an
  // entirely new matchup. If this fails there's no hope for the client.
  return generateNewMatchup(game, cBots, cMaps);
}

bool sc2tm::GameGenerator::generateActiveMap(Game &game, HashSet cBots, HashSet cMaps) {
  // Want end0 to be one before, because the opponent for the second last bot is the last one
  for (auto botIt0 = cBots.begin(), end0 = std::prev(cBots.end()); botIt0 != end0; ++botIt0) {
    for (auto botIt1 = std::next(botIt0), end1 = cBots.end(); botIt1 != end1; ++botIt1) {
      // Make the matchup and try to find it in the map
      Matchup matchup(*botIt0, *botIt1);
      auto activePairIt = active.find(matchup);

      // If we found a matchup we need to find an active map that we also have in common
      if (activePairIt != active.end()) {
        for (const auto &map : cMaps) {
          CounterMap &activeMaps = activePairIt->second;

          // Try to find the map in the counter map that still has games left
          auto counterIt = activeMaps.find(map);
          if (counterIt != activeMaps.end() && counterIt->second.left > 0) {
            // Found a match to give out!
            // Fill in the game
            game.bot0 = matchup.bot0;
            game.bot1 = matchup.bot1;
            game.map = map;

            // Decrement the game counter
            --counterIt->second.left;

            // Notify success
            return true;
          }
        }
      }
    }
  }

  // Failure
  return false;
}

bool sc2tm::GameGenerator::generateActiveMatchup(Game &game, HashSet cBots, HashSet cMaps) {
  for (auto botIt0 = cBots.begin(), end0 = std::prev(cBots.end()); botIt0 != end0; ++botIt0) {
    for (auto botIt1 = std::next(botIt0), end1 = cBots.end(); botIt1 != end1; ++botIt1) {
      // Make the matchup
      Matchup matchup(*botIt0, *botIt1);

      // Find the match up in the active and finished sets
      auto activePairIt = active.find(matchup);
      auto finishedIt = finished.find(matchup);

      // If the match up wasn't in either set then move on because it has never been scheduled
      // before
      if (activePairIt == active.end() && finishedIt == finished.end())
        continue;

      // If we found an active matchup we subtract the set of currently active maps from the set
      // of usable maps. This may seem like an odd thing to do because getting into this function
      // means that we were unable to find an active map to participate in, but this could just
      // mean that there's an active map with all instances currently sent out.
      if (activePairIt != active.end()) {
        // Build a set of active maps
        HashSet activeMaps;
        for (const auto &pair : activePairIt->second)
          activeMaps.insert(pair.first);

        // Subtract active maps from the set of all maps
        HashSet nonActiveMaps;
        std::set_difference(cMaps.begin(), cMaps.end(),
                            activeMaps.begin(), activeMaps.end(),
                            std::inserter(nonActiveMaps, nonActiveMaps.end()),
                            CompareHashPtrFtor());

        // Assign this resulting set over cMaps for use by the next section if there's games in the
        // finished map or straight by the generator in the final section
        cMaps = nonActiveMaps;
      }

      // If we found the matchup in the finished set then we should subtract these maps from the
      // usable map set
      if (finishedIt != finished.end()) {
        // The set of maps that are finished for this match up
        const HashSet &finishedMaps = finishedIt->second;

        // Subtract finished maps from set of all maps
        HashSet nonFinishedMaps;
        std::set_difference(cMaps.begin(), cMaps.end(),
                            finishedMaps.begin(), finishedMaps.end(),
                            std::inserter(nonFinishedMaps, nonFinishedMaps.end()),
                            CompareHashPtrFtor());

        // Assign this resulting set over cMaps for use by the next section
        cMaps = nonFinishedMaps;
      }

      // If we don't have any usable maps, just move onto another matchup
      if (cMaps.empty())
        continue;

      // Good new everyone! We found a usable map!
      // Put it in the schedule and then send the game off.
      // Get the map we're going to schedule.
      assert(!cMaps.empty());
      SHA256Hash::ptr map = *cMaps.begin();

      // Ensure that we haven't screwed up and are trying to schedule a map that is already
      // scheduled
      assert(active[matchup].find(map) == active[matchup].end());

      // TODO maybe get the game count from a game generator arg
      // Put a new game counter in and take a game away.
      // Note that we can't use the CounterMap's operator[] because it requires the value type
      // to have a default constructor. Find still works fine.
      GameCounter counter(numGames);
      --counter.left;
      active[matchup].emplace(map, counter);

      // Fill the game in
      game.bot0 = matchup.bot0;
      game.bot1 = matchup.bot1;
      game.map = map;

      // Tell them of our successes
      return true;
    }
  }

  // Failure
  return false;
}

bool sc2tm::GameGenerator::generateNewMatchup(Game &game, HashSet cBots, HashSet cMaps) {
  // Build the set of matchups that are in the active map
  MatchupSet activeMatchups;
  for (const auto &matchup : active)
    activeMatchups.insert(matchup.first);

  // Add matchups that are in the finished map (union), all of these matchups are "active"
  for (const auto &matchup : finished)
    activeMatchups.insert(matchup.first);

  // Now try to find a matchup that isn't active
  for (auto botIt0 = cBots.begin(), end0 = std::prev(cBots.end()); botIt0 != end0; ++botIt0) {
    for (auto botIt1 = std::next(botIt0), end1 = cBots.end(); botIt1 != end1; ++botIt1) {
      // Make the matchup
      Matchup matchup(*botIt0, *botIt1);

      // If the matchup exists in the active set we just want to move on
      auto activeIt = activeMatchups.find(matchup);
      if (activeIt != activeMatchups.end())
        continue;

      // Now we've found a matchup that hasn't started! Start it!
      // Create counter and take a game from it
      GameCounter counter(numGames);
      --counter.left;

      // Get our map
      assert(!cMaps.empty()); // Need at least one map
      SHA256Hash::ptr map = *cMaps.begin();

      // Create a CounterMap with the map and counter
      CounterMap counterMap;
      counterMap.emplace(map, counter);

      // Place the counter map into the active map with our matchup
      active.emplace(matchup, counterMap);

      // Fill in the game
      game.bot0 = matchup.bot0;
      game.bot1 = matchup.bot1;
      game.map = map;

      // We succeeded!
      return true;
    }
  }
}

// TODO We need to lock this when multithreading happens
// This is fairly easy to begin with, just decrement the done counter for the matchup and map.
// However, if we find that done has hit zero we need to move the map to the finished list. Further,
// if a bot has competed against every bot and finished every map then it should be moved to the
// finishedBots set.
void sc2tm::GameGenerator::notifySuccess(const Game &game) {
  Matchup matchup(game.bot0, game.bot1);

  // Find the matchup/CounterMap pair in the map
  auto activeIt = active.find(matchup);
  assert(activeIt != active.end()); // If it's failing, it must be active

  // Find the map/GameCounter pair in the map
  auto &counterMap = activeIt->second;
  auto counterIt = counterMap.find(game.map);
  assert(counterIt != counterMap.end());

  // If the left counter is greater than one then all we need to do is decrement and move on
  if (counterIt->second.done > 1) {
    --counterIt->second.done;
    return;
  }

  // But if it is one (or zero, but that should never happen because it should've been removed) then
  // we need to move this map to the finished map
  // It shouldn't have ended before
  assert(finished[matchup].find(game.map) == finished[matchup].end());
  finished[matchup].insert(game.map);

  // Remove it from the active map. We can use the iterator here to save the map having to find it
  // again.
  counterMap.erase(counterIt);

  // Now we begin the process of checking if one of the bots is done. This is kind of complicated.
  // Either or both of the bots could be "done". Build sets of the bots that each bot is done
  // (i.e. played every map) against. Because this is such a long process, we're going to try and
  // leave at every opportunity.
  HashSet bot0Done; // The set of bots that bot0 has finished against
  HashSet bot1Done; // The set of bots that bot1 has finished against
  bool bot0Fail = false; // Whether or not bot0 has already failed to be "done"
  bool bot1Fail = false; // Whether or not bot1 has already failed to be "done"
  for (const auto &finishedPair : finished) {
    const Matchup &finishedMatchup = finishedPair.first;
    const HashSet &finishedMaps = finishedPair.second;

    // First check if this bot already failed, then check if bot0 was either of the bots in this
    // matchup
    if (!bot0Fail && finishedMatchup.contains(game.bot0)) {
      // We check size here rather than comparing values. I'm fairly certain that we can't "finish"
      // a map that the GameGenerator doesn't have. If we have the same number of maps then this
      // matchup is "done"
      // TODO Add some debug only code that does per element comparison rather than size comparison
      if (finishedMaps.size() == maps.size())
        bot0Done.insert(finishedMatchup.indexOf(game.bot0) == 0 ?
                        finishedMatchup.bot1 : finishedMatchup.bot0);
      // We found a matchup that isn't done and don't need to keep checking
      else
        bot0Fail = true;
    }

    // Check if bot1 was either of the bots in this matchup
    if (!bot1Fail && finishedMatchup.contains(game.bot1)) {
      // We check size here rather than comparing values. I'm fairly certain that we can't "finish"
      // a map that the GameGenerator doesn't have. If we have the same number of maps then this
      // matchup is "done"
      // TODO Add some debug only code that does per element comparison rather than size comparison
      if (finishedMaps.size() == maps.size())
        bot1Done.insert(finishedMatchup.indexOf(game.bot1) == 0 ?
                        finishedMatchup.bot0 : finishedMatchup.bot1);
      // We found a matchup that isn't done and don't need to keep checking
      else
        bot1Fail = true;
    }

    // Both bots failed to be done, just leave
    if (bot0Fail && bot1Fail)
      return;
  }

  // So we get here meaning that one of the bots had all of their matchups "done", but that doesn't
  // mean they had *all* matchups. Better check that..
  if (!bot0Fail)
    bot0Fail = bot0Done.size() == (bots.size() - 1); // -1 for self

  if (!bot1Fail)
    bot1Fail = bot1Done.size() == (bots.size() - 1); // -1 for self

  // Both bots fail, we can leave now
  if (bot0Fail && bot1Fail)
    return;

  // Getting here means that one of the bots is actually done. That means we need to clean it out of
  // the other two maps and add it to the finishedBots.
  // Check if either of them passed. If they did, add them to the finished set
  if (!bot0Fail)
    finishedBots.insert(game.bot0);
  if (!bot1Fail)
    finishedBots.insert(game.bot1);

  // Now generate every matchup and remove it from the active/finished maps
  for (const auto &bot : bots) {
    // If we didn't fail and this isn't a self matchup, clear it out
    if (!bot0Fail && SHA256Hash::compare(game.bot0, bot) != 0) {
      // Delete this matchup
      Matchup purgeable(game.bot0, bot);
      active.erase(purgeable);
      finished.erase(purgeable);
    }

    if (!bot1Fail && SHA256Hash::compare(game.bot1, bot) != 0) {
      // Delete this matchup
      Matchup purgeable(game.bot1, bot);
      active.erase(purgeable);
      finished.erase(purgeable);
    }
  }
}

// TODO We need to lock this when multithreading happens
// This is actually fairly easy, just make up the matchup and use the map to get the counter so
// that we can increment the left counter
void sc2tm::GameGenerator::notifyFail(const Game &game) {
  Matchup matchup(game.bot0, game.bot1);

  // Find the matchup/CounterMap pair in the map
  auto activeIt = active.find(matchup);
  assert(activeIt != active.end()); // If it's failing, it must be active

  // Find the map/GameCounter pair in the map
  auto &counterMap = activeIt->second;
  auto counterIt = counterMap.find(game.map);
  assert(counterIt != counterMap.end());

  // Increment the left count
  ++counterIt->second.left;
}
