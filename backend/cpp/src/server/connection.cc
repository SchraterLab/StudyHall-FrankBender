#include "server/connection.h"

// OLD
///////////////////////////////////////////////////////////////////////////////
// finds exisiting conn or creates new one if does not exist
Conn* get_conn(SOCKET s) {
    Conn* ci = conns; // pointer to linked list

    while (ci) {
        if (ci->socket == s) break; // found conn
        ci = ci->next; // iterate
    }

    if (ci) return ci; // last conn

    // allocate 1 new conn with block size of conn (calloc actually initializes to 0 unlike malloc)
    Conn* n = (Conn*)calloc(1, sizeof(Conn)); 

    if (!n) { PFAIL(ESERVER, "Out of memory!"); } // not enough memory

    // set address length to size of sockaddr_storage
    n->address_length = sizeof(n->address);
    n->next = conns; // append to front of linked list
    conns = n; // set index to front of conns
    return n; 
}

void drop_conn(Conn* conn) {
    CLOSESOCKET(conn->socket); // kill connection
    Conn** p = &conns; // pointer-to-pointer
    
    // double pointer helps with case of dropped conn at head of list
    while (*p) { 
        if (*p == conn) { // if conn
            *p = conn->next; // set pointer to conn
            free(conn); // free memory, was allocated on heap
            return;
        }
        p = &(*p)->next; // iterate
    }

    PFAIL(ESERVER, "Error dropping conn -- conn not found!");
}


// THREAD POOL
///////////////////////////////////////////////////////////////////////
void conn_set_address(Conn* conn) {
	InetAddr iaddr;

	if (!ISVALIDSOCKET(conn)) { PFAIL(ECONN, "Setting invalid address on server"); }
	SockLen size = sizeof(InetAddr);

	if (getpeername(conn->socket, (Addr*)&iaddr, &size) < 0) {
        PFAIL(ECONN, "Invalid peer name");
    }

	// memset((char*)conn->address, 0, sizeof(conn->address));
	inet_ntop(AF_INET, &iaddr.sin_addr, conn->address, INET_ADDRSTRLEN);
}

int ws_get_state(Conn *conn) {
	return (conn_get_state(conn));
}

// may not be necessary with conn_set_address
// not re-entrant safe with global variable
void conn_get_address(Conn* conn, char* dest) {
	if (!ISVALIDSOCKET(conn)) { PFAIL(ECONN, "Setting invalid address on server"); }
    // static char address_buffer[100]; // char array to store IP address, static so erased after function termination
    // getnameinfo((struct sockaddr*)&ci->address, ci->address_length, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
    // return address_buffer;
	strncpy(dest, conn->address, INET_ADDRSTRLEN);
}

// blocking wait until new conn or all packets from conn received
fd_set wait_on_conns(SOCKET server) {
    fd_set reads; // set of file descriptors
    FD_ZERO(&reads); // set zero bits for all descriptors
    FD_SET(server, &reads); // server fd bit
    SOCKET max_socket = server; // current num of sockets
    Conn* ci = conns; // list of conns

    while (ci) {
        FD_SET(ci->socket, &reads); // set fd bit of socket
        if (ci->socket > max_socket) { // determine if max socket
            max_socket = ci->socket; // set new max
        }
        ci = ci->next; // iterate
    }

    // waits for sockets in fd_set to be available for access
    // params: (num of fds + 1, read fds, write fds, exceptional conditions fds, timeout for blocking)
    // use pselect with 6th param for sigmask to ignore certain signals on threads
    if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
        // fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
        // exit(1);
        PFAIL(ESERVER, "select() failed");
    }

    return reads;
}

SocketState conn_get_state(Conn* conn) {
	SOCK_SAN(conn->socket);
	SocketState state;
	pthread_mutex_lock(&conn->state_mutex);
	state = conn->state;
	pthread_mutex_unlock(&conn->state_mutex);
	return state;
}

