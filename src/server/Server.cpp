#include "server/Server.h"

#include <iostream>

#include <boost/bind.hpp>

void sc2tm::Server::startAccept() {
  Connection::ptr newConn =
      Connection::create(acceptor.get_io_service());

  auto acceptFn = boost::bind(&sc2tm::Server::handleAccept, this, newConn,
                              boost::asio::placeholders::error);
   acceptor.async_accept(newConn->socket(), acceptFn);
}

void sc2tm::Server::handleAccept(Connection::ptr newCon, const boost::system::error_code &error) {
  if (!error) {
    newCon->start();
  }

  startAccept();
}
