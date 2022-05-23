#ifndef SERVER_RESPONSE_H_
#define SERVER_RESPONSE_H_

#include "server/defs.h"

// intuitive
void send_400(SOCKET socket);
void send_404(SOCKET socket);

#endif