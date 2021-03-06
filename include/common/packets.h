#ifndef SC2TM_PACKETS_H
#define SC2TM_PACKETS_H

#include "common/file_operations.h"
#include "common/Game.h"
#include "common/sha256.h"

#include <boost/asio/streambuf.hpp>

#include <vector>

namespace sc2tm {
// TODO provide a way to know how much data to read

// It might be possible to move the streambuf constructor to this so not every packet has to define
// it
//! Base class for all packets.
struct Packet {
  //! Transforms the packet into data appropriate for sending over the network.
  /**
   * Transforms the packet into a data appropriate for sending over the network and places it in a
   * provided buffer. Subclasses should override this to provide the packet writing routine with
   * ability to write its data.
   *
   * @param buffer The buffer this packets bytes should be written into.
   */
  virtual void toBuffer(boost::asio::streambuf &buffer) = 0;

protected:

  //! Fill this packet from the bytes in a buffer.
  /**
   * Fill this packet from the bytes in a buffer. Will pull out as many bytes as necessary.
   * Assumes the correct number of bytes are waiting in the buffer.
   *
   * @param buffer The buffer to construct from.
   */
  virtual void fromBuffer(boost::asio::streambuf &buffer) = 0;
};

// --- ClientHandShakePacket

//! All data required for a client handshake packet.
struct ClientHandshakePacket : Packet {
  // These fields aren't necessary when we're constructing a packet to send because they're
  // available from the config, however, they *are* necessary when we receive one and need somewhere
  // to put it
  //! This client's major version number.
  uint8_t clientMajorVersion;
  //! This client's minor version number.
  uint8_t clientMinorVersion;
  //! This client's patch version number.
  uint8_t clientPatchVersion;

  //! Array of bot hashes
  std::vector<std::vector<uint8_t>> botHashes;
  //! Array of map hashes
  std::vector<std::vector<uint8_t>> mapHashes;

  //! No default constructor.
  ClientHandshakePacket() = delete;

  //! Construct a handshake packet.
  /**
   * Construct a handshake packet, initializing the version numbers and copying hashes.
   *
   * @param botMap The bots that need to be included.
   * @param mapMap The maps that need to be included.
   */
  ClientHandshakePacket(const SHAFileMap &botMap, const SHAFileMap &mapMap);

  //! Construct a handshake from the bytes in a buffer.
  ClientHandshakePacket(boost::asio::streambuf &buffer);

  //! Converts this packet into data appropriate for sending over the network.
  /**
   * Transforms the packet into a data appropriate for sending over the network and places it in a
   * provided buffer. Due to the variable length of this packet, also writes the size of the packet
   * to the stream first so that the destination can wait for the correct amount of bytes to fill
   * out the entire packet.
   *
   * @param buffer The buffer this packets bytes should be written into.
   */
  virtual void toBuffer(boost::asio::streambuf &buffer) override;

  //! Get the size this packet will place in the buffer.
  size_t size() {
    return
        sizeof(uint8_t) * 3 + // The three version numbers
        sizeof(uint32_t) * 2 + // The hash size fields
        sizeof(uint8_t) * (botHashes.size() + mapHashes.size()) * SHA256::DIGEST_SIZE;
  }

protected:
  //! Fill this packet from the bytes in a buffer.
  /**
   * Fill this packet from the bytes in a buffer. Assumes that the caller has already accounted
   * for the variable length of this packet and the correct number of bytes are waiting in the
   * buffer.
   *
   * @param buffer The buffer to construct from.
   */
  virtual void fromBuffer(boost::asio::streambuf &buffer) override;
};

// --- PreGamePacket
//! Represents all possible pregame commands
enum PregameCommand : uint8_t {
  DISCONNECT = 0,
  START_GAME
};

//! All data required for a pregame command packet.
struct PregameCommandPacket : Packet {
  //! The pregame command.
  PregameCommand cmd;

