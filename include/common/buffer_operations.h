#ifndef SC2TM_BUFFER_OPERATIONS_H
#define SC2TM_BUFFER_OPERATIONS_H

#ifdef  _WIN32
#include <Winsock32.h>
#else
#include <arpa/inet.h>
#endif
#include <iostream>

namespace sc2tm {

//! Write a uint32_t to a stream in network byte order.
void writeUint32(uint32_t val, std::ostream &os);

//! Read a uin32_t from a stream in network byte order.
uint32_t readUint32(std::istream &is);

//! Write a hash buffer to a stream.
void writeHashBuffer(const uint8_t * const buffer, std::ostream &os);

//! Read a hash buffer from a stream.
void readHashBuffer(uint8_t *buffer, std::istream &is);

} // End namespace sc2tm

#endif //SC2TM_BUFFER_OPERATIONS_H
