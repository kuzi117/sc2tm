#include "common/packets.h"

#include "common/config.h"
#include "common/buffer_operations.h"

#ifdef _WIN32
#include <Winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <cassert>
#include <cstring>
#include <iostream>


// --- ClientHandshakePacket
sc2tm::ClientHandshakePacket::ClientHandshakePacket(const SHAFileMap &botMap,
                                                    const SHAFileMap &mapMap) :
    clientMajorVersion(sc2tm::clientMajorVersion), clientMinorVersion(sc2tm::clientMinorVersion),
    clientPatchVersion(sc2tm::clientPatchVersion) {

  // Initialize hash arrays with memory and then copy over a hash
  for (const auto &bot : botMap) {
    std::vector<uint8_t> digest(SHA256::DIGEST_SIZE, 0);
    std::memcpy(digest.data(), bot.second->get(), SHA256::DIGEST_SIZE);
    botHashes.push_back(digest);
  }

  for (const auto &map : mapMap) {
    std::vector<uint8_t> digest(SHA256::DIGEST_SIZE, 0);
    std::memcpy(digest.data(), map.second->get(), SHA256::DIGEST_SIZE);
    mapHashes.push_back(digest);
  }

  // Sanity checking
  for (const auto &hash : botHashes)
    assert(hash.size() == SHA256::DIGEST_SIZE);

  for (const auto &hash : mapHashes)
    assert(hash.size() == SHA256::DIGEST_SIZE);
}

sc2tm::ClientHandshakePacket::ClientHandshakePacket(boost::asio::streambuf &buffer) :
    clientMajorVersion(0), clientMinorVersion(0), clientPatchVersion(0) {
  fromBuffer(buffer);
};

void sc2tm::ClientHandshakePacket::toBuffer(boost::asio::streambuf &buffer) {
  // Sanity checking
  for (const auto &hash : botHashes)
    assert(hash.size() == SHA256::DIGEST_SIZE);

  for (const auto &hash : mapHashes)
    assert(hash.size() == SHA256::DIGEST_SIZE);

  // Calculate how many bytes we'll be writing.
  uint32_t size =
      sizeof(uint8_t) * 3 + // The version fields
      sizeof(uint32_t) * 2 + // The hash size fields
      sizeof(uint8_t) * SHA256::DIGEST_SIZE * botHashes.size() + // The bot hashes field
      sizeof(uint8_t) * SHA256::DIGEST_SIZE * mapHashes.size(); // The map hashes field
  std::cout << "PREDICTED CONSTRUCTED PACKET SIZE: " << size << "\n"; // TODO DEBUG

  // Create an ostream from the buffer
  std::ostream os(&buffer);

  // Write size to buffer. Note that we're writing size *and* data in the same step. It makes
  // no difference to the server when it receives the data, just that it listens for the correct
  // amount after receiving the size. This way we can move onto the next step in the state machine.
  writeUint32(size, os);

  // Writing version number is easy since they're just bytes
  os << clientMajorVersion << clientMinorVersion << clientPatchVersion;

  // Cast the size of the vector down to uint32_t, we don't need more than 4b hashes, then into the
  // buffer
  writeUint32((uint32_t) botHashes.size(), os);

  // write all of the bytes of the bot hashes
  for (const auto &hash : botHashes)
    writeHashBuffer(hash.data(), os);

  // Cast the size of the vector down to uint32_t, we don't need more than 4b hashes, then into the
  // buffer
  writeUint32((uint32_t) mapHashes.size(), os);

  // write all of the bytes of the map hashes
  for (const auto &hash : mapHashes)
    writeHashBuffer(hash.data(), os);
}

