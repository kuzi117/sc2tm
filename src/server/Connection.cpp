#include "server/Connection.h"

#include <iostream>

#include <boost/bind.hpp>

using namespace std::placeholders;

sc2tm::Connection::~Connection() {
  std::cout << "CONNECTION DYING\n";
};

void sc2tm::Connection::start() {
  buffer = "hello";
  std::cout << "Writing: " << buffer << std::endl;

  auto noopFn =
      [] (const boost::system::error_code& error, std::size_t bytesSent) { };

  boost::asio::async_write(_socket, boost::asio::buffer(buffer), noopFn);
}
