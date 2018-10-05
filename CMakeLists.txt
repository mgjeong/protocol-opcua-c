CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)
SET(proj_name "opcua-adapter")
PROJECT(${proj_name} C CXX)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

SET(BUILD_DIR "build/")
SET(INC_PATH "include")
SET(SRC_PATH "src")
SET(EXTPATH "extlibs")

SET(OPEN62541_VERSION "_0.2")

ADD_DEFINITIONS("-DWINDOWS")
ADD_DEFINITIONS("-DHAVE_STRUCT_TIMESPEC")
ADD_DEFINITIONS("-DPTW32_BUILD_INLINED")
ADD_DEFINITIONS("-DPTW32_STATIC_LIB")
ADD_DEFINITIONS("-DENABLE_SUB_QUEUE")

ADD_DEFINITIONS("/W3 /wd4710 /wd4711 /wd4668 /wd4996 /wd4018 /wd4005 /wd4047 /wd4024 /wd4013 /wd4244 /nologo")

if(NOT "${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
	SET(PTHREAD_LIBRARY "${CMAKE_SOURCE_DIR}/${EXTPATH}/pthread-win32/lib/x86/pthreadVC2.lib")
else()
	SET(PTHREAD_LIBRARY "${CMAKE_SOURCE_DIR}/${EXTPATH}/pthread-win32/lib/x64/pthreadVC2.lib")
endif()

INCLUDE_DIRECTORIES (
	${INC_PATH}
	${EXTPATH}/open62541/open62541${OPEN62541_VERSION}
	${CMAKE_SOURCE_DIR}/${EXTPATH}/pthread-win32/include
	${SRC_PATH}/command
	${SRC_PATH}/command/browse
	${SRC_PATH}/node
	${SRC_PATH}/queue
	${SRC_PATH}/session
	${SRC_PATH}/session/discovery
	${SRC_PATH}/utils
)

SET (SRCS
	${EXTPATH}/open62541/open62541${OPEN62541_VERSION}/open62541.c
	
	${SRC_PATH}/api/opcua_manager.c
	${SRC_PATH}/command/browse/browse.c
	${SRC_PATH}/command/browse/browse_common.c
	${SRC_PATH}/command/browse/browse_view.c
	${SRC_PATH}/command/read.c
	${SRC_PATH}/command/write.c
	${SRC_PATH}/command/method.c
	${SRC_PATH}/command/subscription.c
	${SRC_PATH}/command/cmd_util.c
	${SRC_PATH}/node/edge_node.c
	${SRC_PATH}/queue/caqueueingthread.c
	${SRC_PATH}/queue/cathreadpool_pthreads.c
	${SRC_PATH}/queue/octhread.c
	${SRC_PATH}/queue/uarraylist.c
	${SRC_PATH}/queue/uqueue.c
	${SRC_PATH}/queue/message_dispatcher.c
	${SRC_PATH}/session/edge_opcua_client.c
	${SRC_PATH}/session/edge_opcua_server.c
	${SRC_PATH}/session/discovery/edge_discovery_common.c
	${SRC_PATH}/session/discovery/edge_find_servers.c
	${SRC_PATH}/session/discovery/edge_get_endpoints.c
	${SRC_PATH}/utils/edge_malloc.c
	${SRC_PATH}/utils/edge_utils.c
	${SRC_PATH}/utils/edge_random.c
	${SRC_PATH}/utils/edge_map.c
	${SRC_PATH}/utils/edge_list.c
	${SRC_PATH}/utils/edge_open62541.c
)

ADD_LIBRARY(${proj_name} STATIC ${SRCS})

target_link_libraries(${proj_name} ${PTHREAD_LIBRARY} wsock32 ws2_32)

ADD_SUBDIRECTORY(example)

ADD_CUSTOM_COMMAND(
    TARGET ${proj_name}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/build/Release/opcua-adapter.lib
    ${CMAKE_SOURCE_DIR}/out

    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/${EXTPATH}/pthread-win32/dll/x86/pthreadVC2.dll
    ${CMAKE_SOURCE_DIR}/example/out
)
