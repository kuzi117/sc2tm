#include <iostream>

#include <boost/asio.hpp>


#include "client/Client.h"
#include "common/CLOpts.h"
#include "common/config.h"

// Shorten the crazy long namespacing to asio tcp
using boost::asio::ip::tcp;


int main(int argc, char **argv) {
  // Parse out command line options
  sc2tm::CLOpts opts;
  if (!opts.parseOpts(argc, argv))
    return 0;

  try {
    boost::asio::io_service service;
    sc2tm::Client s(service, "localhost", sc2tm::serverPortStr, opts.getOpt("bots"),
                    opts.getOpt("maps"));
    service.run();
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
