#ifndef SC2TM_CONNECTION_H
#define SC2TM_CONNECTION_H

#include <boost/asio.hpp>

#include <memory>

using namespace boost;
using boost::asio::ip::tcp;

namespace sc2tm {

//! Forward declare PregameDisconnectReason.
enum PregameDisconnectReason : uint8_t;

//! Represents a client's connection to the server.
class Connection {
  //! The TCP socket this client is connected on.
  tcp::socket _socket;

  //! The buffer this client uses.
  boost::asio::streambuf buffer;

public:
  //! Convenience typedef for a connection shared ptr.
  typedef std::shared_ptr<Connection> ptr;

  //! Create a new Connection and get a ptr to it.
  static ptr create(asio::io_service &service) {
    return std::shared_ptr<Connection>(new Connection(service));
  }

  //! Get the TCP socket this client is connected on.
  tcp::socket& socket()
  {
    return _socket;
  }

  //! Start state function.
  void start();

  //! Deconstruct a Connection.
  ~Connection();

private:
  //! Construct a Connection associated with an io_service.
  Connection(asio::io_service &service) : _socket(service) { }

  // State functions
  //! Read the client handshake.
  void readHandshake();

  // Pregame state functions
  //! Send a PregameDisconnect
  void sendPregameDisconnect(PregameDisconnectReason reason);

};

} // End namespace sc2tm


#endif //SC2TM_CONNECTION_H
