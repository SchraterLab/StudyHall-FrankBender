#include "server/socket.h"

void socket_close(int fd) {
#ifndef _WIN32
	shutdown(fd, SHUT_RDWR);
	close(fd);
#else
	closesocket(fd);
#endif
}

extern EventManager event_manager;

const char* get_content_type(const char* path) {
    const char* last_dot = strrchr(path, '.');
    if (last_dot) {
        if (strcmp(last_dot, ".css") == 0) return "text/css";
        if (strcmp(last_dot, ".csv") == 0) return "text/csv";
        if (strcmp(last_dot, ".gif") == 0) return "text/gif";
        if (strcmp(last_dot, ".htm") == 0) return "text/htm";
        if (strcmp(last_dot, ".html") == 0) return "text/html";
        if (strcmp(last_dot, ".ico") == 0) return "image/x-icon";
        if (strcmp(last_dot, ".jpeg") == 0) return "image/jpeg";
        if (strcmp(last_dot, ".jpg") == 0) return "image/jpeg";
        if (strcmp(last_dot, ".js") == 0) return "application/javascript";
        if (strcmp(last_dot, ".json") == 0) return "application/json";
        if (strcmp(last_dot, ".png") == 0) return "image/png";
        if (strcmp(last_dot, ".pdf") == 0) return "application/pdf";
        if (strcmp(last_dot, ".svg") == 0) return "image/svg+xml";
        if (strcmp(last_dot, ".txt") == 0) return "text/plain";
    }

    return "application/octet-stream";
}

// encrypts the key to authenticate websocket handshake
char* socket_key(const char* subkey) {
    // test subkey is dGhlIHNhbXBsZSBub25jZQ==
    char bufkey[strlen(subkey)];
    strncpy(bufkey, subkey, strlen(subkey) + 1);
    purple();
    printf("Bufkey is: %s\n", bufkey);
    char* key = bufkey; // key from conn
    const char* magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; // magic string
    strcat(key, magic);
    printf("key is: %s\n", key);
    char digest[512]; 
    // sha1(digest, key, strlen(key));

    // from github
    #define WS_KEY_LEN     24
	#define WS_MS_LEN      36
	#define WS_KEYMS_LEN   (WS_KEY_LEN + WS_MS_LEN)
	// unsigned char hash[SHA1HashSize]; /* SHA-1 Hash. */
	// SHA1Context ctx;  
	// SHA1Reset(&ctx);
	// SHA1Input(&ctx, (const uint8_t *)digest, WS_KEYMS_LEN);
	// SHA1Result(&ctx, hash);

    // // hello world
    // char shaTest1[512];
    // char shaTest2[512];
    // const char* ex1 = "hello world";
    // // SHA1(hello world) -> 2aae6c35c94fcfb415dbe95f408b9ce91ee846ed
    // // SHA1(GeeksForGeeks) -> addf120b430021c36c232c99ef8d926aea2acd6b
    // printf("Digest is: %u\n", hash);
    // clearcolor();
    // const char* base_digest = digest;
    const char* base_digest = "Hello World!";
    // char* result = b64_encode(hash, strlen((char*)hash));
    unsigned char* dest;
    unsigned char hash[SHA1HashSize];
	SHA1Context ctx;                  
	char *str;    

	str = (char*)calloc(1, sizeof(char) * (WS_KEY_LEN + WS_MS_LEN + 1));
	strncpy(str, key, WS_KEY_LEN);
	strcat(str, magic);

	SHA1Reset(&ctx);
	SHA1Input(&ctx, (const uint8_t *)str, WS_KEYMS_LEN);
	SHA1Result(&ctx, hash); // hash (key + magic string)

	dest = base64_encode(hash, SHA1HashSize, NULL); // encode hash
	// (dest + strlen((const char *)dest) - 1) = '\0';
	free(str);
    
    static char result[24];
    strcpy(result, (char*)dest);

    // test cases
    const char* goal = "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=";
    // const char* magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    // char* ext = b64_encode((const unsigned char*)"258EAFA5-E914-47DA-95CA-C5AB0DC85B11", strlen(base_digest));
    // yellow(); clearcolor();
    purple(); printf("GOAL: %s\n", goal); clearcolor();
    purple(); printf("BASE_DIGEST: %s\n", result); clearcolor();
    // if (strcmp(base_digest, goal) == 0) {
    //     bold(); green();
    //     printf("SUCCESS!!!!!\n");
    //     clearcolor();
    // }
    return result;
}

