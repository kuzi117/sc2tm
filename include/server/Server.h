#ifndef SC2TM_SERVER_H
#define SC2TM_SERVER_H

#include <boost/asio.hpp>

#include "common/config.h"
#include "server/Connection.h"

using namespace boost;
using boost::asio::ip::tcp;

namespace sc2tm {

class Server {
  tcp::acceptor acceptor;

public:
  Server(asio::io_service &service) : acceptor(service, tcp::endpoint(tcp::v4(), serverPort)) {
    startAccept();
  }

private:
  //! Start accepting new connections asynchronously.
  void startAccept();

  //! Handle accepting a new connection
  void handleAccept(Connection::ptr newCon, const boost::system::error_code& error);

};

} // End namespace sc2tm

#endif //SC2TM_SERVER_H
