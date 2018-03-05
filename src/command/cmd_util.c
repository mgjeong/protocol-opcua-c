#include <cmd_util.h>

int get_response_type(const UA_DataType *datatype)
{
    if (datatype == &UA_TYPES[UA_TYPES_BOOLEAN])
    {
        return UA_NS0ID_BOOLEAN;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_INT16])
    {
        return UA_NS0ID_INT16;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_UINT16])
    {
        return UA_NS0ID_UINT16;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_INT32])
    {
        return UA_NS0ID_INT32;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_UINT32])
    {
        return UA_NS0ID_UINT32;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_INT64])
    {
        return UA_NS0ID_INT64;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_UINT64])
    {
        return UA_NS0ID_UINT64;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_FLOAT])
    {
        return UA_NS0ID_FLOAT;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_DOUBLE])
    {
        return UA_NS0ID_DOUBLE;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_STRING])
    {
        return UA_NS0ID_STRING;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_BYTESTRING])
    {
        return UA_NS0ID_BYTESTRING;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_GUID])
    {
        return UA_NS0ID_GUID;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_SBYTE])
    {
        return UA_NS0ID_SBYTE;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_BYTE])
    {
        return UA_NS0ID_BYTE;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_DATETIME])
    {
        return UA_NS0ID_DATETIME;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_XMLELEMENT])
    {
        return UA_NS0ID_XMLELEMENT;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_QUALIFIEDNAME])
    {
        return UA_NS0ID_QUALIFIEDNAME;
    }
    else if (datatype == &UA_TYPES[UA_TYPES_LOCALIZEDTEXT])
    {
        return UA_NS0ID_LOCALIZEDTEXT;
    }
    return -1;
}

void* parse_response_string()
{

}
