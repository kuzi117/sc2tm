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
    bot0(SHA256Hash::compare(b0, b1) > 0 ? b0 : b1),
    bot1(SHA256Hash::compare(b0, b1) > 0 ? b1 : b0) { }


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
      Matchup m(*botIt0, *botIt1);
      auto activePairIt = active.find(m);

      // If we found a matchup we need to find an active map that we also have in common
      if (activePairIt != active.end()) {
        for (const auto &map : cMaps) {
          CounterMap &activeMaps = activePairIt->second;

          // Try to find the map in the counter map that still has games left
          auto counterIt = activeMaps.find(map);
          if (counterIt != activeMaps.end() && counterIt->second.left > 0) {
            // Found a match to give out!
            // Fill in the game
            game.bot0 = *botIt0;
            game.bot1 = *botIt1;
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
      if (activePairIt == active.end() && finishedIt != finished.end())
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
      game.bot0 = *botIt0;
      game.bot1 = *botIt1;
      game.map = map;

      // Tell them of our successes
      return true;
    }
  }

  // Failure
  return false;
}

bool sc2tm::GameGenerator::generateNewMatchup(Game &game, HashSet cBots, HashSet cMaps) {
  // TODO fill me in please!
}
