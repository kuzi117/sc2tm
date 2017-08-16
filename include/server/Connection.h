#ifndef SC2TM_CONNECTION_H
#define SC2TM_CONNECTION_H

#include "common/Game.h"
#include "common/sha256.h"

#include <boost/asio.hpp>

#include <memory>

using namespace boost;
using boost::asio::ip::tcp;

namespace sc2tm {

//! Forward declare Server.
class Server;

//! Forward declare PregameDisconnectReason.
enum PregameDisconnectReason : uint8_t;

//! Represents a client's connection to the server.
class Connection {
  //! Typedef internally first so we can use it privately.
  typedef uint32_t ConnId_;

  //! The server this connection is associated with.
  Server &server;

  //! The TCP socket this client is connected on.
  tcp::socket _socket;

  //! The buffer this client uses.
  boost::asio::streambuf buffer;

  //! This connection's id.
  ConnId_ id;

  //! This client's available bots.
  HashSet bots;

  //! This client's available maps.
  HashSet maps;

  //! This connection's currently playing game.
  Game game;

public:
  //! Convenience typedef for a connection shared ptr.
  typedef std::shared_ptr<Connection> ptr;

  //! Type to use for the connection id.
  typedef ConnId_ ConnId;

  //! Create a new Connection and get a ptr to it.
  static ptr create(Server &server, asio::io_service &service, ConnId id) {
    return std::shared_ptr<Connection>(new Connection(server, service, id));
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
  Connection(Server &server, asio::io_service &service, ConnId id) :
      server(server), _socket(service), id(id){ }

  // State functions
  //! Read the client handshake.
  void readHandshake();
  //! Schedule a game and send to the client.
  void scheduleGame();
  //! Send a PregameDisconnect.
  void sendPregameDisconnect(PregameDisconnectReason reason);
  //! Send the client a game to play.
  void sendStartGame();
  //! Read the game status.
  void readGameStatus();

};

} // End namespace sc2tm


#endif //SC2TM_CONNECTION_H
