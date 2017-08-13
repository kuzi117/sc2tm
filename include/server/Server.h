#ifndef SC2TM_SERVER_H
#define SC2TM_SERVER_H

#include <boost/asio.hpp>

#include "common/config.h"
#include "common/file_operations.h"
#include "server/Connection.h"

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

  //! The list of active connections.
  /**
   * The list of active connections. Careful care needs to be taken to ensure there aren't race
   * conditions when updating this.
   */
  std::vector<Connection::ptr> conns;

public:
  //! Construct a server.
  /**
   * Construct a server that handles sending clients SC2 games to play in a tournament.
   *
   * @param service The io service this server runs on.
   * @param botDir The directory where the bots are located.
   * @param mapDir The directory where the maps are located.
   */
  Server(asio::io_service &service, std::string botDir, std::string mapDir);

private:
  //! Start accepting new connections asynchronously.
  void startAccept();

  //! Handle accepting a new connection.
  void handleAccept(Connection::ptr newCon, const boost::system::error_code& error);

};

} // End namespace sc2tm

#endif //SC2TM_SERVER_H
