#ifndef SERVER_WEBSOCKET_H_
#define SERVER_WEBSOCKET_H_

#include "server/frame.h"
#include "server/socket.h"

int handshake_key(char *wsKey, unsigned char **dest);
int handshake(char *hsrequest, char **hsresponse);

int upgrade(Frame* frame);

#endif