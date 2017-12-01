CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)
SET(proj_name "opcua-adapter")
PROJECT(${proj_name} C)
include(GNUInstallDirs)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fvisibility=hidden -fPIC -Wall -Werror -std=gnu99")

SET(INC_DIR
	include
)

SET(INTERNAL_INC_DIR
	src/session
	#src/queue
	src/node
	src/command
)

SET(EXTLIB_INC_DIR
	extlib
)

INCLUDE_DIRECTORIES(
	${INC_DIR}
	${INTERNAL_INC_DIR}
	${EXTLIB_INC_DIR}
)

SET(SRCS
  extlib/open62541.c
  # api
  src/api/opcua_manager.c

  #session
  src/session/edge_opcua_server.c
  src/session/edge_opcua_client.c

  #queue
  #src/queue/queue.c
  #src/queue/message_dispatcher.c

  #node
  src/node/edge_node.c

  #command
  src/command/read.c
  src/command/write.c
)

add_library(${proj_name} SHARED ${SRCS} ${INC_DIR} ${INTERNAL_INC_DIR} ${EXTLIB_INC_DIR})


set_target_properties(${proj_name} PROPERTIES
	PUBLIC_HEADER include/opcua_manager.h)
configure_file(${proj_name}.pc.in ${proj_name}.pc @ONLY)
#target_include_directories(${proj_name} PRIVATE .)

target_link_libraries(${proj_name} -lrt)

install(TARGETS ${proj_name} 
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_BINARY_DIR}/${proj_name}.pc
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)