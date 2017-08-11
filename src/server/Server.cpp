#include "server/Server.h"

#include <iostream>

#include <boost/asio.hpp>

#include "common/server_info.h"

// Shorten the crazy long namespacing to asio tcp
using boost::asio::ip::tcp;

void sc2tm::Server::run() {
  try {
    // Set up our service and tcp socket
    boost::asio::io_service service;
    tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), sc2tm::serverPort));
    tcp::socket socket(service);
    acceptor.accept(socket);

    // Send our message
    std::string message = "Hello";
    boost::system::error_code ignored_error;
    boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
