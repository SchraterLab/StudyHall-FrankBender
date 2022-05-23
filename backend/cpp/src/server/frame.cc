#include "server/frame.h"

int ws_sendframe(Conn* conn, const char *message, uint64_t size, int type) {
	unsigned char *response; /* Response data.     */
	unsigned char frame[10]; /* Frame.             */
	uint8_t idx_first_rData; /* Index data.        */
	int idx_response;        /* Index response.    */
	ssize_t output;          /* Bytes sent.        */
	ssize_t send_ret;        /* Ret send function  */
	uint64_t i;              /* Loop index.        */
	Conn* lconn;

	frame[0] = (FIN | type);
	uint64_t length = (uint64_t)size; // message length

	/* Split the size between octets. */
	if (length <= 125) {
		frame[1] = length & 0x7F;
		idx_first_rData = 2;
	}

	/* Size between 126 and 65535 bytes. */
	else if (length >= 126 && length <= 65535) {
		frame[1] = 126;
		frame[2] = (length >> 8) & 255;
		frame[3] = length & 255;
		idx_first_rData = 4;
	}

	/* More than 65535 bytes. */
	else {
		frame[1] = 127;
		frame[2] = (unsigned char)((length >> 56) & 255);
		frame[3] = (unsigned char)((length >> 48) & 255);
		frame[4] = (unsigned char)((length >> 40) & 255);
		frame[5] = (unsigned char)((length >> 32) & 255);
		frame[6] = (unsigned char)((length >> 24) & 255);
		frame[7] = (unsigned char)((length >> 16) & 255);
		frame[8] = (unsigned char)((length >> 8) & 255);
		frame[9] = (unsigned char)(length & 255);
		idx_first_rData = 10;
	}

	/* Add frame bytes. */
	idx_response = 0;
	response = (unsigned char*)malloc(sizeof(unsigned char) * (idx_first_rData + length + 1));
	if (!response)
		return (-1);

	for (i = 0; i < idx_first_rData; i++) {
		response[i] = frame[i];
		idx_response++;
	}

	/* Add data bytes. */
	for (i = 0; i < length; i++) {
		response[idx_response] = message[i];
		idx_response++;
	}

	response[idx_response] = '\0';

	// printf("FRAME RESPONSE: %s\n", (char*)response);

	/* Send to the conn if there is one. */
	output = 0;
	if (conn) output = broadcast(conn, response, idx_response, 0);

	/* If no conn specified, broadcast to everyone. */
	if (!conn) {

		pthread_mutex_lock(&global_mutex);
		for (i = 0; i < THREAD_POOL; i++) {
			lconn = &connections[i];
			if ((lconn->socket > -1) && conn_get_state(lconn) == SOCKST_OPEN_WS) {
				if ((send_ret = send(lconn->socket, response, idx_response, 0)) != -1)
					output += send_ret;
				else {
					output = -1;
					break;
				}
			}
		}
		pthread_mutex_unlock(&global_mutex);
	}
	
	free(response);
	return ((int)output);
}

int ws_sendframe_txt(Conn *conn, const char *message) {
	return ws_sendframe(conn, message, (uint64_t)strlen(message), FRAME_TXT);
}

int ws_sendframe_bin(Conn *conn, const char *message, uint64_t size) {
	return ws_sendframe(conn, message, size, FRAME_BIN);
}

static inline int is_control_frame(int frame) {
	return (frame == FRAME_CLOSE || frame == FRAME_PING || frame == FRAME_PONG);
}

static inline int next_byte(Frame *frame) {
	ssize_t n;

	// If empty or full
	if (frame->cur_pos == 0 || frame->cur_pos == frame->received) {
		if ((n = recv(frame->conn->socket, frame->request, sizeof(frame->request), 0)) <= 0) {
			frame->error = 1;
			PERR(EWS, "An error has occurred while trying to read next byte\n");
			return (-1);
		}
		frame->received = (size_t)n;
		printf("Next byte: %s\n", (char*)frame->request);
		printf("Frame received: %i\n", (int)frame->received);
		frame->cur_pos = 0;
	}

	// BCYA("Frame next!\n");
	return (frame->request[frame->cur_pos++]);
}

int skip_frame(Frame* frame, uint64_t frame_size) {
	uint64_t i;
	for (i = 0; i < frame_size; i++) {
		if (next_byte(frame) == -1) {
			frame->error = 1;
			return (-1);
		}
	}
	return (0);
}