  //! No default constructor.
  PregameCommandPacket() = delete;

  //! Construct a PregameCommandPacket from a command.
  PregameCommandPacket(PregameCommand cmd) : cmd(cmd) { }

  //! Construct a PregameCommandPacket from the bytes in a buffer.
  PregameCommandPacket(boost::asio::streambuf &buffer) : cmd() { fromBuffer(buffer); }

  //! Converts this packet into data appropriate for sending over the network.
  virtual void toBuffer(boost::asio::streambuf &buffer) override;

  //! Get the size this packet will place in the buffer.
  static size_t size() {
    return sizeof(PregameCommand);
  }

protected:
  //! Fill this packet from the bytes in a buffer.
  virtual void fromBuffer(boost::asio::streambuf &buffer) override;
};

//! Represents all possible reasons for disconnecting pregame.
enum PregameDisconnectReason : uint8_t {
  BAD_VERSION = 0,
  NO_GAMES
};

//! All data required for a pregame disconnect packet
struct PregameDisconnectPacket : Packet {
  //! The disconnect reason
  PregameDisconnectReason reason;

  //! No default constructor.
  PregameDisconnectPacket() = delete;

  //! Construct a PregameDisconnectPacket from a command.
  PregameDisconnectPacket(PregameDisconnectReason reason) : reason(reason) { }

  //! Construct a PregameDisconnectPacket from the bytes in a buffer.
  PregameDisconnectPacket(boost::asio::streambuf &buffer) : reason() { fromBuffer(buffer); }

  //! Converts this packet into data appropriate for sending over the network.
  virtual void toBuffer(boost::asio::streambuf &buffer) override;

  //! Get the size this packet will place in the buffer.
  static size_t size() {
    return sizeof(PregameDisconnectReason);
  }

protected:
  //! Fill this packet from the bytes in a buffer.
  virtual void fromBuffer(boost::asio::streambuf &buffer) override;
};

//! All data required for scheduling a new game.
struct StartGamePacket : Packet {
  //! The game to send to the client.
  uint8_t data[3][SHA256::DIGEST_SIZE];

  //! No default constructor.
  StartGamePacket() = delete;

  //! Construct a StartGamePacket from a game.
  StartGamePacket(const Game &game);

  //! Construct a PregameDisconnectPacket from the bytes in a buffer.
  StartGamePacket(boost::asio::streambuf &buffer) : data() { fromBuffer(buffer); }

  //! Converts this packet into data appropriate for sending over the network.
  virtual void toBuffer(boost::asio::streambuf &buffer) override;

  //! Get the size this packet will place in the buffer.
  static size_t size() {
    return sizeof(data);
  }

protected:
  //! Fill this packet from the bytes in a buffer.
  virtual void fromBuffer(boost::asio::streambuf &buffer) override;

};

//! Represents all possible status codes of a game finishing.
enum GameStatus : uint8_t {
  SUCCESS = 0,
  FAILURE
};

//! All data required for a game status packet.
struct GameStatusPacket : Packet {
  //! The status of the game.
  GameStatus status;

  //! No default constructor.
  GameStatusPacket() = delete;

  //! Construct a GameStatusPacket from a status code.
  GameStatusPacket(GameStatus status) : status(status) { }

  //! Construct a GameStatusPacket from the bytes in a buffer.
  GameStatusPacket(boost::asio::streambuf &buffer) : status() { fromBuffer(buffer); }

  //! Converts this packet into data appropriate for sending over the network.
  virtual void toBuffer(boost::asio::streambuf &buffer) override;

  //! Get the size this packet will place in the buffer.
  static size_t size() {
    return sizeof(GameStatus);
  }

protected:
  //! Fill this packet from the bytes in a buffer.
  virtual void fromBuffer(boost::asio::streambuf &buffer) override;

};

} // End sc2tm namespace

#endif //SC2TM_PACKETS_H
