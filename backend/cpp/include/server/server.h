#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include "server/defs.h"
#include "server/event.h"
#include "server/connection.h"
#include "server/socket.h"
#include "server/frame.h"
#include "server/ping_pong.h"
#include "server/shutdown.h"
#include "util/file_system_c.h"

extern EventManager event_manager;

void connect(void* targ);
static void* service(void* targ);

#endif