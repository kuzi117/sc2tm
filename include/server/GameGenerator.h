#ifndef SC2TM_GAMEGENERATOR_H
#define SC2TM_GAMEGENERATOR_H

#include "common/file_operations.h"
#include "common/Game.h"
#include "common/sha256.h"

namespace sc2tm {

//! Object that manages generating games to be played.
/**
 * This is part of an effort to reduce the amount of Game objects in memory due to the factorial
 * growth rate when adding more bots, more maps, and more games played.
 */
// TODO TEST THE SHIT OUT OF THIS THING
class GameGenerator {
  //! The set of bots we have to work with.
  HashSet bots;
  //! The set of maps we have to work with.
  HashSet maps;

  //! Holds info about a particular matchup.
  struct GameCounter {
    // The difference between these two numbers is the number of games currently being played for this
    // matchup.
    //! The number of games left to be issued for this matchup.
    /**
     * The number of games left to be issued for this matchup. This number decreases as games are
     * issued to clients, but can increase if a matchup fails to finish for some reason.
     */
    uint32_t left;

    //! The number of games left to be confirmed as done.
    /**
     * The number of games left to be confirmed as done. This number will never increase. These
     * games have been played and reported as done. When this hits 0 this set of games is considered
     * to be entirely played.
     */
    uint32_t done;

    //! No default constructor.
    GameCounter() = delete;

    //! Construct a game counter with a given number of games to play.
    GameCounter(uint32_t games) : left(games), done(games) { }
  };

  //! A bot matchup.
  /**
   * A bot matchup. The constructor always ensures that the bots are in sorted order.
   */
  struct Matchup {
    //! The first bot in the matchup.
    const SHA256Hash::ptr bot0;
    //! The second bot in the matchup.
    const SHA256Hash::ptr bot1;

    //! No default constructor.
    Matchup() = delete;

    //! Construct a matchup from two bots.
    Matchup(SHA256Hash::ptr bot0, SHA256Hash::ptr bot1);
  };

  //! Functor to order matchups.
  struct CompareMatchupFtor {
    int operator()(const Matchup &match0, const Matchup &match01) const {
      int first = SHA256Hash::compare(match0.bot0, match01.bot0);
      if (first == 0)
        return SHA256Hash::compare(match0.bot1, match01.bot1);
      else
        return first;
    }
  };

  //! Typedef that maps a map to its game counter.
  typedef std::map<SHA256Hash::ptr, GameCounter, CompareHashPtrFtor> CounterMap;
  //! Typedef that maps a matchup to map of game counters.
  typedef std::map<Matchup, CounterMap, CompareMatchupFtor> MatchupMap;
  //! Typdef that maps a matchup to a list of finished maps.
  typedef std::map<Matchup, HashSet, CompareMatchupFtor> FinishedMap;

  //! Map of games that are trying to be scheduled.
  /**
   * Map of games that are trying to be scheduled. This represents the set of matches that the
   * generator has begun scheduling. It will keep trying to schedule games from these matchups and
   * map combos any time a client asks for a new one. Only if a client doesn't have a matchup of
   * bots or a map to go with a matchup will the scheduler begin a new matchup.
   *
   * The idea is that we would like to keep this as small as possible. The way things are scheduled,
   * "nice" clients, clients that have many of our bots and maps, that normally would have inserted
   * a matchup that's sequentially earlier, will try to finish matchups that less forgiving clients
   * have created out of necessity.
   *
   * In the ideal situation (all clients have all bots and maps) clients will play through a single
   * matchup and through each map sequentially. This would mean at most this map contains a single
   * matchup and a single map with a counter (maybe two if we're on the cusp of finishing one).
   */
  MatchupMap active;

  //! The map of matchups to maps that are completed.
  /**
   * This maps a matchup to a set of maps that have been completed. These will only be moved in here
   * if the GameCounter.done hits the maximum. The generator will check here before adding a new
   * matchup and maps to the active map.
   */
  FinishedMap finished;

public:
  //! Default constructor.
  GameGenerator() = default;

  //! Construct a game generator for given bots and map sets.
  GameGenerator(const SHAFileMap &botMap, const SHAFileMap &mapMap);

  //! Generate a game for a client with given bot and map sets.
  bool generateGame(Game &game, HashSet cBots, HashSet cMaps);

private:
  //! Try to generate a game for a client from an active matchup and map.
  bool generateActiveMap(Game &game, HashSet cBots, HashSet cMaps);

  //! Try to generate a game for a client from an active matchup and new map.
  bool generateActiveMatchup(Game &game, HashSet cBots, HashSet cMaps);

  //! Try to generate a game for a client from a new matchup and map.
  bool generateNewMatchup(Game &game, HashSet cBot, HashSet cMaps);
};

} // End sc2tm namespace

#endif //SC2TM_GAMEGENERATOR_H
