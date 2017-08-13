#include "common/packets.h"

#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <istream>
#include <ostream>

#include "common/config.h"

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

sc2tm::ClientHandshakePacket::ClientHandshakePacket() :
    clientMajorVersion(0), clientMinorVersion(0), clientPatchVersion(0) { };

void sc2tm::ClientHandshakePacket::toBuffer(boost::asio::streambuf &buff) {
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

  // Create an ostream from the buffer
  std::ostream os(&buff);

  // Write size to buffer. Note that we're writing size *and* data in the same step. It makes
  // no difference to the server when it receives the data, just that it listens for the correct
  // amount after receiving the size. This way we can move onto the next step in the state machine.
  os << htonl(size);

  // Writing version number is easy since they're just bytes
  os << clientMajorVersion << clientMinorVersion << clientPatchVersion;

  // Cast the size of the vector down to uint32_t, we don't need more than 4b hashes, then
  // to network order
  os << htonl((uint32_t) botHashes.size());

  // write all of the bytes of the bot hashes
  for (const auto &hash : botHashes)
    for (uint8_t b : hash)
      os << b;

  // Cast the size of the vector down to uint32_t, we don't need more than 4b hashes, then
  // to network order
  os << htonl((uint32_t) mapHashes.size());

  // write all of the bytes of the map hashes
  for (const auto &hash : botHashes)
    for (uint8_t b : hash)
      os << b;
}

void sc2tm::ClientHandshakePacket::fromBuffer(boost::asio::streambuf &buffer) {
  // Create an istream from the buffer
  std::istream is(&buffer);

  // Read the version numbers, they're easy.
  is >> clientMajorVersion;
  is >> clientMinorVersion;
  is >> clientPatchVersion;

  // Read bot hash size in, then convert from network order
  uint32_t botHashesSize = 0;
  is >> botHashesSize;
  botHashesSize = ntohl(botHashesSize);

  // Now read in that number of hashes
  for (int i = 0; i < botHashesSize; ++i) {
    std::vector<uint8_t> digest(SHA256::DIGEST_SIZE, 0);
    for (int j = 0; j < SHA256::DIGEST_SIZE; ++j) {
      uint8_t byte = 0;
      is >> byte;
      digest.push_back(byte);
    }
    botHashes.push_back(digest);
  }

  // Read map hash size in, then convert from network order
  uint32_t mapHashesSize = 0;
  is >> mapHashesSize;
  mapHashesSize = ntohl(mapHashesSize);

  // Now read in that number of hashes
  for (int i = 0; i < mapHashesSize; ++i) {
    std::vector<uint8_t> digest(SHA256::DIGEST_SIZE, 0);
    for (int j = 0; j < SHA256::DIGEST_SIZE; ++j) {
      uint8_t byte = 0;
      is >> byte;
      digest.push_back(byte);
    }
    mapHashes.push_back(digest);
  }
}
