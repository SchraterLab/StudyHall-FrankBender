#include "server/server.h"

int link(Frame* frame) {
	// pthread_mutex_lock(&global_mutex);
    BCYA("Linking...\n");
	char* response; /* Handshake response message. */

	// SEGH

    // ssize_t n;
	// if (n = recv(frame->conn->socket, frame->request, sizeof(frame->request) - 1, 0) < 0) {
    //     PERR(ESERVER, "No data recieved in handshake\n");
    //     conn_set_state(frame->conn, SOCKST_NULL); 
	// 	return -1;
    // } 

	// SEGH

    char tbuffer[5000];
    sprintf(tbuffer, "Conn request is: %s\n", frame->request);
    write_thread(frame->conn->socket, tbuffer);

	BMAG("Request: %s\n", (char*)frame->request);

	/* Advance our pointers before the first next_byte(). */
	const char* p = strstr((const char *)frame->request, "\r\n\r\n");
	if (p == NULL) { 
        PERR(ESERVER, "An empty line with \\r\\n was expected!\n"); 
        conn_set_state(frame->conn, SOCKST_NULL); 
    }

	// frame->received = sizeof(frame->r);
	frame->cur_pos = (size_t)((ptrdiff_t)(p - (char *)frame->request)) + 4;

	response = (char*)malloc(sizeof(char) * ACCEPT_LEN);
	if (handshake((char *)frame->request, &response) < 0) {
		DEBUG("Cannot get handshake response, request was: %s\n", frame->request);
		return (-1);
	}

	printf("Handshaked, response: \n"
		"------------------------------------\n"
		"%s"
		"------------------------------------\n",
	response);

    // strcat(response, "sUBSUSusush");

    printf("RESPONSE SIZE IS: %i", (int)strlen(response));

	/* Send handshake. */
	if (broadcast(frame->conn, response, strlen(response), 0) < 0) {
		free(response);
		PERR(ESERVER, "As error has occurred while handshaking!\n");
		return -1;
	}

    // do_pong(frame, strlen(frame->payload), FRAM)

	// Trigger events and clean up buffers
    GRE("YIPPEE KI YAE!\n");
	event_manager.open(frame->conn);
	free(response);

    // if (scan("Connection: keep-alive", (char*)frame->request)) {
	// 	DEBUG("Serving resource, request was: %s\n", frame->request);
    //     serve_resource(frame->conn, "/");
    //     conn_set_state(frame->conn, SOCKST_ALIVE);
    // } else if (scan("Upgrade: websocket", (char*)frame->request)) {
	// 	DEBUG("Upgrading to websocket, request was: %s\n", frame->request);
    //     if (upgrade(frame) < 0) {
    //         PFAIL(EWS, "Failed to upgrade to websocket!");
    //     }
    //     conn_set_state(frame->conn, SOCKST_OPEN_WS);
    // } else {
	// 	DEBUG("Cannot get handshake response, request was: %s\n", (char*)frame->request);
    //     conn_set_state(frame->conn, SOCKST_OPEN_WS);
    // }
	/* Get response. */
	// if (get_handshake_response((char*)frame->request, &response) < 0) {

	// 	return (-1);
	// }
	// pthread_mutex_unlock(&global_mutex);
    return 0;

    // char *hsrequest, char **hsresponse
}

