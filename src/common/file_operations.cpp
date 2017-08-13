#include "common/file_operations.h"
#include "common/sha256.h"

#include <boost/iterator/filter_iterator.hpp>

#include <cassert>
#include <fstream>
#include <iostream>

namespace {

struct FileExtFilter {
  fs::path ext;
  FileExtFilter(const std::string &ext) : ext(ext) { } // Constructed with extension
  FileExtFilter() : ext("..") { } // Default constructor
  bool operator()(fs::path p) {
    if (!fs::is_regular_file(p))
      return false;
    else
      return ext == p.extension();
  }
};

typedef fs::recursive_directory_iterator rd_it;
typedef boost::filter_iterator<FileExtFilter, rd_it> frd_it;

void hashDirectory(frd_it it, sc2tm::SHAFileMap &map) {
  for (auto end = frd_it(); it != end; ++it) {
    assert(fs::is_regular_file(*it)); // Anything coming in here should be a regular file

    // Open our file
    fs::path filePath = it->path();
    std::cout << "SEE FILE: " << filePath << "\n"; // TODO DEBUG
    std::ifstream file(filePath.string(), std::fstream::in | std::fstream::binary);

    // Add its hash to our map
    map[filePath] = sha256(file);

    // Close the file
    file.close();
  }
}

} // End anonymous namespace

bool sc2tm::hashMapDirectory(std::string filepath, sc2tm::SHAFileMap &map) {
  // Make a path out of the string
  fs::path dir(filepath);

  // Check if it exists, if so canonicalize it
  if (fs::exists(dir))
    dir = fs::canonical(dir);
  else
    return false;

  // Make sure it's a directory
  if (!fs::is_directory(dir))
    return false;

  // TODO DEBUG
  std::cout << "MAP DIR: " << dir.string() << "\n";

  // Build up our filter iterator
  auto dirIt = rd_it(dir);
  auto it = boost::filter_iterator<FileExtFilter, rd_it>(FileExtFilter(".SC2Map"), dirIt);

  hashDirectory(it, map);

  return true;
}

bool sc2tm::hashBotDirectory(std::string filepath, SHAFileMap &map) {
  // Make a path out of the string

  // Check if it exists, if so canonicalize it
  fs::path dir(filepath);
  if (fs::exists(dir))
    dir = fs::canonical(dir);
  else
    return false;

  // Make sure it's a directory
  if (!fs::is_directory(dir))
    return false;

  // TODO DEBUG
  std::cout << "BOT DIR: " << dir.string() << "\n";

  // Platform specific shared object extension
#if defined(__linux__)
  std::string extension(".so");
#elif defined(_WIN32)
  std::string extension(".dll");
#elif define(__APPLE__)
  std::string extension(".dylib");
#endif

  // Build up our filter iterator
  auto dirIt = rd_it(dir);
  auto it = boost::filter_iterator<FileExtFilter, rd_it>(FileExtFilter(extension), dirIt);

  hashDirectory(it, map);

  return true;
}
