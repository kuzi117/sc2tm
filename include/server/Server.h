#ifndef SC2TM_SERVER_H
#define SC2TM_SERVER_H

#include "common/config.h"
#include "common/file_operations.h"
#include "server/Connection.h"
#include "server/GameGenerator.h"

#include <boost/asio.hpp>

#include <mutex>
#include <map>

using namespace boost;
using boost::asio::ip::tcp;

namespace sc2tm {

//! Represents a server that clients connect to.
/**
 * Represents a server that clients connect to. Handles organizing games, distributing them to
 * clients to play, and aggregating the results.
 */
class Server {
  //! The TCP acceptor.
  tcp::acceptor acceptor;

  //! The map for maps that are involved in this run.
  /**
   * The map for maps that are involved in this run. This should only be built once, at server
   * start up, and never modified again, so accessing this should be safe.
   */
  SHAFileMap mapMap;

  //! The map for bots that are involved in this run.
  /**
   * The map for bots that are involved in this run. This should only be built once, at server
   * start up, and never modified again, so accessing this should be safe.
   */
  SHAFileMap botMap;

  //! The id that will be give to the next incoming connection.
  /**
   * The id that will be give to the next incoming connection. Careful care needs to be taken to
   * ensure there aren't race conditions when updating this. Use connMutex to ensure single
   * accesses.
   */
  // I decided against a second mutex because I can't think of a reason that you'd ever need to
  // access nextId unless you were already editing the list of active connections.
  Connection::ConnId nextId = 0;

  //! The list of active connections.
  /**
   * The list of active connections. Careful care needs to be taken to ensure there aren't race
   * conditions when updating this. Use connMutex to ensure single accesses.
   */
  std::map<Connection::ConnId, Connection::ptr> conns;

  //! Lock for editing the list of active connections.
  std::mutex connMutex;

  //! The game generator.
  GameGenerator gen;

public:
  //! Construct a server.
  /**
   * Construct a server that handles sending clients SC2 games to play in a tournament.
   *
   * @param service The io service this server runs on.
   * @param botDir The directory where the bots are located.
   * @param mapDir The directory where the maps are located.
   */
  Server(asio::io_service &service, const std::string &botDir, const std::string &mapDir);

  //! Declare Connection as a friend class.
  /**
   * Declare Connection as a friend class so that it can request to be destroyed.
   */
  friend Connection;

private:
  //! Start accepting new connections asynchronously.
  void startAccept();

  //! Handle accepting a new connection.
  void handleAccept(Connection &newCon, const boost::system::error_code& error);

  //! Request that a connection be destroyed.
  void requestDestroyConnection(Connection::ConnId id);
};

} // End namespace sc2tm

#endif //SC2TM_SERVER_H
