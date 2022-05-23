#ifndef SERVER_EVENT_H_
#define SERVER_EVENT_H_

#include "server/defs.h"
#include "server/connection.h"

// implentation specifc
typedef void (*__Open)(Conn* conn);
typedef void (*__Close)(Conn* conn);
typedef void (*__Message)(Conn* conn, const unsigned char* message, uint64_t size, int type);

typedef struct EventManager {
    __Open open;
    __Close close;
    __Message message;
} EventManager;

#endif