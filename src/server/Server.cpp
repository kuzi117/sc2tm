#include "server/Server.h"

sc2tm::Server::Server(asio::io_service &service) :
    acceptor(service, tcp::endpoint(tcp::v4(), serverPort)) {
  startAccept();
}

void sc2tm::Server::startAccept() {
  Connection::ptr newConn =
      Connection::create(acceptor.get_io_service());

  auto acceptFn =
      [=] (const boost::system::error_code &error) {
        handleAccept(newConn, error);
      };

  acceptor.async_accept(newConn->socket(), acceptFn);
}

void sc2tm::Server::handleAccept(Connection::ptr newCon, const boost::system::error_code &error) {
  if (!error) {
    newCon->start();
  }

  startAccept();
}
