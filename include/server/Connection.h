#ifndef SC2TM_CONNECTION_H
#define SC2TM_CONNECTION_H

#include <memory>

#include <boost/asio.hpp>

using namespace boost;
using boost::asio::ip::tcp;

namespace sc2tm {

class Connection {
  tcp::socket _socket;
  boost::asio::streambuf buffer;

public:
  typedef std::shared_ptr<Connection> ptr;

  static ptr create(asio::io_service &service) {
    return std::shared_ptr<Connection>(new Connection(service));
  }

  tcp::socket& socket()
  {
    return _socket;
  }

  //! Start state function
  void start();

  ~Connection();

private:
  Connection(asio::io_service &service) : _socket(service) { }

  // State functions
  void readHandshake();

};

} // End namespace sc2tm


#endif //SC2TM_CONNECTION_H
