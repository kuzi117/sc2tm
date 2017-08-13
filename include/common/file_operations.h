#ifndef SC2TM_FILE_OPERATIONS_H
#define SC2TM_FILE_OPERATIONS_H

#include "common/sha256.h"

#include <experimental/filesystem>
#include <map>

// Windows has some extra filesystem header..
// https://docs.microsoft.com/en-us/cpp/standard-library/filesystem
#ifdef _WIN32
#include <filesystem>
#endif


namespace fs = std::experimental::filesystem;

namespace sc2tm {

//! Convenience typedef for mapping a file to a SHA256 hash.
typedef std::map<fs::path, SHA256Hash::ptr> SHAFileMap;

//! Hash all .so files in a directory.
bool hashBotDirectory(std::string filepath, SHAFileMap &map);
//! Hash all .SC2Map files in a directory.
bool hashMapDirectory(std::string filepath, SHAFileMap &map);

} // End sc2tm namespace

#endif //SC2TM_FILE_OPERATIONS_H
