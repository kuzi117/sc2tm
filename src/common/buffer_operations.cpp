#include "common/buffer_operations.h"
#include "common/sha256.h"

void sc2tm::writeUint32(uint32_t val, std::ostream &os) {
  val = htonl(val);
  os.write((const char *) &val, sizeof(val));
}

uint32_t sc2tm::readUint32(std::istream &is) {
  uint32_t value = 0;
  is.read((char *) &value, sizeof(value));
  return ntohl(value);
}

void sc2tm::writeHashBuffer(const uint8_t * const buffer, std::ostream &os) {
  os.write((const char *) buffer, SHA256::DIGEST_SIZE);
}

void sc2tm::readHashBuffer(uint8_t *buffer, std::istream &is) {
  is.read((char *) buffer, SHA256::DIGEST_SIZE);
}
