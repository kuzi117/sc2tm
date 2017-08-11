#ifndef SC2TM_CONNECTION_H
#define SC2TM_CONNECTION_H

#include <memory>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

using namespace boost;
using boost::asio::ip::tcp;

namespace sc2tm {

class Connection : public std::enable_shared_from_this<Connection> {
  tcp::socket _socket;
  std::string buffer;

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

private:
  Connection(asio::io_service &service) : _socket(service) { }

  // State Functions
  void noop(const boost::system::error_code& error, std::size_t bytesSent) { }


};

} // End namespace sc2tm


#endif //SC2TM_CONNECTION_H
