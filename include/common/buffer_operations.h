#ifndef SC2TM_BUFFER_OPERATIONS_H
#define SC2TM_BUFFER_OPERATIONS_H

#include <string>

namespace sc2tm {

enum BuffOpsErr : uint8_t {
  GOOD = 0, // No error
  NOFILE, // File doesn't exist
};

BuffOpsErr writeFileToSocket(std::string filepath, int sockfd);

} // End namespace sc2tm

#endif //SC2TM_BUFFER_OPERATIONS_H
