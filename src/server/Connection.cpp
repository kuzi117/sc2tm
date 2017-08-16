#include "server/Connection.h"

#include "common/buffer_operations.h"
#include "common/config.h"
#include "common/packets.h"
#include "server/Server.h"

#include <iostream>

sc2tm::Connection::~Connection() {
  std::cout << "CONNECTION DYING\n";
};

void sc2tm::Connection::start() {
  auto readHandshakeSizeFn =
      [&] (const boost::system::error_code& error, std::size_t bytes_transferred) {
        // Sanity checking
        assert(error.value() == boost::system::errc::success); // TODO handle failures
        assert(bytes_transferred == sizeof(uint32_t));

        // Create an istream from the buffer
        std::istream is(&buffer);

        // Read in size and convert to host
        uint32_t size = readUint32(is);

        // Make the second callback that actually reads the handshake
        auto readHandshakeFn =
            [&, size] (const boost::system::error_code& error2, std::size_t bytes_transferred2) {
              assert(error2.value() == boost::system::errc::success); // TODO handle failures
              assert(bytes_transferred2 == size);
              readHandshake();
            };
        boost::asio::async_read(_socket, buffer, boost::asio::transfer_exactly(size),
                                readHandshakeFn);
      };
  boost::asio::async_read(_socket, buffer, boost::asio::transfer_exactly(sizeof(uint32_t)),
                          readHandshakeSizeFn);
};

void sc2tm::Connection::readHandshake() {
  ClientHandshakePacket packet(buffer);
  std::cout << "\nClient connect: \n";
  std::cout << "VERSION: "
            << (int) packet.clientMajorVersion << '.'
            << (int) packet.clientMinorVersion << '.'
            << (int) packet.clientPatchVersion << '\n';

  // Import the client's bots
  std::cout << "bots\n"; // TODO DEBUG
  for (auto hash : packet.botHashes) {
    bots.insert(std::make_shared<SHA256Hash>(hash.data()));
    std::cout << SHA256Hash(hash.data()) << "\n"; // TODO DEBUG
  }

  // Import the client's maps
  std::cout << "maps\n"; // TODO DEBUG
  for (auto hash : packet.mapHashes) {
    maps.insert(std::make_shared<SHA256Hash>(hash.data()));
    std::cout << SHA256Hash(hash.data()) << "\n"; // TODO DEBUG
  }

  // If there's a version mismatch we should just disconnect
  // This might be more complicated later but for now it's reasonable to not deal with clients
  // with the wrong version
  if (packet.clientMajorVersion != clientMajorVersion ||
      packet.clientMinorVersion != clientMinorVersion ||
      packet.clientPatchVersion != clientPatchVersion)
    sendPregameDisconnect(BAD_VERSION);

  // No client version mismatch, so we can send them a game
  scheduleGame();
}

void sc2tm::Connection::scheduleGame() {
  // Try to find a game, if we do, send it, if we don't that means there are no games for it, so
  // we might as well disconnect it
  bool success = server.gen.generateGame(game, bots, maps);
  if (!success)
    sendPregameDisconnect(NO_GAMES);
  sendGame();
}

void sc2tm::Connection::sendPregameDisconnect(PregameDisconnectReason r) {
  // Generate our packets and put them in the buffer.
  PregameCommandPacket cmd(DISCONNECT);
  PregameDisconnectPacket reason(r);
  cmd.toBuffer(buffer);
  reason.toBuffer(buffer);

  // Make a function to write the bytes and then request that we destroy this client connection
  auto destroyConnectionFn =
      [&] (const boost::system::error_code& error, std::size_t byteCount) {
        server.requestDestroyConnection(id);
      };
  boost::asio::async_write(_socket, buffer, destroyConnectionFn);
}

void sc2tm::Connection::sendGame() {
  // Generate our packets and put them in the buffer.
  PregameCommandPacket cmd(START_GAME);
  StartGamePacket gamePacket(game);
  cmd.toBuffer(buffer);
  gamePacket.toBuffer(buffer);

  // Make a function to wait on reading the game play status code back
  auto waitReadStatusFn =
      [&] (const boost::system::error_code& error, std::size_t byteCount) {
        assert(error.value() == boost::system::errc::success); // TODO handle failures
        assert(byteCount == sizeof(PregameCommandPacket) + sizeof(StartGamePacket));

        // Make a function to call the readHandshake function
        auto readStatusFn =
            [&] (const boost::system::error_code& error2, std::size_t byteCount2) {
              assert(error2.value() == boost::system::errc::success); // TODO handle failures
              assert(byteCount2 == sizeof(GameStatusPacket));
              readGameStatus();
            };
        boost::asio::async_read(_socket, buffer,
                                boost::asio::transfer_exactly(sizeof(GameStatusPacket)),
                                readStatusFn);
      };
  boost::asio::async_write(_socket, buffer, waitReadStatusFn);
}

void sc2tm::Connection::readGameStatus() {
  //TODO
}