int read_frame(Frame* frame, int opcode, unsigned char** buf, 
  uint64_t* frame_length, uint64_t* frame_size, uint64_t* message_idx, uint8_t* masks, int is_fin) {

	unsigned char* tmp; /* Tmp message.     */
	int cur_byte;       /* Curr byte read.  */
	uint64_t i;         /* Loop index.      */

	unsigned char* message = *buf; // current message

	/* Decode masks and length for 16-bit messages. */
	if (*frame_length == 126) *frame_length = (((uint64_t)next_byte(frame)) << 8) | next_byte(frame);

	/* 64-bit messages. */
	else if (*frame_length == 127) {
		*frame_length =
			(((uint64_t)next_byte(frame)) << 56) | /* frame[2]. */
			(((uint64_t)next_byte(frame)) << 48) | /* frame[3]. */
			(((uint64_t)next_byte(frame)) << 40) | (((uint64_t)next_byte(frame)) << 32) |
			(((uint64_t)next_byte(frame)) << 24) | (((uint64_t)next_byte(frame)) << 16) |
			(((uint64_t)next_byte(frame)) << 8) |
			(((uint64_t)next_byte(frame))); /* frame[9]. */
	}

	*frame_size += *frame_length;

	if (*frame_size > MAX_FRAME_LENGTH) {
		PERR(EWS, "Current frame from conn %d, exceeds the maximum\n"
			  "amount of bytes allowed (%lu/%d)!",
			frame->conn->socket, *frame_size + *frame_length, MAX_FRAME_LENGTH)
		frame->error = 1;
		return (-1);
	}

	// Read masks
	masks[0] = next_byte(frame);
	masks[1] = next_byte(frame);
	masks[2] = next_byte(frame);
	masks[3] = next_byte(frame);

	// Abort if error.
	if (frame->error) return (-1);

	// Allocate memory
	if (*frame_length > 0) {
		if (!is_control_frame(opcode)) {
			tmp = (unsigned char*)realloc(
				message, sizeof(unsigned char) * (*message_idx + *frame_length + is_fin));
			if (!tmp) {
				// CHECK FOR PRI64
				PERR(EWS, "Cannot allocate memory, requested: %lu\n",
					(*message_idx + *frame_length + is_fin));

				frame->error = 1;
				return (-1);
			}
			message = tmp;
			*buf = message;
		}

		/* Copy to the proper location. */
		for (i = 0; i < *frame_length; i++, (*message_idx)++) {
			/* We were able to read? .*/
			cur_byte = next_byte(frame);
			if (cur_byte == -1)
				return (-1);

			message[*message_idx] = cur_byte ^ masks[i % 4];
		}
	}

	/* If we're inside a FIN frame, lets... */
	if (is_fin && *frame_size > 0) {
		/* Increase memory if our FIN frame is of length 0. */
		if (!*frame_length && !is_control_frame(opcode)) {
			tmp = (unsigned char*)realloc(message, sizeof(unsigned char) * (*message_idx + 1));
			if (!tmp) {
				// DEBUG("Cannot allocate memory, requested: %" PRId64 "\n",
				// 	(*message_idx + 1));

				frame->error = 1;
				return (-1);
			}
			message = tmp;
			*buf = message;
		}
		message[*message_idx] = '\0';
	}

	return (0);
}

// static int next_frame(struct ws_frame_data *wfd)
// {
// 	unsigned char *msg_data; /* Data frame.                */
// 	unsigned char *msg_ctrl; /* Control frame.             */
// 	uint8_t masks_data[4];   /* Masks data frame array.    */
// 	uint8_t masks_ctrl[4];   /* Masks control frame array. */
// 	uint64_t msg_idx_data;   /* Current msg index.         */
// 	uint64_t msg_idx_ctrl;   /* Current msg index.         */
// 	uint64_t frame_length;   /* Frame length.              */
// 	uint64_t frame_size;     /* Current frame size.        */
// 	uint32_t utf8_state;     /* Current UTF-8 state.       */
// 	int32_t pong_id;         /* Current PONG id.           */
// 	uint8_t opcode;          /* Frame opcode.              */
// 	uint8_t is_fin;          /* Is FIN frame flag.         */
// 	uint8_t mask;            /* Mask.                      */
// 	int cur_byte;            /* Current frame byte.        */

