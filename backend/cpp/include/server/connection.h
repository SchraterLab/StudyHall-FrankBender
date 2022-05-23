#ifndef SERVER_CLIENT_H_
#define SERVER_CLIENT_H_

#include "server/defs.h"
#include "server/shutdown.h"

SocketState conn_get_state(Conn* conn);
void conn_set_state(Conn* conn, SocketState state);
void close_conn(Conn* conn, int lock);
int ws_close_conn(Conn* conn);
void conn_set_address(Conn* conn);
void conn_get_address(Conn* conn, char* dest);
void conn_create(Conn* conn, SOCKET s);
void conn_create(Conn* conn, SOCKET s, char* request, int received);
// char* get_conn_address(Conn* conn);
Conn* get_conn(SOCKET s);
// will not work with thread pool
void drop_conn(Conn* conn);
int ws_get_state(Conn *conn);

#endif