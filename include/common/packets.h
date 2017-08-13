#ifndef SC2TM_PACKETS_H
#define SC2TM_PACKETS_H

#include <vector>

#include <boost/asio/streambuf.hpp>

#include "common/file_operations.h"
#include "common/sha256.h"


namespace sc2tm {

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

  //! Construct a handshake packet.
  /**
   * Construct a handshake packet, initializing the version numbers and copying hashes.
   *
   * @param botMap The bots that need to be included.
   * @param mapMap The maps that need to be included.
   */
  ClientHandshakePacket(const SHAFileMap &botMap, const SHAFileMap &mapMap);

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

  //! Construct a handshake from the bytes in a buffer.
  /**
   * Construct an invalid handshake that is intended to be filled in by fromBuffer.
   */
  ClientHandshakePacket(boost::asio::streambuf &buffer);

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
enum PregameCommand : uint8_t {
  DISCONNECT = 0,
  START_GAME
};

} // End sc2tm namespace

#endif //SC2TM_PACKETS_H