void connect(void* targ) {
    Frame frame; // WebSocket frame data
	int close_timeout; // Time-out close thread
	Conn* conn = (Conn*)targ;
	bool ws_protocol = false;
	int r;
	// Prepare frame data
	memset(&frame, 0, sizeof(frame));
	frame.conn = conn;
	memcpy(frame.request, (unsigned char*)conn->request, sizeof(frame.request));
	frame.request[2048] = (unsigned char)'\0';
	frame.received = conn->received;

	DEBUG("Connecting client...\n");

	if (link(&frame) < 0) {
		PERR(ECONN, "Failed to establish connection!");
		goto closed;
	} else {
		conn_set_state(conn, SOCKST_OPEN_WS);
	}

    // TODO: Implement message passing over socket
	ws_sendframe_txt(frame.conn, "Web sockets enabled!");
	BMAG("STARTING WEBSOCKET!\n");
	/* Read next frame until conn disconnects or an error occur. */
	while (next_frame(&frame) >= 0) {
        printf("POLLING\n");
	// 	/* Text/binary event. */
		if ((frame.type == FRAME_TXT || frame.type == FRAME_BIN) && !frame.error) {
			BGRE("SENDING MESSAGE\n");
			event_manager.message(conn, frame.message, frame.size, frame.type);
			SEGH
		}
		/* Close event. */
		else if (frame.type == FRAME_CLOSE && !frame.error) {
			/*
			 * We only send a CLOSE frame once, if we're already
			 * in CLOSING state, there is no need to send.
			 */
			if (conn_get_state(conn) != SOCKST_CLOSING) {
				conn_set_state(conn, SOCKST_CLOSING);
				BYEL("CLOSE FRAME RECEIVED!\n");
				/* We only send a close frameSend close frame */
				do_close(&frame, -1);
			}
			free(frame.message);
			break;			
        }
		free(frame.message);
	}

	/*
	 * on_close events always occur, whether for conn closure
	 * or server closure, as the server is expected to
	 * always know when the conn disconnects.
	 */
	event_manager.close(conn);

// TODO: Properly shutdown threads
closed:
	close_timeout = conn->close_thread;

	/* Wait for timeout thread if necessary. */
	if (close_timeout) {
		pthread_cond_signal(&conn->state_close_cond);
		pthread_join(conn->thread_tout, NULL);
	}

	/* Close connection properly. */
	if (conn_get_state(conn) != SOCKST_CLOSED)
		close_conn(conn, 1);
	BRED("Shutting down connection!\n");
	// return targ;
    // return;
}

static void* service(void* targ) {
    struct sockaddr_in conn; /* Conn.                */
    pthread_t conn_thread;   /* Conn thread.         */
    struct timeval time;       /* Conn socket timeout. */
    SOCKET threaded_socket;    /* New opened connection. */
    int i;                     /* Loop index.            */

	// SOCKET socket = *(int*)targ;
    SOCKET socket = *(SOCKET*)targ;

	int len = sizeof(struct sockaddr_in);
    printf("Socket initialized\n");

	while (1) {
		/* Accept. */
		threaded_socket = accept(socket, (struct sockaddr *)&conn, (socklen_t *)&len);
        // SOCKET socket_thread = accept(server, (struct sockaddr*) &(conn->address), &(conn->address_length));

        printf("Threaded sockets initialized\n");

        if (timeout) {
            time.tv_sec = timeout / 1000;
            time.tv_usec = (timeout % 1000) * 1000;

            setsockopt(threaded_socket, SOL_SOCKET, SO_SNDTIMEO, &time, sizeof(struct timeval));

            printf("Setting socket timeout\n");
        }

        if (!ISVALIDSOCKET(threaded_socket)) {
            fprintf(stderr, "accept() failed. (%d)\n", SOCKERR());
            // return -1;
            PFAIL(ESERVER, "Failed to accept threaded socket\n");
            // exit(1);
        }

		/* Adds conn socket to socks list. */
		pthread_mutex_lock(&global_mutex);
		for (i = 0; i < THREAD_POOL; i++) {
			if (connections[i].socket == -1) {
				connections[i].socket = threaded_socket;
				connections[i].state = SOCKST_CONNECTING;
				connections[i].close_thread = FALSE;
				connections[i].last_pong_id = -1;
				connections[i].current_ping_id = -1;
				conn_set_address(&connections[i]);

                write_thread(i, "Intitializing connection\n");

				if (pthread_mutex_init(&connections[i].state_mutex, NULL)) {
					PERR(ESERVER, "Error on allocating close mutex");
				}
				if (pthread_cond_init(&connections[i].state_close_cond, NULL)) {
					PERR(ESERVER, "Error on allocating condition var");
				}
				if (pthread_mutex_init(&connections[i].send_mutex, NULL)) {
					PERR(ESERVER, "Error on allocating send mutex");
				}
				if (pthread_mutex_init(&connections[i].ping_mutex, NULL)) {
					PERR(ESERVER, "Error on allocating ping/pong mutex");
				}
				break;
			} else {
				PWARN(ESERVER, "Socket = -1");
			}
		}
		pthread_mutex_unlock(&global_mutex);

        printf("Thread connections initialized\n");

		/* Conn socket added to socks list ? */
		// if (i != THREAD_POOL) {
		// 	if (pthread_create(&conn_thread, NULL, connect, &connections[i])) {
        //         PERR(ESERVER, "Could not create thread!");
        //     }
		// 	pthread_detach(conn_thread);
		// } else {
		// 	// close_socket(threaded_socket);
    	// }
    }
	// free(targ);
    // return;
	// return (targ);
}