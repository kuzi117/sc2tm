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

  std::cout << "bots\n";
  for (auto hash : packet.botHashes) {
    SHA256Hash shahash(hash.data());
    std::cout << shahash << "\n";
  }

  std::cout << "maps\n";
  for (auto hash : packet.mapHashes) {
    SHA256Hash shahash(hash.data());
    std::cout << shahash << "\n";
  }

  if (packet.clientMajorVersion != clientMajorVersion ||
      packet.clientMinorVersion != clientMinorVersion ||
      packet.clientPatchVersion != clientPatchVersion)
    sendPregameDisconnect(BAD_VERSION);
}

void sc2tm::Connection::sendPregameDisconnect(PregameDisconnectReason r) {
  PregameCommandPacket cmd(DISCONNECT);
  PregameDisconnectPacket reason(r);

  cmd.toBuffer(buffer);
  reason.toBuffer(buffer);

  auto destroyConnectionFn =
      [&] (const boost::system::error_code& error, std::size_t byteCount) {
        server.requestDestroyConnection(id);
      };
  boost::asio::async_write(_socket, buffer, destroyConnectionFn);
}
