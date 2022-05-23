#ifndef SERVER_FRAME_H_
#define SERVER_FRAME_H_

#include "server/defs.h"
#include "server/connection.h"
#include "server/socket.h"
#include "server/ping_pong.h"

int ws_sendframe(Conn* conn, const char *message, uint64_t size, int type);
int ws_sendframe_txt(Conn *conn, const char *message);
int ws_sendframe_bin(Conn *conn, const char *message, uint64_t size);
static inline int is_control_frame(int frame);
static inline int next_byte(Frame *frame);
int skip_frame(Frame* frame, uint64_t frame_size);
int read_frame(Frame* frame, int opcode, unsigned char** buf, 
  uint64_t* frame_length, uint64_t* frame_size, uint64_t* message_idx, uint8_t* masks, int is_fin);
int next_frame(Frame* frame);


#endif