#include "common/CLOpts.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <list>

namespace {

void printOption(std::string name, sc2tm::OptionInfo info) {
  if (info.flag)
    std::cout << "    -" << name << "  -  " << info.desc << '\n';
  else
    std::cout << "    --" << name << "  -  " << info.desc << " "
        << (info.req ? "(required)" : "(not required)") << "\n";

}

} // End anonymous namespace

sc2tm::CLOpts::CLOpts() {
  registerOption("maps", "Directory containing maps");
  registerOption("bots", "Directory containing bots");
}

void sc2tm::CLOpts::registerOption(std::string name, std::string description, bool require) {
  assert(options.find(name) == options.end()); // Don't overwrite options
  options[name] = OptionInfo(description, require, false);
  if (require)
    ++requiredCount;
}

bool sc2tm::CLOpts::parseOpts(int argc, char **argv) {
  // Shift off exe name
  --argc;
  ++argv;

  // TODO DEBUG
//  std::cout << "GOT ARG COUNT: " << argc << '\n';

  // This assumes the minimum when all options are in the format --name=value and no flags are
  // specified
  if (argc < requiredCount) {
    usage();
    return false;
  }

  // Convert all args into strings in vector
  std::list<std::pair<std::string, std::string>> args;
  for (int i = 0; i < argc; ++i) {
    // Get pointer to string
    char *argp = argv[i];

    // If it starts with a - then it's some sort of value, just put it in the list
    if (argp[0] != '-') {
      args.emplace_back(argp, "");
      continue;
    }

    // Remove up to 2 leading - from options
    for (int j = 0; j < 2 && argp[0] == '-'; ++j, ++argp);

    // Make our arg string
    std::string arg(argp);

    // Does the string contain =
    size_t eqIt = arg.find("=");

    // If it does, we want to split it
    if (eqIt != std::string::npos)
      args.emplace_back(arg.substr(0, eqIt), arg.substr(eqIt+2, arg.length()));
      // If it doesn't then the second half is empty
    else
      args.emplace_back(arg, "");
  }

  // TODO DEBUG
//  for (auto arg : args)
//    std::cout << "GOT ARG " << arg.first << " " << arg.second << "\n";

  // Find one of our options
  for (const auto &option : options) {
    auto it = std::find_if(args.begin(), args.end(),
                           [&] (std::pair<std::string, std::string> arg) {
                             std::string name = option.first;
                             return arg.first.compare(0, name.length(), name) == 0;
                           });

    if (it == args.end())
      continue;

    std::cout << "FOUND " << it->first << " " << it->second << "\n";

    // If it's a flag, add it to the flag results and drop the arg
    if (option.second.flag) {
      flagResults[option.first] = true;
      args.erase(it);
    }
    // If it's an option, check to see if it's in the form --name=value and drop one arg
    else if (it->second != "") {
      optionResults[it->first] = it->second;
      args.erase(it);
    }
    // It's in the form --name value, drop 2 args
    else {
      // Try to get the next arg, if it doesn't work, just skip it and let it fail later
      auto next = std::next(it);
      if (next == args.end())
        continue;

      // Note that if value didn't start with a - then it's guaranteed to be contained entirely in
      // first. If it did start with a dash, poo on the user.
      optionResults[it->first] = next->first;

      // Delete this value and the next
      it = args.erase(it);
      args.erase(it);
    }
  }

  // Now do bookkeeping
  for (const auto &option : options) {
    // Verify that we found all of the required options
    if (option.second.req) {
      // Check option results only, flags are never required
      if (optionResults.find(option.first) == optionResults.end()) {
        std::cout << "Missing option: " << option.first << "\n";
        return false;
      }
    }

    // Set all unfound flags to false
    if (option.second.flag && flagResults.find(option.first) == flagResults.end())
      flagResults[option.first] = false;
  }

  return true;
}

void sc2tm::CLOpts::usage() {
  std::cout << usageHeader << "\n\n";


  for(auto option : options)
    printOption(option.first, option.second);
}
