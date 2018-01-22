

#ifndef EDGE_LOGGER_H_
#define EDGE_LOGGER_H_

#include "open62541.h"
#include "opcua_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG 0
#if DEBUG
#define EDGE_LOG(tag, param) printf("[%s] %s\n", tag, param)
//#define EDGE_LOG_V(tag, param, ...) printf("[%s] %s\n", tag, param)
#define EDGE_LOG_V(tag, param, ...) fprintf(stdout, param, __VA_ARGS__)
#else
#define EDGE_LOG(tag, param)
#define EDGE_LOG_V(tag, param, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* EDGE_LOGGER_H_ */
