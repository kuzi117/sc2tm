#ifndef SC2TM_GAME_H
#define SC2TM_GAME_H

#include "common/sha256.h"

namespace sc2tm {

//! Lightweight container for a game.
struct Game {
  // We don't want to duplicate data here like we do in packets because we can have so many of these
  // alive at any time. Using the shared_ptr saves us a bit of memory.
  //! The first participant in the game.
  SHA256Hash::ptr bot0;
  //! The second participant in the game.
  SHA256Hash::ptr bot1;
  //! The map the game will be played on.
  SHA256Hash::ptr map;
};

}

#endif //SC2TM_GAME_H