// 	msg_data = NULL;
// 	msg_ctrl = wfd->msg_ctrl;
// 	is_fin = 0;
// 	frame_length = 0;
// 	frame_size = 0;
// 	msg_idx_data = 0;
// 	msg_idx_ctrl = 0;
// 	wfd->frame_size = 0;
// 	wfd->frame_type = -1;
// 	wfd->msg = NULL;
// 	utf8_state = UTF8_ACCEPT;

// 	/* Read until find a FIN or a unsupported frame. */
// 	do
// 	{
// 		/*
// 		 * Obs: next_byte() can return error if not possible to read the
// 		 * next frame byte, in this case, we return an error.
// 		 *
// 		 * However, please note that this check is only made here and in
// 		 * the subsequent next_bytes() calls this also may occur too.
// 		 * wsServer is assuming that the client only create right
// 		 * frames and we will do not have disconnections while reading
// 		 * the frame but just when waiting for a frame.
// 		 */
// 		cur_byte = next_byte(wfd);
// 		if (cur_byte == -1)
// 			return (-1);

// 		is_fin = (cur_byte & 0xFF) >> WS_FIN_SHIFT;
// 		opcode = (cur_byte & 0xF);

// 		/*
// 		 * Check for RSV field.
// 		 *
// 		 * Since wsServer do not negotiate extensions if we receive
// 		 * a RSV field, we must drop the connection.
// 		 */
// 		if (cur_byte & 0x70)
// 		{
// 			DEBUG("RSV is set while wsServer do not negotiate extensions!\n");
// 			wfd->error = 1;
// 			break;
// 		}

// 		/*
// 		 * Check if the current opcode makes sense:
// 		 * a) If we're inside a cont frame but no previous data frame
// 		 *
// 		 * b) If we're handling a data-frame and receive another data
// 		 *    frame. (it's expected to receive only CONT or control
// 		 *    frames).
// 		 *
// 		 * It is worth to note that in a), we do not need to check
// 		 * if the previous frame was FIN or not: if was FIN, an
// 		 * on_message event was triggered and this function returned;
// 		 * so the only possibility here is a previous non-FIN data
// 		 * frame, ;-).
// 		 */
// 		if ((wfd->frame_type == -1 && opcode == WS_FR_OP_CONT) ||
// 			(wfd->frame_type != -1 && !is_control_frame(opcode) &&
// 				opcode != WS_FR_OP_CONT))
// 		{
// 			DEBUG("Unexpected frame was received!, opcode: %d, previous: %d\n",
// 				opcode, wfd->frame_type);
// 			wfd->error = 1;
// 			break;
// 		}

// 		/* Check if one of the valid opcodes. */
// 		if (opcode == WS_FR_OP_TXT || opcode == WS_FR_OP_BIN ||
// 			opcode == WS_FR_OP_CONT || opcode == WS_FR_OP_PING ||
// 			opcode == WS_FR_OP_PONG || opcode == WS_FR_OP_CLSE)
// 		{
// 			/*
// 			 * Check our current state: if CLOSING, we only accept close
// 			 * frames.
// 			 *
// 			 * Since the server may, at any time, asynchronously, asks
// 			 * to close the client connection, we should terminate
// 			 * immediately.
// 			 */
// 			if (get_client_state(wfd->client) == WS_STATE_CLOSING &&
// 				opcode != WS_FR_OP_CLSE)
// 			{
// 				DEBUG("Unexpected frame received, expected CLOSE (%d), "
// 					  "received: (%d)",
// 					WS_FR_OP_CLSE, opcode);
// 				wfd->error = 1;
// 				break;
// 			}

// 			/* Only change frame type if not a CONT frame. */
// 			if (opcode != WS_FR_OP_CONT && !is_control_frame(opcode))
// 				wfd->frame_type = opcode;

// 			mask = next_byte(wfd);
// 			frame_length = mask & 0x7F;
// 			frame_size = 0;
// 			msg_idx_ctrl = 0;

// 			/*
// 			 * We should deny non-FIN control frames or that have
// 			 * more than 125 octets.
// 			 */
// 			if (is_control_frame(opcode) && (!is_fin || frame_length > 125))
// 			{
// 				DEBUG("Control frame bigger than 125 octets or not a FIN "
// 					  "frame!\n");
// 				wfd->error = 1;
// 				break;
// 			}

