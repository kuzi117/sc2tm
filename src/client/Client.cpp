#include "client/Client.h"

#include "common/packets.h"

#include <iostream>

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
  std::cout << "GOT BUFFER SIZE: " << buffer.size() << "\n"; // TODO DEBUG


  // Build the function that will respond to the write being done by starting a wait for a read.
  auto waitPregameCmdFn =
      [&] (const boost::system::error_code& error, std::size_t byteCount) {
        assert(error == boost::system::errc::success); // TODO handle error
        assert(buffer.size() == 0);

        // Build the function that will respond to the buffer being filled with the server's
        // response, which will be some PregameCommand.
        auto readPregameCommandFn =
            [&] (const boost::system::error_code& error2, std::size_t byteCount2) {
              assert(error2 == boost::system::errc::success); // TODO handle error
              assert(byteCount2 == sizeof(PregameCommand));
              readPregameCommand();
            };

        // Async wait for the buffer to be filled with a PregameCommand. Respond by calling the
        // function that reads PregameCommands.
        boost::asio::async_read(_socket, buffer,
                                boost::asio::transfer_exactly(sizeof(PregameCommand)),
                                readPregameCommandFn);
      };

  // Async wait for the client handshake data to be written. Respond by waiting for a response from
  // the server, namely a PregameCommand.
  boost::asio::async_write(_socket, buffer, waitPregameCmdFn);
}

void sc2tm::Client::readPregameCommand() {
  PregameCommandPacket p(buffer);
  std::cout << "GOT PREGAME COMMAND: " << p.cmd << '\n'; // TODO debug

  switch (p.cmd) {
  case DISCONNECT: {
    // Make wait for reason function
    auto waitForReasonFn =
        [&] (const boost::system::error_code& error, std::size_t byteCount)  {
          assert(error == boost::system::errc::success); // TODO handle error
          assert(byteCount == sizeof(PregameDisconnectPacket));
          // Once we have the data we can handle it
          readPregameDisconnectReason();
        };
    boost::asio::async_read(_socket, buffer,
                            boost::asio::transfer_exactly(sizeof(PregameDisconnectPacket)),
                            waitForReasonFn);

    break;
  }
  case START_GAME: break; // TODO start game
  default:
    assert(false); // TODO handle bad command?
  }
}

void sc2tm::Client::readPregameDisconnectReason() {
  PregameDisconnectPacket p(buffer);
  std::cout << "GOT PREGAME DISCONNECT REASON: " << p.reason << '\n';

  // Note that we don't schedule any work here thus the ioservice will have no more work and should
  // end the run loop

  // TODO print useful disconnect message.
}
