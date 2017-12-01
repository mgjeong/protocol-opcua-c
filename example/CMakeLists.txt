CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)
SET(proj_name "samples")
PROJECT(${proj_name} C)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fvisibility=hidden -fPIC -Wall -Werror -std=gnu99")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/../extlib
  ${CMAKE_SOURCE_DIR}/../include
  ${CMAKE_SOURCE_DIR}/../src/session
  #${CMAKE_SOURCE_DIR}/../src/queue
  ${CMAKE_SOURCE_DIR}/../src/node
  ${CMAKE_SOURCE_DIR}/../src/command
)

add_executable(server ${CMAKE_SOURCE_DIR}/server.c)
add_executable(client ${CMAKE_SOURCE_DIR}/client.c)

target_link_libraries(server -lpthread -lrt ${CMAKE_SOURCE_DIR}/libopcua-adapter.so)
target_link_libraries(client -lpthread -lrt ${CMAKE_SOURCE_DIR}/libopcua-adapter.so)
