#include "server/Server.h"
#include "server/ServerOpts.h"

int main(int argc, char **argv) {
  // Parse out command line options
  sc2tm::ServerOpts opts;
  if (!opts.parseOpts(argc, argv))
    return 0;

  boost::asio::io_service service;
  sc2tm::Server s(service);
  service.run();

  return 0;
}
