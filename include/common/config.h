#ifndef SC2TM_SERVER_INFO_H
#define SC2TM_SERVER_INFO_H

namespace sc2tm {

// Create server version
const uint8_t serverMajorVersion = 0;
const uint8_t serverMinorVersion = 1;
const uint8_t serverPatchVersion = 0;
const std::string serverVersionStr = std::to_string(serverMajorVersion) + '.' +
                                     std::to_string(serverMinorVersion) + '.' +
                                     std::to_string(serverPatchVersion);

// Create client version
const uint8_t clientMajorVersion = 0;
const uint8_t clientMinorVersion = 1;
const uint8_t clientPatchVersion = 0;
const std::string clientVersionStr = std::to_string(clientMajorVersion) + '.' +
                                     std::to_string(clientMinorVersion) + '.' +
                                     std::to_string(clientPatchVersion);

// Server info
const short serverPort = 5122;
const std::string serverPortStr = std::to_string(serverPort);

} // End namespace sc2tm

#endif //SC2TM_SERVER_INFO_H
