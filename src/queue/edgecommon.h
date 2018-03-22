#ifndef CA_EDGE_COMMON_H_
#define CA_EDGE_COMMON_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    typedef enum
    {
        CA_STATUS_OK = 0,
        CA_STATUS_INVALID_PARAM,
        CA_MEMORY_ALLOC_FAILED,
        CA_STATUS_FAILED,

    } CAResult_t;

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CA_EDGE_COMMON_H_ */
