#ifndef SC2TM_GAMEGENERATOR_H
#define SC2TM_GAMEGENERATOR_H

#include "common/file_operations.h"
#include "common/Game.h"
#include "common/sha256.h"

namespace sc2tm {
// TODO TEST THE SHIT OUT OF THIS THING
// Client has same maps/bots as us
// Client has a subset of our maps/bots
// Client has a superset of our maps/bots
// Client's set of maps/bots intersects with ours but isn't a superset or subset

//! Object that manages generating games to be played.
/**
 * This is part of an effort to reduce the amount of Game objects in memory due to the factorial
 * growth rate when adding more bots, more maps, and more games played.
 */
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

    //! Does this matchup contain the bot?
    bool contains(const SHA256Hash::ptr bot) const;

    //! Index of the bot in this matchup.
    uint8_t indexOf(const SHA256Hash::ptr bot) const;
  };

  //! Functor to order matchups.
  struct CompareMatchupFtor {
    int operator()(const Matchup &match0, const Matchup &match1) const {
      int first = SHA256Hash::compare(match0.bot0, match1.bot0);
      if (first == 0)
        return SHA256Hash::compare(match0.bot1, match1.bot1);
      else
        return first;
    }
  };

  //! Typedef that maps a map to its game counter.
  typedef std::map<SHA256Hash::ptr, GameCounter, CompareHashPtrFtor> CounterMap;
  //! Typedef that maps a matchup to map of game counters.
  typedef std::map<Matchup, CounterMap, CompareMatchupFtor> MatchupMap;
  //! Typedef that maps a matchup to a list of finished maps.
  typedef std::map<Matchup, HashSet, CompareMatchupFtor> FinishedMap;
  //! Typdef that holds matchups as a set.
  typedef std::set<Matchup, CompareMatchupFtor> MatchupSet;

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

  //! The set of bots that have played all of their matchups.
  /**
   * The set of bots that have played all of their matchups. This means that every matchup has been
   * played on every map the requisite number of times. Bots are added to this set when the final
   * for this bot's final matchup has been played.
   */
  HashSet finishedBots;

public:
  // TODO This could (and should) be deleted once Server can construct this as part of its init.
  // TODO This won't leave it invalid, but no games will be generated because the set of bots and
  // TODO maps will be empty.
  //! Default constructor.
  GameGenerator() = default;

  //! Construct a game generator for given bots and map sets.
  GameGenerator(const SHAFileMap &botMap, const SHAFileMap &mapMap);

  //! Generate a game for a client with given bot and map sets.
  /**
   * Generate a game for a client with given bot and map sets. This will try to finish matchups and
   * their active maps as soon as possible, within the list of bots and maps that the client has.
   * This means it will target its generated games to the maps and bots available to a client.
   *
   * @param game The game to fill in.
   * @param cBots The set of bots the client has available.
   * @param cMaps The set of maps the client has available.
   * @return True if a game was found, false otherwise.
   */
  bool generateGame(Game &game, HashSet cBots, HashSet cMaps);

  //! Notify the generator that a game completed successfully.
  /**
   * Notify the generator that a game completed successfully. This will commit the game as done and
   * remove it from the in progress state. This function will also try to commit maps as done for a
   * matchup and bots as done entirely if they've competed against every other bot on every map the
   * requisite number of times.
   *
   * @param game The game that completed successfully.
   */
  void notifySuccess(const Game &game);

  //! Notify the generator that a game did not complete successfully.
  /**
   * Notify the generator that a game did not complete successfully. This game will be added back
   * into the pool of games to be rescheduled to another client.
   *
   * @param game The game that did not complete successfully.
   */
  void notifyFail(const Game &game);

private:
  //! Try to generate a game for a client from an active matchup and map.
  /**
   * Try to generate a game for a client from an active matchup and map. An active matchup is one
   * that has not finished all of its games on every map, an active map is one that has not finished
   * playing all of its games for a single matchup. Concretely this means that a matchup exists in
   * the active map, in which one of the active maps has a counter whose left value is > 0.
   *
   * @param game The game to fill in.
   * @param cBots The set of bots the client has available.
   * @param cMaps The set of maps the client has available.
   * @return True if a game was found, false otherwise.
   */
  bool generateActiveMap(Game &game, HashSet cBots, HashSet cMaps);

  //! Try to generate a game for a client from an active matchup and new map.
  /**
   * Try to generate a game for a client from an active matchup and new map. An active matchup is
   * one that has not finished all of its games on every map. Concretely this means that a matchup
   * exists in the active and/or finished maps where cMaps - active[matchup] - finished[matchup] is
   * not empty. The new game is generated from this difference and added to the active map.
   *
   * This is meant to be used under the assumption that a game couldn't be generated by
   * generateActiveMap but even without that assumption a game generated by this function holds the
   * above property.
   *
   * @param game The game to fill in.
   * @param cBots The set of bots the client has available.
   * @param cMaps The set of maps the client has available.
   * @return True if a game was found, false otherwise.
   */
  bool generateActiveMatchup(Game &game, HashSet cBots, HashSet cMaps);

  //! Try to generate a game for a client from a new matchup and map.
  /**
   * Try to generate a game for a client from a new matchup and map. A new matchup is one that is
   * not in either the active or finished maps. A new matchup is generated and the first map
   * possible is scheduled.
   *
   * This is meant to be used under the assumption that a game couldn't be generated by
   * generateActiveMatchup but even without that assumption a game generated by this function holds
   * the above property.
   *
   * @param game The game to fill in.
   * @param cBots The set of bots the client has available.
   * @param cMaps The set of maps the client has available.
   * @return True if a game was found, false otherwise.
   */
  bool generateNewMatchup(Game &game, HashSet cBot, HashSet cMaps);
};

} // End sc2tm namespace

#endif //SC2TM_GAMEGENERATOR_H
