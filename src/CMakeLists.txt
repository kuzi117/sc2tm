set(
  common_src
    common/buffer_operations.cpp
)

set(
  client_src
    client/main.cpp
)

set(
  server_src
    server/main.cpp
    server/Server.cpp
)

add_executable(sc2tm_srv ${common_src} ${server_src})
add_executable(sc2tm_clt ${common_src} ${client_src})

target_link_libraries(sc2tm_srv ${Boost_LIBRARIES})
target_link_libraries(sc2tm_clt ${Boost_LIBRARIES} pthread)