void sc2tm::ClientHandshakePacket::fromBuffer(boost::asio::streambuf &buffer) {
  // Create an istream from the buffer
  std::istream is(&buffer);

  // Read the version numbers, they're easy.
  is >> clientMajorVersion >> clientMinorVersion >> clientPatchVersion;

  // Read bot hash size in
  uint32_t botHashesSize = readUint32(is);

  // Now read in that number of hashes
  for (int i = 0; i < botHashesSize; ++i) {
    std::vector<uint8_t> digest(SHA256::DIGEST_SIZE, 0);
    assert(digest.size() >= SHA256::DIGEST_SIZE);
    readHashBuffer(digest.data(), is);
    botHashes.push_back(digest);
  }

  // Read map hash size in
  uint32_t mapHashesSize = readUint32(is);

  // Now read in that number of hashes
  for (int i = 0; i < mapHashesSize; ++i) {
    std::vector<uint8_t> digest(SHA256::DIGEST_SIZE, 0);
    assert(digest.size() >= SHA256::DIGEST_SIZE);
    readHashBuffer(digest.data(), is);
    mapHashes.push_back(digest);
  }
}

// --- PregameCommandPacket
void sc2tm::PregameCommandPacket::toBuffer(boost::asio::streambuf &buffer) {
  // Create an ostream from the buffer
  std::ostream os(&buffer);
  std::cout << "SENDING COMMAND: " << cmd << '\n';

  // Write the command to the buffer.
  os << static_cast<uint8_t>(cmd);
}

void sc2tm::PregameCommandPacket::fromBuffer(boost::asio::streambuf &buffer) {
  // Create an istream from the buffer
  std::istream is(&buffer);

  // Read the command from the buffer
  uint8_t buffCmd;
  is >> buffCmd;
  cmd = static_cast<PregameCommand>(buffCmd);
}

// --- PregameDisconnectPacket
void sc2tm::PregameDisconnectPacket::toBuffer(boost::asio::streambuf &buffer) {
  // Create an ostream from the buffer
  std::ostream os(&buffer);

  // Write the command to the buffer.
  os << static_cast<uint8_t>(reason);
}

void sc2tm::PregameDisconnectPacket::fromBuffer(boost::asio::streambuf &buffer) {
  // Create an istream from the buffer
  std::istream is(&buffer);

  // Read the command from the buffer
  uint8_t buffReason;
  is >> buffReason;
  reason = static_cast<PregameDisconnectReason>(buffReason);
}

// --- StartGamePacket
sc2tm::StartGamePacket::StartGamePacket(const Game &game) : data{} {
  std::memcpy(data[0], game.bot0->get(), SHA256::DIGEST_SIZE);
  std::memcpy(data[1], game.bot1->get(), SHA256::DIGEST_SIZE);
  std::memcpy(data[2], game.map->get(), SHA256::DIGEST_SIZE);
}

void sc2tm::StartGamePacket::toBuffer(boost::asio::streambuf &buffer) {
  // Create an ostream from the buffer
  std::ostream os(&buffer);

  // Write the two bots and the map
  writeHashBuffer(data[0], os);
  writeHashBuffer(data[1], os);
  writeHashBuffer(data[2], os);
}

void sc2tm::StartGamePacket::fromBuffer(boost::asio::streambuf &buffer) {
  // Create an istream from the buffer
  std::istream is(&buffer);

  // Read the two bots and the map
  readHashBuffer(data[0], is);
  readHashBuffer(data[1], is);
  readHashBuffer(data[2], is);
}

// --- GameStatusPacket
void sc2tm::GameStatusPacket::toBuffer(boost::asio::streambuf &buffer) {
  // Create an ostream from the buffer
  std::ostream os(&buffer);
  std::cout << "SENDING STATUS: " << status << '\n';

  // Write the status to the buffer.
  os << static_cast<uint8_t>(status);
}

void sc2tm::GameStatusPacket::fromBuffer(boost::asio::streambuf &buffer) {
  // Create an istream from the buffer
  std::istream is(&buffer);

  // Read the status from the buffer
  uint8_t buffCmd;
  is >> buffCmd;
  status = static_cast<GameStatus>(buffCmd);
}