void conn_set_state(Conn* conn, SocketState state) {
	SOCK_SAN(conn->socket);
	pthread_mutex_lock(&conn->state_mutex);
	conn->state = state;
	pthread_mutex_unlock(&conn->state_mutex);
}

void close_conn(Conn* conn, int lock) {
	if (!ISVALIDSOCKET(conn)) return;

	conn_set_state(conn, SOCKST_CLOSED);

	socket_close(conn->socket);

	/* Destroy conn mutexes and clear fd 'slot'. */
	/* clang-format off */
	if (lock)
		pthread_mutex_lock(&global_mutex);
			conn->socket = -1;
			pthread_cond_destroy(&conn->state_close_cond);
			pthread_mutex_destroy(&conn->state_mutex);
			pthread_mutex_destroy(&conn->send_mutex);
			pthread_mutex_destroy(&conn->ping_mutex);
	if (lock)
		pthread_mutex_unlock(&global_mutex);
	/* clang-format on */
}

int ws_close_conn(Conn *conn) {
	unsigned char clse_code[2];
	int cc;

	/* Check if conn is a valid and connected conn. */
	if (!ISVALIDSOCKET(conn) || conn->socket == -1)
		return (-1);

	/*
	 * Instead of using do_close(), we use this to avoid using
	 * payload buffer from wfd and avoid a race condition
	 * if this is invoked asynchronously.
	 */
	cc = WS_CLSE_NORMAL;
	clse_code[0] = (cc >> 8);
	clse_code[1] = (cc & 0xFF);
	if (ws_sendframe(conn, (const char *)clse_code, sizeof(char) * 2, FRAME_CLOSE) < 0) {
		PERR(ESERVER, "An error has occurred while sending closing frame!\n");
		return -1;
	}

	/*
	 * Starts the timeout thread: if the conn did not send
	 * a close frame in TIMEOUT_MS milliseconds, the server
	 * will close the connection with error code (1002).
	 */
	start_close_timeout(conn);
	return 0;
}

void conn_create(Conn* conn, SOCKET s) {
	SOCK_SAN(s)
	conn->socket = s;
	conn->state = SOCKST_CONNECTING;
	conn->close_thread = FALSE;
	conn->last_pong_id = -1;
	conn->current_ping_id = -1;
	conn_set_address(conn);	
	if (pthread_mutex_init(&conn->state_mutex, NULL)) {
		PERR(ESERVER, "Error on allocating close mutex");
	}
	if (pthread_cond_init(&conn->state_close_cond, NULL)) {
		PERR(ESERVER, "Error on allocating condition var");
	}
	if (pthread_mutex_init(&conn->send_mutex, NULL)) {
		PERR(ESERVER, "Error on allocating send mutex");
	}
	if (pthread_mutex_init(&conn->ping_mutex, NULL)) {
		PERR(ESERVER, "Error on allocating ping/pong mutex");
	}
}

void conn_create(Conn* conn, SOCKET s, char* request, int received) {
	SOCK_SAN(s)
	conn->socket = s;
	conn->state = SOCKST_CONNECTING;
	conn->close_thread = FALSE;
	conn->last_pong_id = -1;
	conn->current_ping_id = -1;
	strncpy(conn->request, request, 2047);
	conn->request[2048] = '\0';
	conn->received = received;
	conn_set_address(conn);	
	if (pthread_mutex_init(&conn->state_mutex, NULL)) {
		PERR(ESERVER, "Error on allocating close mutex");
	}
	if (pthread_cond_init(&conn->state_close_cond, NULL)) {
		PERR(ESERVER, "Error on allocating condition var");
	}
	if (pthread_mutex_init(&conn->send_mutex, NULL)) {
		PERR(ESERVER, "Error on allocating send mutex");
	}
	if (pthread_mutex_init(&conn->ping_mutex, NULL)) {
		PERR(ESERVER, "Error on allocating ping/pong mutex");
	}
}