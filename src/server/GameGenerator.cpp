#include "server/GameGenerator.h"

#include "common/config.h"

#include <algorithm>

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
  HashSet botInter;
  HashSet mapInter;
  std::set_intersection(bots.begin(), bots.end(),
                        cBots.begin(), cBots.end(),
                        std::inserter(botInter, botInter.end()),
                        CompareHashPtrFtor());
  std::set_intersection(maps.begin(), maps.end(),
                        cMaps.begin(), cMaps.end(),
                        std::inserter(mapInter, mapInter.end()),
                        CompareHashPtrFtor());

  // If there's not enough bots for a matchup or a single map to play on then there's no games
  // to give out for this client.
  if (botInter.size() < 2 || mapInter.size() == 0)
    return false;

  // Do we ever succeed?
  bool success;

  // Try to find a matchup in the active matches from our list of common bots
  success = generateActiveMap(game, botInter, mapInter);
  if (success)
    return true;

  // Well we didn't find an already active matchup that this client could participate in, so we'll
  // try scheduling a new map for an existing matchup.
  success = generateActiveMatchup(game, botInter, mapInter);
  if (success)
    return true;

  // Couldn't find an existing matchup and new map, time to just see what sticks and generate an
  // entirely new matchup. If this fails there's no hope for the client.
  return generateNewMatchup(game, botInter, mapInter);
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
          const CounterMap &activeMaps = activePairIt->second;

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
      // Make the matchup and try to find it in the map
      Matchup m(*botIt0, *botIt1);
      auto activePairIt = active.find(m);

      // If we found a matchup we need to create a set of maps that we could schedule on
      if (activePairIt != active.end()) {
        // Build a set of active
        HashSet activeMaps;
        for (const auto &pair : activePairIt->second)
          activeMaps.insert(pair.first);

        // Generate the set of maps that aren't currently active. This may seem odd, because we just
        // determined that there was no map that we could participate in, but there might still be
        // an active map that we have in common (no more plays left)
        HashSet nonActiveMaps;
        std::set_difference(maps.begin(), maps.end(),
                            activeMaps.begin(), activeMaps.end(),
                            std::inserter(nonActiveMaps, nonActiveMaps.end()),
                            CompareHashPtrFtor());

        // Container for usable maps, will be filled in the next step
        HashSet usableMaps;

        // Try to find this match up in the finished map, if we find it, do another set subtraction
        auto finishedIt = finished.find(m);
        if (finishedIt != finished.end()) {
          const HashSet &finishedMaps = finishedIt->second;
          std::set_difference(nonActiveMaps.begin(), nonActiveMaps.end(),
                              activeMaps.begin(), activeMaps.end(),
                              std::inserter(usableMaps, usableMaps.end()),
                              CompareHashPtrFtor());
        }
          // Otherwise just put the nonActiveMaps in
        else
          usableMaps = nonActiveMaps;

        // If we don't have any usable maps, just move onto another matchup
        if (usableMaps.empty())
          continue;

        // Good new everyone! We found a usable map!
        // Put it in the schedule and then send the game off.
        SHA256Hash::ptr map = *usableMaps.begin();
        // TODO maybe get the game count from a game generator arg
        active[m][map] = GameCounter(numGames);
        --active[m][map].left; // Pop off the first game

        // Fill the game in
        game.bot0 = *botIt0;
        game.bot1 = *botIt1;
        game.map = map;

        // Tell them of our successes
        return true;
      }
    }
  }

  // Failure
  return false;
}

bool sc2tm::GameGenerator::generateNewMatchup(Game &game, HashSet cBot, HashSet cMaps) {
  // TODO fill me in please!
}
