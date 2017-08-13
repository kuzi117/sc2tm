#include "server/Server.h"

#include <iostream>

sc2tm::Server::Server(asio::io_service &service, std::string botDir, std::string mapDir) :
    acceptor(service, tcp::endpoint(tcp::v4(), serverPort)) {
  hashBotDirectory(botDir, botMap);
  hashMapDirectory(mapDir, mapMap);

  for (const auto &botInfo : botMap)
    std::cout << botInfo.first.filename() << " SHA256: " << botInfo.second << "\n";

  for (const auto &mapInfo : mapMap)
    std::cout << mapInfo.first.filename() << " SHA256: " << mapInfo.second << "\n";

  startAccept();
}

void sc2tm::Server::startAccept() {
  // Create a new connection
  Connection::ptr newConn =
      Connection::create(acceptor.get_io_service());

  // Add it to the list of connections so it won't die..
  conns.push_back(newConn);

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
