#ifndef SC2TM_CLOPTS_H
#define SC2TM_CLOPTS_H

#include <string>
#include <map>

namespace sc2tm {

struct OptionInfo {
  std::string desc;
  bool req;
  bool flag;

  OptionInfo(std::string d, bool r, bool f) : desc(d), req(r), flag(f) { }
  OptionInfo() : desc(""), req(false), flag(false) { }
};

//! Class that handles command line options.
class CLOpts {
public:
  CLOpts();

  //! Parse command line options.
  /**
   * Parse command line options for the tournament manager.
   *
   * @param argc Argument count.
   * @param argv Arguments list.
   * @return True if opts were parse successfully, false otherwise.
   */
  bool parseOpts(int argc, char **argv);

  //! Prints usage message.
  void usage();

protected:
  //! Register a new option to be parsed
  /**
   *
   * @param name The option's name.
   * @param description Description of the option.
   * @param flag Is this option a flag (doesn't require an argument.
   * @param require Is this option required.
   */
  void registerOption(std::string name, std::string description, bool require = true);
  void registerFlag(std::string name, std::string description);

  std::string usageHeader;

private:
  // Options info
  std::map<std::string, OptionInfo> options;
  int requiredCount = 0;

  //  Results map
  std::map<std::string, std::string> optionResults;
  std::map<std::string, bool> flagResults;
};

} // End sc2tm namespace

#endif //SC2TM_CLOPTS_H