// 			/* Normal data frames. */
// 			if (opcode == WS_FR_OP_TXT || opcode == WS_FR_OP_BIN ||
// 				opcode == WS_FR_OP_CONT)
// 			{
// 				if (read_frame(wfd, opcode, &msg_data, &frame_length,
// 						&wfd->frame_size, &msg_idx_data, masks_data, is_fin) < 0)
// 					break;

// #ifdef VALIDATE_UTF8
// 				/* UTF-8 Validate partial (or not) frame. */
// 				if (wfd->frame_type == WS_FR_OP_TXT)
// 				{
// 					if (is_fin)
// 					{
// 						if (is_utf8_len_state(
// 								msg_data + (msg_idx_data - frame_length),
// 								frame_length, utf8_state) != UTF8_ACCEPT)
// 						{
// 							DEBUG("Dropping invalid complete message!\n");
// 							wfd->error = 1;
// 							do_close(wfd, WS_CLSE_INVUTF8);
// 						}
// 					}

// 					/* Check current state for a CONT or initial TXT frame. */
// 					else
// 					{
// 						utf8_state = is_utf8_len_state(
// 							msg_data + (msg_idx_data - frame_length), frame_length,
// 							utf8_state);

// 						/* We can be in any state, except reject. */
// 						if (utf8_state == UTF8_REJECT)
// 						{
// 							DEBUG("Dropping invalid cont/initial frame!\n");
// 							wfd->error = 1;
// 							do_close(wfd, WS_CLSE_INVUTF8);
// 						}
// 					}
// 				}
// #endif
// 			}

// 			/*
// 			 * We _may_ send a PING frame if the ws_ping() routine was invoked.
// 			 *
// 			 * If the content is invalid and/or differs the size, ignore it.
// 			 * (maybe unsolicited PONG).
// 			 */
// 			else if (opcode == WS_FR_OP_PONG)
// 			{
// 				if (read_frame(wfd, opcode, &msg_ctrl, &frame_length, &frame_size,
// 						&msg_idx_ctrl, masks_ctrl, is_fin) < 0)
// 					break;

// 				is_fin = 0;

// 				/* If there is no content and/or differs the size, ignore it. */
// 				if (frame_size != sizeof(wfd->client->last_pong_id))
// 					continue;

// 				/*
// 				 * Our PONG id should be positive and smaller than our
// 				 * current PING id. If not, ignore.
// 				 */
// 				/* clang-format off */
// 				pthread_mutex_lock(&wfd->client->mtx_ping);

// 					pong_id = pong_msg_to_int32(msg_ctrl);
// 					if (pong_id < 0 || pong_id > wfd->client->current_ping_id)
// 					{
// 						pthread_mutex_unlock(&wfd->client->mtx_ping);
// 						continue;
// 					}
// 					wfd->client->last_pong_id = pong_id;

// 				pthread_mutex_unlock(&wfd->client->mtx_ping);
// 				/* clang-format on */
// 				continue;
// 			}

// 			/* We should answer to a PING frame as soon as possible. */
// 			else if (opcode == WS_FR_OP_PING)
// 			{
// 				if (read_frame(wfd, opcode, &msg_ctrl, &frame_length, &frame_size,
// 						&msg_idx_ctrl, masks_ctrl, is_fin) < 0)
// 					break;

// 				if (do_pong(wfd, frame_size) < 0)
// 					break;

// 				/* Quick hack to keep our loop. */
// 				is_fin = 0;
// 			}

// 			/* We interrupt the loop as soon as we find a CLOSE frame. */
// 			else
// 			{
// 				if (read_frame(wfd, opcode, &msg_ctrl, &frame_length, &frame_size,
// 						&msg_idx_ctrl, masks_ctrl, is_fin) < 0)
// 					break;

// #ifdef VALIDATE_UTF8
// 				/* If there is a close reason, check if it is UTF-8 valid. */
// 				if (frame_size > 2 && !is_utf8_len(msg_ctrl + 2, frame_size - 2))
// 				{
// 					DEBUG("Invalid close frame payload reason! (not UTF-8)\n");
// 					wfd->error = 1;
// 					break;
// 				}
// #endif

// 				/* Since we're aborting, we can scratch the 'data'-related
// 				 * vars here. */
// 				wfd->frame_size = frame_size;
// 				wfd->frame_type = WS_FR_OP_CLSE;
// 				free(msg_data);
// 				return (0);
// 			}
// 		}

