#include "client/Client.h"

#include <iostream>

#include "common/packets.h"


sc2tm::Client::Client(asio::io_service &service, std::string host, std::string port,
                      std::string botDir, std::string mapDir) :
    _socket(service) {
  // Generate our bot and map hashes.
  hashBotDirectory(botDir, botMap);
  hashMapDirectory(mapDir, mapMap);

  // TODO DEBUG
  for (const auto &info : botMap)
    std::cout << info.second << " - " << info.first.filename().string()  << "\n";

  for (const auto &info : mapMap)
    std::cout << info.second << " - " << info.first.filename().string()  << "\n";

  // Try to find a valid endpoint
  tcp::resolver resolver(service);
  tcp::resolver::query query(host, port);
  tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

  // TODO handle no endpoint?

  boost::asio::connect(_socket, endpoint_iterator);

  sendHandshake();
}

void sc2tm::Client::sendHandshake() {
  // Make a handshake packet from our data
  sc2tm::ClientHandshakePacket handshake(botMap, mapMap);

  // Put the handshake into our buffer.
  handshake.toBuffer(buffer);
  std::cout << "GOT BUFFER SIZE: " << buffer.size() << "\n"; // TODO debug

  auto noopFn = [&] (const boost::system::error_code& error, std::size_t bytes_transferred) {
    assert(buffer.size() == 0);
    std::cout << "GOT ERROR CODE " << error << "\n";
    std::cout << "WROTE " << bytes_transferred << " BYTES\n";

    auto noopFn2 = [] (const boost::system::error_code& error, std::size_t bytes_transferred) { };
    boost::asio::async_read(_socket, buffer, noopFn2);
  };

  // Async write our buffer
  boost::asio::async_write(_socket, buffer, noopFn);
}
