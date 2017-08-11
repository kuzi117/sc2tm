#include "server/Server.h"

int main() {

  boost::asio::io_service service;
  sc2tm::Server s(service);
  service.run();

  return 0;
}
