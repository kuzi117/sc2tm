#include "server/Connection.h"

#include <iostream>
#include <common/packets.h>

#include "common/buffer_operations.h"

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
        std::cout << buffer.size() << "\n";
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
}
