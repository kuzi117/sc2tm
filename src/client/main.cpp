#include <iostream>

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "common/CLOpts.h"
#include "common/config.h"
#include "common/file_operations.h"
#include "common/packets.h"

// Shorten the crazy long namespacing to asio tcp
using boost::asio::ip::tcp;

int main(int argc, char **argv) {
  // Parse out command line options
  sc2tm::CLOpts opts;
  if (!opts.parseOpts(argc, argv))
    return 0;

  try {
    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query("localhost", sc2tm::serverPortStr);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    tcp::socket socket(io_service);
    boost::asio::connect(socket, endpoint_iterator);

    sc2tm::SHAFileMap mapMap;
    sc2tm::SHAFileMap botMap;
    sc2tm::hashMapDirectory(opts.getOpt("maps"), mapMap);
    sc2tm::hashBotDirectory(opts.getOpt("bots"), botMap);

    for (const auto &mapInfo : mapMap)
      std::cout << mapInfo.first.filename() << " SHA256: " << mapInfo.second << "\n";

    for (const auto &botInfo : botMap)
      std::cout << botInfo.first.filename() << " SHA256: " << botInfo.second << "\n";

    // Make a handshake packet from the
    sc2tm::ClientHandshakePacket handshake(botMap, mapMap);

    // Write the packet to the buffer
    boost::asio::streambuf buffer;
    handshake.toBuffer(buffer);

    auto noopFn = [&] (const boost::system::error_code& error, std::size_t bytes_transferred) {
      assert(buffer.size() == 0);
    };
    boost::asio::async_write(socket, buffer, noopFn);
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
