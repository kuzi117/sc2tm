#ifndef SC2TM_SERVER_INFO_H
#define SC2TM_SERVER_INFO_H

namespace sc2tm {

// Create server version
//! Server major version number.
const uint8_t serverMajorVersion = 0;
//! Server minor version number.
const uint8_t serverMinorVersion = 1;
//! Server patch version number.
const uint8_t serverPatchVersion = 0;
//! Server version number as a dot separated string.
const std::string serverVersionStr = std::to_string(serverMajorVersion) + '.' +
                                     std::to_string(serverMinorVersion) + '.' +
                                     std::to_string(serverPatchVersion);

// Create client version
//! Client major version number.
const uint8_t clientMajorVersion = 0;
//! Client minor version number.
const uint8_t clientMinorVersion = 1;
//! Client patch version number.
const uint8_t clientPatchVersion = 0;
//! Client version number as a dot separated string.
const std::string clientVersionStr = std::to_string(clientMajorVersion) + '.' +
                                     std::to_string(clientMinorVersion) + '.' +
                                     std::to_string(clientPatchVersion);

// Server info
//! The port the server will be listening on.
const short serverPort = 5122;
//! The server port as a string.
const std::string serverPortStr = std::to_string(serverPort);

} // End namespace sc2tm

#endif //SC2TM_SERVER_INFO_H
