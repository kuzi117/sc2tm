#include "common/buffer_operations.h"

void sc2tm::writeUint32(uint32_t val, std::ostream &os) {
  val = htonl(val);
  os.write((const char *) &val, sizeof(val));
}

uint32_t sc2tm::readUint32(std::istream &is) {
  uint32_t value = 0;
  is.read((char *) &value, sizeof(value));
  return ntohl(value);
}
