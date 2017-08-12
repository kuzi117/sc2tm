#include <iostream>

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "common/config.h"

// Shorten the crazy long namespacing to asio tcp
using boost::asio::ip::tcp;

int main() {

  try
  {
    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query("localhost", sc2tm::serverPortStr);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    tcp::socket socket(io_service);
    boost::asio::connect(socket, endpoint_iterator);

    boost::array<char, 128> buf;
    boost::system::error_code error;

    size_t len = socket.read_some(boost::asio::buffer(buf), error);

    std::cout << "READ: ";
    std::cout.write(buf.data(), len) << std::endl;
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
