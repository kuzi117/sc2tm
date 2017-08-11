#include "server/Connection.h"

#include <iostream>

#include <boost/bind.hpp>

using namespace std::placeholders;

void sc2tm::Connection::start() {
  buffer = "hello";
  std::cout << "Writing: " << buffer << std::endl;

  auto noopFn = boost::bind(&Connection::noop, shared_from_this(), boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred);

  boost::asio::async_write(_socket, boost::asio::buffer(buffer), noopFn);
}
