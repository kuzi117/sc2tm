#ifndef SC2TM_CLIENT_H
#define SC2TM_CLIENT_H

#include "common/file_operations.h"

#include <boost/asio.hpp>

using namespace boost;
using boost::asio::ip::tcp;

namespace sc2tm {

class Client {
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

  //! The TCP socket this client is connected on.
  tcp::socket _socket;

  //! The buffer this client uses.
  boost::asio::streambuf buffer;

public:
  //! Get the TCP socket this client is connected on.
  tcp::socket& socket()
  {
    return _socket;
  }

  //! Construct a Client and attempt to connect.
  /**
   * Construct a Client associated with an io_service. Try to connect and send handshake packet
   * as well.
   * @param service The io_service to associate with.
   * @param host The host to connect to.
   * @param port The port to connect to.
   * @param botDir The directory that contains bots for this client.
   * @param mapDir The directory that contains maps for this client.
   */
  Client(asio::io_service &service, std::string host, std::string port, std::string botDir,
         std::string mapDir);

private:
  // State functions
  //! Send the client handshake to the server.
  void sendHandshake();
};

} // End sc2tm namespace

#endif //SC2TM_CLIENT_H