// 		/* Anything else (unsupported frames). */
// 		else
// 		{
// 			DEBUG("Unsupported frame opcode: %d\n", opcode);
// 			/* We should consider as error receive an unknown frame. */
// 			wfd->frame_type = opcode;
// 			wfd->error = 1;
// 		}

// 	} while (!is_fin && !wfd->error);

// 	/* Check for error. */
// 	if (wfd->error)
// 	{
// 		free(msg_data);
// 		wfd->msg = NULL;
// 		return (-1);
// 	}

// 	wfd->msg = msg_data;
// 	return (0);
// }

int next_frame(Frame* frame) {
	uint8_t masks_data[4];   /* Masks data frame array.    */
	uint8_t masks_ctrl[4];   /* Masks control frame array. */
	uint32_t utf8_state;     /* Current UTF-8 state.       */
	int32_t pong_id;         /* Current PONG id.           */
	uint8_t opcode;          /* Frame opcode.              */
	uint8_t mask;            /* Mask.                      */
	int cur_byte;            /* Current frame byte.        */

	unsigned char* message_data = NULL;         // data frame
	unsigned char* payload = frame->payload;    // control frame
	uint8_t is_fin = 0;                         // FIN frame flag
	uint64_t frame_length = 0;                   // Frame length
	uint64_t frame_size = 0;                    // Current frame size
	uint64_t message_idx_data = 0;              // Current message index
	uint64_t message_idx_ctrl = 0;              // Current message index
	frame->size = 0; 
	frame->type = -1;
	frame->message = NULL;
	utf8_state = UTF8_ACCEPT;

// 	unsigned char *msg_data; /* Data frame.                */
// 	unsigned char *msg_ctrl; /* Control frame.             */
// 	uint8_t masks_data[4];   /* Masks data frame array.    */
// 	uint8_t masks_ctrl[4];   /* Masks control frame array. */
// 	uint64_t msg_idx_data;   /* Current msg index.         */
// 	uint64_t msg_idx_ctrl;   /* Current msg index.         */
// 	uint64_t frame_length;   /* Frame length.              */
// 	uint64_t frame_size;     /* Current frame size.        */
// 	uint32_t utf8_state;     /* Current UTF-8 state.       */
// 	int32_t pong_id;         /* Current PONG id.           */
// 	uint8_t opcode;          /* Frame opcode.              */
// 	uint8_t is_fin;          /* Is FIN frame flag.         */
// 	uint8_t mask;            /* Mask.                      */
// 	int cur_byte;            /* Current frame byte.        */

// 	msg_data = NULL;
// 	msg_ctrl = wfd->msg_ctrl;
// 	is_fin = 0;
// 	frame_length = 0;
// 	frame_size = 0;
// 	msg_idx_data = 0;
// 	msg_idx_ctrl = 0;
// 	wfd->frame_size = 0;
// 	wfd->frame_type = -1;
// 	wfd->msg = NULL;
// 	utf8_state = UTF8_ACCEPT;


	/* Read until find a FIN or a unsupported frame. */
	do {
		cur_byte = next_byte(frame);
		if (cur_byte == -1) { PERR(ESERVER, "Can't read next byte"); return -1; }

		is_fin = (cur_byte & 0xFF) >> FRAME_FIN;
		opcode = (cur_byte & 0xF);

		// Check for RSV field.
		if (cur_byte & 0x70) {
			PERR(ESERVER, "RSV is set while wsServer do not negotiate extensions!\n");
			frame->error = 1;
			break;
		}

		if ((frame->type == -1 && opcode == FRAME_CONT) ||
			(frame->type != -1 && !is_control_frame(opcode) && opcode != FRAME_CONT)) {
			PERR(ESERVER, "Unexpected frame was received!, opcode: %d, previous: %d\n", opcode, frame->type);
			frame->error = 1;
			break;
		}

		/* Check if one of the valid opcodes. */
		if (opcode == FRAME_TXT || opcode == FRAME_BIN ||
			opcode == FRAME_CONT || opcode == FRAME_PING ||
			opcode == FRAME_PONG || opcode == FRAME_CLOSE) {

			if (conn_get_state(frame->conn) == SOCKST_CLOSING &&
				opcode != FRAME_CLOSE) {
				// DEBUG("Unexpected frame received, expected CLOSE (%d), "
				// 	  "received: (%d)",
				// 	FRAME_CLOSE, opcode);
				frame->error = 1;
				break;
			}

			// Only change frame type if not a CONT frame.
			if (opcode != FRAME_CONT && !is_control_frame(opcode)) frame->type = opcode;

			mask = next_byte(frame);
			frame_length = mask & 0x7F;
			frame_size = 0;
			message_idx_ctrl = 0;

			
			// We should deny non-FIN control frames or that have more than 125 octets.
			if (is_control_frame(opcode) && (!is_fin || frame_length > 125)) {
				PERR(ESERVER, "Control frame bigger than 125 octets or not a FIN frame!\n");
				frame->error = 1;
				break;
			}

			// Normal data frames.
			if (opcode == FRAME_TXT || opcode == FRAME_BIN ||
				opcode == FRAME_CONT) {
				if (read_frame(frame, opcode, &message_data, &frame_length,
						&frame->size, &message_idx_data, masks_data, is_fin) < 0)
					break;

#ifdef VALIDATE_UTF8
				// /* UTF-8 Validate partial (or not) frame. */
				if (frame->type == FRAME_TXT)
				{
					if (is_fin)
					{
						if (is_utf8_len_state(
								message_data + (message_idx_data - frame_length),
								frame_length, utf8_state) != UTF8_ACCEPT)
						{
							DEBUG("Dropping invalid complete message!\n");
							frame->error = 1;
							do_close(frame, WS_CLSE_INVUTF8);
						}
					}

					/* Check current state for a CONT or initial TXT frame. */
					else
					{
						utf8_state = is_utf8_len_state(
							message_data + (message_idx_data - frame_length), frame_length,
							utf8_state);

						/* We can be in any state, except reject. */
						if (utf8_state == UTF8_REJECT)
						{
							DEBUG("Dropping invalid cont/initial frame!\n");
							frame->error = 1;
							do_close(frame, WS_CLSE_INVUTF8);
						}
					}
				}
#endif
			}

			else if (opcode == FRAME_PONG) {
				if (read_frame(frame, opcode, &payload, &frame_length, &frame_size,
						&message_idx_ctrl, masks_ctrl, is_fin) < 0)
					break;

				is_fin = 0;

				// If there is no content and/or differs the size, ignore it.
				if (frame_size != sizeof(frame->conn->last_pong_id)) continue;

				pthread_mutex_lock(&frame->conn->ping_mutex);

					pong_id = pong_message_to_int32(payload);
					if (pong_id < 0 || pong_id > frame->conn->current_ping_id) {
						pthread_mutex_unlock(&frame->conn->ping_mutex);
						continue;
					}
					frame->conn->last_pong_id = pong_id;

				pthread_mutex_unlock(&frame->conn->ping_mutex);
				/* clang-format on */
				continue;
			}

			/* We should answer to a PING frame as soon as possible. */
			else if (opcode == FRAME_PING) {
				if (read_frame(frame, opcode, &payload, &frame_length, &frame_size,
						&message_idx_ctrl, masks_ctrl, is_fin) < 0)
					break;

				if (do_pong(frame, frame_size) < 0)
					break;

				/* Quick hack to keep our loop. */
				is_fin = 0;
			}

			/* We interrupt the loop as soon as we find a CLOSE frame. */
			else
			{
				if (read_frame(frame, opcode, &payload, &frame_length, &frame_size,
						&message_idx_ctrl, masks_ctrl, is_fin) < 0)
					break;

#ifdef VALIDATE_UTF8
				// /* If there is a close reason, check if it is UTF-8 valid. */
				// if (frame_size > 2 && !is_utf8_len(payload + 2, frame_size - 2))
				// {
				// 	PERR(ESERVER, "Invalid close frame payload reason! (not UTF-8)\n");
				// 	frame->error = 1;
				// 	break;
				// }
#endif

				/* Since we're aborting, we can scratch the 'data'-related
				 * vars here. */
				frame->size = frame_size;
				frame->type = FRAME_CLOSE;
				free(message_data);
				return 0;
			}
		}

		/* Anything else (unsupported frames). */
		else {
            char buffer[64];
			PERR(ESERVER, "Unsupported frame opcode: %d\n", opcode);
			// We should consider as error receive an unknown frame.
			frame->type = opcode;
			frame->error = 1;
		}

	} while (!is_fin && !frame->error);

	// Check for error.
	if (frame->error) {
		free(message_data);
		frame->message = NULL;
		return -1;
	}

	frame->message = message_data;
	return 0;
}