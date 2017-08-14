set(
  common_src
    common/buffer_operations.cpp
    common/CLOpts.cpp
    common/file_operations.cpp
    common/packets.cpp
    common/sha256.cpp
)

set(
  client_src
    client/main.cpp
    client/Client.cpp
)

set(
  server_src
    server/main.cpp
    server/Connection.cpp
    server/GameGenerator.cpp
    server/Server.cpp
)

add_executable(sc2tm_srv ${common_src} ${server_src})
add_executable(sc2tm_clt ${common_src} ${client_src})

set(
  common_libs
    stdc++fs
)

target_link_libraries(sc2tm_srv ${Boost_LIBRARIES} ${common_libs})
target_link_libraries(sc2tm_clt ${Boost_LIBRARIES} ${common_libs} pthread)
