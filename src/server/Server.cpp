#include "server/Server.h"

#include <iostream>

sc2tm::Server::Server(asio::io_service &service, std::string botDir, std::string mapDir) :
    acceptor(service, tcp::endpoint(tcp::v4(), serverPort)) {
  hashBotDirectory(botDir, botMap);
  hashMapDirectory(mapDir, mapMap);

  // TODO DEBUG
  for (const auto &info : botMap)
    std::cout << info.second << " - " << info.first.filename().string()  << "\n";

  for (const auto &info : mapMap)
    std::cout << info.second << " - " << info.first.filename().string()  << "\n";

  startAccept();
}

void sc2tm::Server::startAccept() {
  // Create a new connection
  Connection::ConnId id = nextId++; // Generate id, we need to use it twice
  Connection::ptr newConn =
      Connection::create(*this, acceptor.get_io_service(), id);

  // Locking scope
  {
    std::lock_guard<std::mutex> lock(connMutex);
    conns[id] = newConn;
  }

  auto acceptFn =
      [&, newConn] (const boost::system::error_code &error) {
        handleAccept(*newConn, error);
      };

  acceptor.async_accept(newConn->socket(), acceptFn);
}

void sc2tm::Server::handleAccept(Connection &newCon, const boost::system::error_code &error) {
  if (!error)
    newCon.start();

  startAccept();
}

void sc2tm::Server::requestDestroyConnection(Connection::ConnId id) {
  std::cout << "ERASING CONNECTION " << id << '\n';
  std::lock_guard<std::mutex> lock(connMutex);
  size_t erased = conns.erase(id);
  assert(erased == 1);
}