void handshake_response(Conn* conn, const char* path, const char* subkey) {
    const char* key = socket_key(subkey);
    // static test cases
    printf("COMPARE: %s = %s\n", key, "SGVsbG8gV29ybGQh");
    printf("COMPARE: %s = %s\n", key, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
    // Hello World! ->(Base64) SGVsbG8gV29ybGQh
    // 2nd one is for magic string websocket handshake
    char* buf = (char*)calloc(1, 75 + 50);
    const char* handshake = "HTTP/1.1 101 Switching Protocols\r\n"
                            "Upgrade: websocket\r\n"
                            "Connection: Upgrade\r\n"
                            "Sec-WebSocket-Accept: ";

    strncpy(buf, handshake, strlen(handshake));
    strcat(buf, key);
    strcat(buf, "\r\n\r\n");

    if (send(conn->socket, buf, strlen(buf), 0) == -1) {
        PERR(ESERVER, "Failed to send data in handshake");
    }
    blue(); printf("SENT: %s", buf); clearcolor();
    free(buf);
}

void serve_resource(Conn* conn, const char* path) {
    char addr_buffer[16];
    conn_get_address(conn, addr_buffer);
    green(); printf("serve_resource %s %s\n", addr_buffer, path); clearcolor();

    // if at root file
    if (strcmp(path, "/") == 0) path = "/index.html";

    // security precaution for long url, ex: https://localhost:8080////////////////////index.html
    // and buffer overflow
    // if (strlen(path) > 100) { 
    //     send_400(conn->socket);
    //     return;
    // }

    // security precaution to prevent unpriveleged access https://localhost:8080/../../keys/key.pem
    // if (strstr(path, "..")) {
    //     send_404(conn->socket);
    //     return;
    // }

    //may need more sanitization (xss attack prevention with regex/octet substition)
    // csrf tokens for same origin verification
    // session authentication for login 
    // timeouts for inactivity

    char full_path[128];
    sprintf(full_path, "frontend%s", "/index.html");

#ifdef _WIN32 // dumb windows and their FAT file system
    char* p = full_path;
    while (*p) {
        if (*p == '/') *p = '\\';
        ++p;
    }
#endif

    FILE* fp = fopen(full_path, "rb"); // open file, set fds to read in bytes

    if (!fp) {
        send_404(conn->socket); // file does not exist
        return;
    }

    fseek(fp, 0L, SEEK_END); // move to end of file
    size_t sz = ftell(fp); // get size of file in bytes
    rewind(fp); // set back to beginning of file

    // get content-type i.e. text/html, application/json
    const char* content = get_content_type(full_path); 

    // #define BSIZE 1024
    char buffer[4096];

    sprintf(buffer, "HTTP/1.1 200 OK\r\n");
    send(conn->socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Connection: close\r\n");
    send(conn->socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Content-Length: %lu\r\n", sz);
    send(conn->socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Content-Type: %s\r\n", content);
    send(conn->socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "\r\n");
    send(conn->socket, buffer, strlen(buffer), 0);

    // read file contents into multiple 1024 packets
    int r = fread(buffer, 1, 1024, fp);
    while (r) {
        send(conn->socket, buffer, r, 0); // send bytes
        r = fread(buffer, 1, 1024, fp); // read another 1024
    }

    fclose(fp); // close file
    // drop_conn(conn); // close conn connection
    // close thread
}

ssize_t broadcast(Conn *conn, const void *buf, size_t len, int flags) {
	const char *p;
	ssize_t ret;

	if (!ISVALIDSOCKET(conn)) { PERR(ESERVER, "Trying to send invalid socket"); return -1; }

	p = (const char*)buf;

	pthread_mutex_lock(&conn->send_mutex);
		while (len) {
			ret = send(conn->socket, p, len, flags);
			if (ret == -1) {
                PERR(ESERVER, "Nothing sent to conn");
				pthread_mutex_unlock(&conn->send_mutex);
				return -1;
			}
			p += ret;
			len -= ret;
		}
	pthread_mutex_unlock(&conn->send_mutex);

	return 0;
}

// extern "C" int main() {
//     yellow(); printf("Starting server...\n"); clearcolor();
//     int threadloop = 1;
// 	pthread_t accept_thread;   /* Accept thread. */
// 	struct sockaddr_in server_addr; /* Server. */

// 	timeout = 5000;

// 	event.onopen    = &onopen;
// 	event.onclose   = &onclose;
// 	event.onmessage = &onmessage;

// // for windows
// #ifdef _WIN32
//     struct WSAData d;
//     if (WSAStartup(MAKEWORD(2, 2), &d)) {
//         fprintf(stderr, "Failed to initialize.\n");
//     }
// #endif

//     BOOL web_socket = FALSE;
//     int* server = (int*)malloc(sizeof(int));
//     // *server = create_socket("127.0.0.1", "8081", 1); // creates initial socket

// 	/* Create socket. */
// 	*server = socket(AF_INET, SOCK_STREAM, 0);
// 	if (*server < 0) {
// 		PERR(ESERVER, "Could not create socket");
// 	}
// 	/* Reuse previous address. */
// 	int reuse = 1;
// 	if (setsockopt(*server, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0) {
// 		PERR(ESERVER, "setsockopt(SO_REUSEADDR) failed");
// 	}

// 	/* Prepare the sockaddr_in structure. */
// 	server_addr.sin_family = AF_INET;
// 	server_addr.sin_addr.s_addr = INADDR_ANY;
// 	server_addr.sin_port = htons(8081);

// 	/* Bind. */
// 	if (bind(*server, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
// 		PERR(ESERVER, "Bind failed");
// 	}

// 	/* Listen. */
// 	listen(*server, THREAD_POOL);


// 	printf("Waiting for incoming connections...\n");
// 	memset(connections, -1, sizeof(connections));
//     printf("Connections set\n");
//     printf("SOCKET is: %i\n", *server);
// 	// Accept connections
// 	if (!threadloop) {
//         printf("Non threaded accept\n");
// 		service((void*)server);
// 	} else {
//         printf("Threaded accept\n");
// 		if (pthread_create(&accept_thread, NULL, service, (void*)server)) {
//             PERR(ESERVER, "Could not create conn thread!");
//         }
// 		pthread_detach(accept_thread);
// 	}

//     free(server);

//     char* real_key = (char*)calloc(1, 100);
//     BOOL ws_protocol = FALSE;

//     while (1) {

//     printf("New connection from %s.\n", get_conn_address(conn));

//     struct Conn* conn = conns;

//     while (conn) {
        
//         // iterate through conns
//         struct Conn* next = conn->next;

//         if (FD_ISSET(conn->socket, &reads)) {
//             // max request, minor ddos prevention
//             if (MAX_REQUEST_SIZE == conn->received) {
//                 printf("MAX REQUEST SIZE\n");
//                 send_400(conn);
//                 continue;
//             }

//             if (web_socket == FALSE) { // need to evolve to mutexes

//                 // receives bytes from conn and asserts against request limit
//                 int r = recv(conn->socket, conn->request + conn->received, MAX_REQUEST_SIZE - conn->received, 0); 

//                 //     printf("WS KEY IS: %s\n", ws_key);
//                 //     const char* prefix = "Sec-WebSocket-Key: ";
//                 //     strncpy(real_key, &ws_key[0] + strlen(prefix), sizeof(&ws_key[0]) + strlen(prefix));
//                 //     printf("DAS KEY: %s\n", real_key);
//                 //     printf("KEY LENGTH IS: %i\n", (int)strlen(real_key));
//                 // }

//                 if (r < 1) { // no bytes received
//                     red(); printf("Unexpected disconnect from %s.\n", get_conn_address(conn)); clearcolor();
//                     drop_conn(conn);
//                 } else {
//                     conn->received += r; // increment bytes received
//                     conn->request[conn->received] = 0; 
//                     char* q = strstr(conn->request, "\r\n\r\n");

//                     if (q) {
//                         if (strncmp("GET /", conn->request, 5)) {
//                             send_400(conn);
//                         } else {
//                             char* path = conn->request + 4; // removes "GET "
//                             char* end_path = strstr(path, " "); // finds first occurence of " "
//                             if (!end_path) {
//                                 send_400(conn); // none terminating path
//                             } else {
//                                 *end_path = 0; // zero out char
//                                 if (ws_protocol) { // if websocket protocol detected
//                                     green(); printf("Starting websocket\n"); clearcolor();
//                                     // test case: "dGhlIHNhbXBsZSBub25jZQ=="
//                                     // web_socket = TRUE;
//                                     handshake(conn, path, real_key); // encrypts key and establishes socket connection
//                                     // handshake(conn, path, "dGhlIHNhbXBsZSBub25jZQ=="); // test case
//                                     // ws_protocol = FALSE;
//                                 } else {
//                                     serve_resource(conn, path); // static page serving
//                                 }
//                             }
//                         }
//                     }
//                 }
//             }
//         }
//         conn = next;
//     } // while (conn)
//     // free(real_key);
// } // while (1)
// printf("Closing socket...\n");
// CLOSESOCKET(server);

// #ifdef _WIN32
//     WSACleanup();
// #endif

//     printf("\033[1;32mTerminating Gracefully.\n\033[0m");

//     return 0;
// }
