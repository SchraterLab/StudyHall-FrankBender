#include "server/ping_pong.h"

int do_pong(Frame* frame, uint64_t frame_size) {
	if (ws_sendframe(frame->conn, (const char*)frame->payload, frame_size, FRAME_PONG) < 0) {
		frame->error = 1;
		PERR(ESERVER, "An error has occurred while ponging!\n");
		return (-1);
	}
	return (0);
}

int32_t pong_message_to_int32(uint8_t *message) {
	int32_t pong_id;
	// Decodes as big-endian.
	pong_id = (message[3] << 0) | (message[2] << 8) | (message[1] << 16) | (message[0] << 24);
	return (pong_id);
}

static inline void int32_to_ping_message(int32_t ping_id, uint8_t *message) {
	// Encodes as big-endian.
	message[0] = (ping_id >> 24);
	message[1] = (ping_id >> 16);
	message[2] = (ping_id >>  8);
	message[3] = (ping_id >>  0);
}

static void send_ping_close(Conn* conn, int threshold, int lock) {
	uint8_t ping_message[4];

	if (!ISVALIDSOCKET(conn) || conn_get_state(conn) != SOCKST_OPEN_WS)
		return;

	pthread_mutex_lock(&conn->ping_mutex);

		conn->current_ping_id++;
		int32_to_ping_message(conn->current_ping_id, ping_message);

		// Send ping
		ws_sendframe(conn, (const char*)ping_message, sizeof(ping_message), FRAME_PING);

		// Check previous PONG: if greater than threshold, abort.
		if ((conn->current_ping_id - conn->last_pong_id) > threshold)
			close_conn(conn, lock);

	pthread_mutex_unlock(&conn->ping_mutex);
}

void ws_ping(Conn* conn, int threshold) {
	int i;

	if (threshold <= 0) return;

	// ping a single conn. */
	if (conn) send_ping_close(conn, threshold, 1);

	// ping broadcast
	// else {
	// 	pthread_mutex_lock(&global_mutex);
	// 		for (i = 0; i < THREAD_POOL; i++)
	// 			send_ping_close(&connections[i], threshold, 0);
	// 	pthread_mutex_unlock(&global_mutex);
	// }
}