#ifndef SC2TM_FILE_OPERATIONS_H
#define SC2TM_FILE_OPERATIONS_H

#include <map>
#include <experimental/filesystem>
// Windows has some extra filesystem header..
// https://docs.microsoft.com/en-us/cpp/standard-library/filesystem
#ifdef _WIN32
#include <filesystem>
#endif

#include "common/sha256.h"

namespace fs = std::experimental::filesystem;

namespace sc2tm {

typedef std::map<fs::path, SHA256Hash::ptr> SHAFileMap;

bool hashMapDirectory(std::string filepath, SHAFileMap &map);
bool hashBotDirectory(std::string filepath, SHAFileMap &map);

}

#endif //SC2TM_FILE_OPERATIONS_H
