#include <fstream>

#include "common/buffer_operations.h"

namespace {
  const int bufferSize = 2048;
}

sc2tm::BuffOpsErr sc2tm::writeFileToSocket(std::string filepath, int sockfd) {
  // Try to open the file
  std::ifstream f(filepath, std::ios::in | std::ios::binary);
  if (!f.good())
    return NOFILE;

  char buffer[bufferSize] = {};
  f.get(buffer, bufferSize);
}
