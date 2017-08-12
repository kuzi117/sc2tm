#ifndef SC2TM_SERVEROPTS_H
#define SC2TM_SERVEROPTS_H

#include "common/CLOpts.h"
#include "common/config.h"

namespace sc2tm {

class ServerOpts : public CLOpts {
public:
  ServerOpts() : CLOpts() {
    usageHeader = "Starcraft 2 Tournament Manager Server v" + sc2tm::serverVersionStr;
  }

private:
};

}

#endif //SC2TM_SERVEROPTS_H
