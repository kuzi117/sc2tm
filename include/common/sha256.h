/*
 * Updated to C++, zedwood.com 2012
 * Based on Olivier Gay's version
 * See Modified BSD License below:
 *
 * FIPS 180-2 SHA-224/256/384/512 implementation
 * Issue date:  04/30/2005
 * http://www.ouah.org/ogay/sha2/
 *
 * Copyright (C) 2005, 2007 Olivier Gay <olivier.gay@a3.epfl.ch>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef SHA256_H
#define SHA256_H

// Taken from http://www.zedwood.com/article/cpp-sha256-function
// Significant changes (Braedy Kuzma):
//   - removed std::string sha256(std::string input)
//   - added SHA256Hash class which is a light wrapper around a digest
//   - added SHA256Hash::ptr sha256(std::ifstream &file)
//   - added ostream overloads for printing SHA256Hash

#include <memory>
#include <set>


class SHA256
{
protected:
    typedef unsigned char uint8;
    typedef unsigned int uint32;
    typedef unsigned long long uint64;

    const static uint32 sha256_k[];
    static const unsigned int SHA224_256_BLOCK_SIZE = (512/8);
public:
    void init();
    void update(const unsigned char *message, unsigned int len);
    void final(unsigned char *digest);
    static const unsigned int DIGEST_SIZE = ( 256 / 8);

protected:
    void transform(const unsigned char *message, unsigned int block_nb);
    unsigned int m_tot_len;
    unsigned int m_len;
    unsigned char m_block[2*SHA224_256_BLOCK_SIZE];
    uint32 m_h[8];
};

// TODO Test the comparison function
//! Light wrapper around SHA256 digest.
class SHA256Hash {
  //! The actual digest bits.
  uint8_t buff[SHA256::DIGEST_SIZE];
public:
  //! Pointer to a hash.
  typedef std::shared_ptr<SHA256Hash> ptr;

  //! Default constructor.
  SHA256Hash() : buff() { }

  //! Constructor from uint8_t array
  SHA256Hash(const uint8_t * const bytes);

  //! Operator to offer convenient buffer access.
  uint8_t &operator[](const uint8_t i) { return buff[i]; }

  //! Operator to offer convenient buffer access.
  const uint8_t &operator[](const uint8_t i) const { return buff[i]; }

  //! Offers access to the underlying digest.
  uint8_t *get() { return (uint8_t *) &buff; }

  //! Compare function for SHA256hashes.
  static int compare(const SHA256Hash &hash1, const SHA256Hash &hash2);

  //! Convenience compare between two ptrs.
  static int compare(const ptr &hash1, const ptr &hash2) { compare(*hash1, *hash2); }

public:
};

struct CompareHashPtrFtor {
  int operator()(const SHA256Hash::ptr p0, const SHA256Hash::ptr p1) const {
    return SHA256Hash::compare(p0, p1);
  }
};

//! A set of hashes.
/**
 * A set of hashes. Ensures that the underlying hashes are unique rather than the pointers.
 */
typedef std::set<SHA256Hash::ptr, CompareHashPtrFtor> HashSet;

//! Generates a pointer to SHA256Hash from a file.
SHA256Hash::ptr sha256(std::ifstream &file);

//! Prints a SHA256Hash by pointer (delegates to the reference version).
std::ostream &operator<<(std::ostream &os, const SHA256Hash::ptr &hashp);

//! Prints a SHA256Hash.
std::ostream& operator<<(std::ostream& os, const SHA256Hash &hash);

#define SHA2_SHFR(x, n)    (x >> n)
#define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))
#define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
#define SHA2_UNPACK32(x, str)                 \
{                                             \
    *((str) + 3) = (uint8) ((x)      );       \
    *((str) + 2) = (uint8) ((x) >>  8);       \
    *((str) + 1) = (uint8) ((x) >> 16);       \
    *((str) + 0) = (uint8) ((x) >> 24);       \
}
#define SHA2_PACK32(str, x)                   \
{                                             \
    *(x) =   ((uint32) *((str) + 3)      )    \
           | ((uint32) *((str) + 2) <<  8)    \
           | ((uint32) *((str) + 1) << 16)    \
           | ((uint32) *((str) + 0) << 24);   \
}
#endif
