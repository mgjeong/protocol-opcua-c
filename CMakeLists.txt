CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)
SET(proj_name "opcua-adapter")
PROJECT(${proj_name} C CXX)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fvisibility=hidden -fPIC -Wall -Werror -std=gnu99")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpie ${EXTRA_CFLAGS} -Wall -g -fPIC -std=c++0x -fvisibility=hidden")

SET(INC_DIR
	include
)

SET(INTERNAL_INC_DIR
	src/session
	src/queue
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
	src/queue/queue.c
	src/queue/message_dispatcher.c
)

set_source_files_properties(${CMAKE_SOURCE_DIR}/extlib/open62541.c PROPERTIES HEADER_FILES_ONLY TRUE)
set_source_files_properties(${CMAKE_SOURCE_DIR}/extlib/open62541.c PROPERTIES LANGUAGE C)

link_directories(${CMAKE_SOURCE_DIR}/src/extlib/open62541)

#add_library(open62541 STATIC ${CMAKE_SOURCE_DIR}/src/extlib/open62541/open62541.c)

SET(CMAKE_C_STANDARD 99)

ADD_LIBRARY(${proj_name} SHARED ${SRCS})

#TARGET_LINK_LIBRARIES(${proj_name} ${proj_name}_LDFLAGS)

message(${PROJECT_BINARY_DIR})
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

#INSTALL(TARGETS ${proj_name} DESTINATION LIBRARY_OUTPUT_DIRECTORY)
