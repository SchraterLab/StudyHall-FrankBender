#ifndef SERVER_PING_PONG_H_
#define SERVER_PING_PONG_H_

#include "server/defs.h"
#include "server/connection.h"

int do_pong(Frame* frame, uint64_t frame_size);
int32_t pong_message_to_int32(uint8_t *message);
static inline void int32_to_ping_message(int32_t ping_id, uint8_t *message);
static void send_ping_close(Conn* conn, int threshold, int lock);
void ws_ping(Conn* conn, int threshold);

#endif