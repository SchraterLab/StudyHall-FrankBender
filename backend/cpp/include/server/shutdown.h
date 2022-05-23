#ifndef SERVER_SHUTDOWN_H_
#define SERVER_SHUTDOWN_H_

#include "server/connection.h"
#include "server/frame.h"

static Any close_timeout(Any p);
int start_close_timeout(Conn* conn);
int do_close(Frame* frame, int close_code);

#endif