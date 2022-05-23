/*
* Frank Bender: This file needs to be split up
*/
#ifndef SERVER_SOCKET_H_
#define SERVER_SOCKET_H_

#include "server/defs.h"
#include "server/response.h"
#include "server/connection.h"
#include "server/event.h"
#include "server/web_socket.h"
#include "util/scan.h"

void socket_close(int fd);
void socket_create(int port, int reuse);
ssize_t broadcast(Conn* conn, const void *buf, size_t len, int flags);
const char* get_content_type(const char* path);
int link(Frame* frame);
int handshake_key(char* key, unsigned char** dest);
void handshake_response(Conn* conn, const char* path, const char* subkey);
char* socket_key(const char* subkey);
void serve_resource(Conn* conn, const char* path);
void server_get_request(int *conn, char *req_buf, int port);

#endif