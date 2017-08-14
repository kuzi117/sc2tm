#include "server/Server.h"

#include <iostream>

sc2tm::Server::Server(asio::io_service &service, const std::string &botDir,
                      const std::string &mapDir) :
    acceptor(service, tcp::endpoint(tcp::v4(), serverPort)) {
  // Generate our directory hashes
  // TODO do these really need to map from file to hash on the server? Not really...
  hashBotDirectory(botDir, botMap);
  hashMapDirectory(mapDir, mapMap);

  // Initialize the generator
  gen = GameGenerator(botMap, mapMap);

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
