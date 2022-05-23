#ifndef SERVER_DEFS_H_
#define SERVER_DEFS_H_

#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#endif 

#ifndef IPV6_V6ONLY
#define IPV6_V6ONLY 27
#endif

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <regex.h>

#include "util/error.h"
#include "crypt/base64.h"
#include "crypt/sha1.h"
#include "crypt/utf8.h"

#define BOOL int
#define TRUE 1
#define FALSE 0

// SHA-1 hash

#ifndef _WIN32
#define SOCKET int
#endif

// poor implementation
static uint32_t timeout;
typedef struct sockaddr_in InetAddr;
typedef struct sockaddr Addr;
typedef struct sockaddr_storage AddrStorage;
typedef socklen_t SockLen;

// -1 to avoid buffer overflow
#define MAX_REQUEST_SIZE 4095
#define THREAD_POOL 1
#define MAX_CONNECTIONS 100

// may be a poor implentation
static pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
typedef void* Any;
typedef enum SocketState {
    SOCKST_NULL,
    SOCKST_CONNECTING,
    SOCKST_CLOSING,
    SOCKST_OPEN_WS,
    SOCKST_UPGRADING,
    SOCKST_ALIVE,
    SOCKST_CLOSED
} SocketState;

typedef struct Conn {
    socklen_t address_length; // ip address length
    char address[16]; // actual address
    SOCKET socket; // socket
    SocketState state;
    char request[MAX_REQUEST_SIZE + 1]; // request buffer
    int received; // bytes received
    struct Conn* next; // next conn in list
    pthread_mutex_t state_mutex; // mutex for state
    pthread_mutex_t send_mutex; // sending mutex
	pthread_cond_t state_close_cond; // condition close for websocket shutdown
	pthread_t thread_tout; // timeout
	pthread_mutex_t ping_mutex;
    BOOL close_thread;
    int32_t last_pong_id;
	int32_t current_ping_id;
} Conn;

// TEST: re-entry
static Conn* conns = 0;
static Conn connections[THREAD_POOL];

#ifdef _WIN32
// #define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET && s >= (int)&connections[0] && (int)&connections[THREAD_POOL -1])
#define CLOSESOCKET(s) closesocket(s)
#define SOCKERR() (WSAGetLastError())
#else
// #define ISVALIDSOCKET(s) ((s) != NULL && s >= &connections[0] && &connections[THREAD_POOL -1])
#define SOCKET int
#define CLOSESOCKET(s) close(s)
#define SOCKERR() (errno)
#endif

#define FIN      128
#define FRAME_FIN  7
#define FRAME_CONT 0
#define FRAME_TXT  1
#define FRAME_BIN  2
#define FRAME_CLOSE 8
#define FRAME_PING 0x9
#define FRAME_PONG 0xA
#define FRAME_INV 0xF
#define WS_CLSE_NORMAL  1000
#define WS_CLSE_PROTERR 1002
#define WS_CLSE_INVUTF8 1007
#define MS_TO_NS(x) ((x)*1000000)
#define TIMEOUT_MS (500)
#define MAX_FRAME_LENGTH (16*1024*1024)
#define WS_HEADER                          \
    "HTTP/1.1 101 Switching Protocols\r\n" \
    "Upgrade: websocket\r\n"               \
    "Connection: Upgrade\r\n"              \
    "Sec-WebSocket-Accept: "
#define KEY_LEN     24
#define MAGIC_STRING_LEN      36
#define PLAIN_LEN (KEY_LEN + MAGIC_STRING_LEN)
#define MAGIC_STRING   "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define WS_REQUEST      "Sec-WebSocket-Key"
#define ACCEPT_LEN   130 	// Handshake accept message length.

#define ISVALIDSOCKET(cli) ((cli) != 0)
#define ARRAY_SIZE(arr) (sizeof((arr))/sizeof((arr)[0]))

#define SOCK_SAN(s) if (!ISVALIDSOCKET(s)) { PFAIL(ESOCK, "Not a valid socket!"); }

// fd_set server_connections;

struct Frame {
	unsigned char request[MAX_REQUEST_SIZE + 1]; // Frame read.
	unsigned char* message; // Processed message at the moment.
	unsigned char payload[125]; // Control frame payload
	size_t cur_pos; // Current byte position.
	size_t received; // Amount of read bytes.
	int type; // Frame type, like text or binary.
	uint64_t size; // Frame size.
	int error; // Error flag, set when a read was not possible.
	Conn* conn; // Conn connection structure.
};

#endif