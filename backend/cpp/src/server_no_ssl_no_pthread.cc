// #if defined(_WIN32)
// #ifndef _WIN32_WINNT
// #define _WIN32_WINNT 0x0600
// #endif
// #include <winsock2.h>
// #include <ws2tcpip.h>
// #pragma comment(lib, "ws2_32.lib")

// #else
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <netdb.h>
// #include <unistd.h>
// #include <errno.h>
// #endif 

// #ifdef _WIN32
// #define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
// #define CLOSESOCKET(s) closesocket(s)
// #define GETSOCKETERRNO() (WSAGetLastError())

// #else
// #define ISVALIDSOCKET(s) ((s) >= 0)
// #define CLOSESOCKET(s) close(s)
// #define SOCKET int
// #define GETSOCKETERRNO() (errno)
// #endif

// #ifndef IPV6_V6ONLY
// #define IPV6_V6ONLY 27
// #endif

// #include <stdio.h>
// // #include "color.h"
// #include <string.h>
// #include <time.h>
// #include "lib.hpp"
// #include <regex.h>
// #include "util/error.h"
// #include "crypt/base64.h"

// #define BOOL int
// #define TRUE 1
// #define FALSE 0

// // SHA-1 hash
// extern "C" {
// #include "sha1.h"
// }

// #define ARRAY_SIZE(arr) (sizeof((arr))/sizeof((arr)[0]))

// // const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// // size_t b64_encoded_size(size_t inlen)
// // {
// // 	size_t ret;

// // 	ret = inlen;
// // 	if (inlen % 3 != 0)
// // 		ret += 3 - (inlen % 3);
// // 	ret /= 3;
// // 	ret *= 4;

// // 	return ret;
// // }

// // char *b64_encode(const unsigned char *in, size_t len)
// // {
// // 	char   *out;
// // 	size_t  elen;
// // 	size_t  i;
// // 	size_t  j;
// // 	size_t  v;

// // 	if (in == NULL || len == 0)
// // 		return NULL;

// // 	elen = b64_encoded_size(len);
// // 	out  = (char*)malloc(elen+1);
// // 	out[elen] = '\0';

// // 	for (i=0, j=0; i<len; i+=3, j+=4) {
// // 		v = in[i];
// // 		v = i+1 < len ? v << 8 | in[i+1] : v << 8;
// // 		v = i+2 < len ? v << 8 | in[i+2] : v << 8;

// // 		out[j]   = b64chars[(v >> 18) & 0x3F];
// // 		out[j+1] = b64chars[(v >> 12) & 0x3F];
// // 		if (i+1 < len) {
// // 			out[j+2] = b64chars[(v >> 6) & 0x3F];
// // 		} else {
// // 			out[j+2] = '=';
// // 		}
// // 		if (i+2 < len) {
// // 			out[j+3] = b64chars[v & 0x3F];
// // 		} else {
// // 			out[j+3] = '=';
// // 		}
// // 	}

// // 	return out;
// // }

// const char* get_content_type(const char* path) {
//     const char* last_dot = strrchr(path, '.');
//     if (last_dot) {
//         if (strcmp(last_dot, ".css") == 0) return "text/css";
//         if (strcmp(last_dot, ".csv") == 0) return "text/csv";
//         if (strcmp(last_dot, ".gif") == 0) return "text/gif";
//         if (strcmp(last_dot, ".htm") == 0) return "text/htm";
//         if (strcmp(last_dot, ".html") == 0) return "text/html";
//         if (strcmp(last_dot, ".ico") == 0) return "image/x-icon";
//         if (strcmp(last_dot, ".jpeg") == 0) return "image/jpeg";
//         if (strcmp(last_dot, ".jpg") == 0) return "image/jpeg";
//         if (strcmp(last_dot, ".js") == 0) return "application/javascript";
//         if (strcmp(last_dot, ".json") == 0) return "application/json";
//         if (strcmp(last_dot, ".png") == 0) return "image/png";
//         if (strcmp(last_dot, ".pdf") == 0) return "application/pdf";
//         if (strcmp(last_dot, ".svg") == 0) return "image/svg+xml";
//         if (strcmp(last_dot, ".txt") == 0) return "text/plain";
//     }

//     return "application/octet-stream";
// }

// SOCKET create_socket(const char* host, const char* port) {
//     printf("Configuring local address...\n");
//     struct addrinfo hints;
//     memset(&hints, 0, sizeof(hints));
//     hints.ai_family = AF_INET; // use AF_INET6 for IPv6 (websockets dont work with IPv6)
//     hints.ai_socktype = SOCK_STREAM; // type of socket (use DGRAM for UDP)
//     hints.ai_flags = AI_PASSIVE; // idk, something related to allowing the socket bind to be ignored if NULL

//     struct addrinfo* bind_address;
//     getaddrinfo(host, port, &hints, &bind_address);  // address to bind to

//     // create the socket
//     printf("Creating socket...\n");
//     SOCKET socket_listen;
//     socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);

//     // allow for development and reusable connection
//     int enable = 1;
//     if (setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
//         // PFAIL(ESERVER, "REUSE SOCKET ERROR.");
//     }

//     // maybe add another sockopt

//     // error check in socket creation
//     if (!ISVALIDSOCKET(socket_listen)) {
//         // fprintf(stderr, "Socket() failed. (%d)\n", GETSOCKETERRNO());
//         // exit(1);
//         PFAIL(ESERVER, "Socket() failed."); // maybe change for vargs preprocess PFAIL(...) __VA_ARGS__
//     }

//     //  bind the socket to local address
//     printf("Binding socket to local address...\n");
//     if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
//         fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
//         exit(1);
//     }
//     freeaddrinfo(bind_address);

//     // set to listen for a client
//     printf("Listening...\n");
//     if (listen(socket_listen, 10) < 0) {
//         fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
//         exit(1);
//     }

//     return socket_listen;

// }

// #define MAX_REQUEST_SIZE 4095

// struct client_info {
//     socklen_t address_length; // ip address length
//     struct sockaddr_storage address; // actual address
//     SOCKET socket; // socket
//     char request[MAX_REQUEST_SIZE + 1]; // request buffer
//     int received; // bytes received
//     struct client_info* next; // next client in list
//     pthread_mutex_t mtx_state; // mutex for state
//     pthread_mutex_t mtx_send; // sending mutex
// 	pthread_cond_t cnd_state_close; // condition close for websocket shutdown
// 	pthread_t thrd_tout; // timeout
// };

// #define THREAD_POOL 5000

// static struct client_info* clients = 0;

// // finds exisiting client or creates new one if does not exist
// struct client_info* get_client(SOCKET s) {
//     struct client_info* ci = clients; // pointer to linked list

//     while (ci) {
//         if (ci->socket == s) break; // found client
//         ci = ci->next; // iterate
//     }

//     if (ci) return ci; // last client

//     // allocate 1 new client with block size of client_info (calloc actually initializes to 0 unlike malloc)
//     struct client_info* n = (struct client_info*) calloc(1, sizeof(struct client_info)); 

//     if (!n) { PFAIL(ESERVER, "Out of memory!"); } // not enough memory

//     // set address length to size of sockaddr_storage
//     n->address_length = sizeof(n->address);
//     n->next = clients; // append to front of linked list
//     clients = n; // set index to front of clients
//     return n; 
// }

// void drop_client(struct client_info* client) {
//     CLOSESOCKET(client->socket); // kill connection
//     struct client_info** p = &clients; // pointer-to-pointer
    
//     // double pointer helps with case of dropped client at head of list
//     while (*p) { 
//         if (*p == client) { // if client
//             *p = client->next; // set pointer to client
//             free(client); // free memory, was allocated on heap
//             return;
//         }
//         p = &(*p)->next; // iterate
//     }

//     PFAIL(ESERVER, "Error dropping client -- client not found!");
// }

// // not re-entrant safe with global variable
// const char* get_client_address(struct client_info* ci) {
//     static char address_buffer[100]; // char array to store IP address, static so erased after function termination
//     getnameinfo((struct sockaddr*)&ci->address, ci->address_length, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
//     return address_buffer;
// }

// // blocking wait until new client or all packets from client received
// fd_set wait_on_clients(SOCKET server) {
//     fd_set reads; // set of file descriptors
//     FD_ZERO(&reads); // set zero bits for all descriptors
//     FD_SET(server, &reads); // server fd bit
//     SOCKET max_socket = server; // current num of sockets
//     struct client_info* ci = clients; // list of clients

//     while (ci) {
//         FD_SET(ci->socket, &reads); // set fd bit of socket
//         if (ci->socket > max_socket) { // determine if max socket
//             max_socket = ci->socket; // set new max
//         }
//         ci = ci->next; // iterate
//     }

//     // waits for sockets in fd_set to be available for access
//     // params: (num of fds + 1, read fds, write fds, exceptional conditions fds, timeout for blocking)
//     // use pselect with 6th param for sigmask to ignore certain signals on threads
//     if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
//         // fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
//         // exit(1);
//         PFAIL(ESERVER, "select() failed");
//     }

//     return reads;
// }


// // intuitive
// void send_400(struct client_info* client) {
//     const char* c400 = "HTTP/1.1 400 Bad Request\r\n"
//                         "Connection: close\r\n"
//                         "Content-length: 11\r\n\r\nBad Request";
//     send(client->socket, c400, strlen(c400), 0);
//     drop_client(client);
// }

// void send_404(struct client_info* client) {
//     const char* c404 = "HTTP/1.1 404 Not Found\r\n"
//                     "Connection: close\r\n"
//                     "Content-length: 9\r\n\r\nNot Found";
//     send(client->socket, c404, strlen(c404), 0);
//     drop_client(client);
// }


// // encrypts the key to authenticate websocket handshake
// char* socket_key(const char* subkey) {
//     // test subkey is dGhlIHNhbXBsZSBub25jZQ==
//     char bufkey[strlen(subkey)];
//     strncpy(bufkey, subkey, strlen(subkey) + 1);
//     purple();
//     printf("Bufkey is: %s\n", bufkey);
//     char* key = bufkey; // key from client
//     const char* magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; // magic string
//     strcat(key, magic);
//     printf("key is: %s\n", key);
//     char digest[512]; 
//     // sha1(digest, key, strlen(key));

//     // from github
//     #define WS_KEY_LEN     24
// 	#define WS_MS_LEN      36
// 	#define WS_KEYMS_LEN   (WS_KEY_LEN + WS_MS_LEN)
// 	// unsigned char hash[SHA1HashSize]; /* SHA-1 Hash. */
// 	// SHA1Context ctx;  
// 	// SHA1Reset(&ctx);
// 	// SHA1Input(&ctx, (const uint8_t *)digest, WS_KEYMS_LEN);
// 	// SHA1Result(&ctx, hash);

//     // // hello world
//     // char shaTest1[512];
//     // char shaTest2[512];
//     // const char* ex1 = "hello world";
//     // // SHA1(hello world) -> 2aae6c35c94fcfb415dbe95f408b9ce91ee846ed
//     // // SHA1(GeeksForGeeks) -> addf120b430021c36c232c99ef8d926aea2acd6b
//     // printf("Digest is: %u\n", hash);
//     // clearcolor();
//     // const char* base_digest = digest;
//     const char* base_digest = "Hello World!";
//     // char* result = b64_encode(hash, strlen((char*)hash));
//     unsigned char* dest;
//     unsigned char hash[SHA1HashSize];
// 	SHA1Context ctx;                  
// 	char *str;    

// 	str = (char*)calloc(1, sizeof(char) * (WS_KEY_LEN + WS_MS_LEN + 1));
// 	strncpy(str, key, WS_KEY_LEN);
// 	strcat(str, magic);

// 	SHA1Reset(&ctx);
// 	SHA1Input(&ctx, (const uint8_t *)str, WS_KEYMS_LEN);
// 	SHA1Result(&ctx, hash); // hash (key + magic string)

// 	dest = base64_encode(hash, SHA1HashSize, NULL); // encode hash
// 	// (dest + strlen((const char *)dest) - 1) = '\0';
// 	free(str);
    
//     static char result[24];
//     strcpy(result, (char*)dest);

//     // test cases
//     const char* goal = "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=";
//     // const char* magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
//     // char* ext = b64_encode((const unsigned char*)"258EAFA5-E914-47DA-95CA-C5AB0DC85B11", strlen(base_digest));
//     // yellow(); clearcolor();
//     purple(); printf("GOAL: %s\n", goal); clearcolor();
//     purple(); printf("BASE_DIGEST: %s\n", result); clearcolor();
//     // if (strcmp(base_digest, goal) == 0) {
//     //     bold(); green();
//     //     printf("SUCCESS!!!!!\n");
//     //     clearcolor();
//     // }
//     return result;
// }

// void handshake(struct client_info* client, const char* path, const char* subkey) {
//     const char* key = socket_key(subkey);
//     // static test cases
//     printf("COMPARE: %s = %s\n", key, "SGVsbG8gV29ybGQh");
//     printf("COMPARE: %s = %s\n", key, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
//     // Hello World! ->(Base64) SGVsbG8gV29ybGQh
//     // 2nd one is for magic string websocket handshake
//     char* buf = (char*)calloc(1, 75 + 50);
//     const char* handshake = "HTTP/1.1 101 Switching Protocols\r\n"
//                             "Upgrade: websocket\r\n"
//                             "Connection: Upgrade\r\n"
//                             "Sec-WebSocket-Accept: ";

//     strncpy(buf, handshake, strlen(handshake));
//     strcat(buf, key);
//     strcat(buf, "\r\n\r\n");

//     if (send(client->socket, buf, strlen(buf), 0) == -1) {
//         PERR(ESERVER, "Failed to send data in handshake");
//     }
//     blue(); printf("SENT: %s", buf); clearcolor();
//     free(buf);
// }

// void serve_resource(struct client_info* client, const char* path) {
//     green(); printf("serve_resource %s %s\n", get_client_address(client), path); clearcolor();

//     // if at root file
//     if (strcmp(path, "/") == 0) path = "/index.html";

//     // security precaution for long url, ex: https://localhost:8080////////////////////index.html
//     // and buffer overflow
//     // if (strlen(path) > 100) { 
//     //     send_400(client);
//     //     return;
//     // }

//     // security precaution to prevent unpriveleged access https://localhost:8080/../../keys/key.pem
//     // if (strstr(path, "..")) {
//     //     send_404(client);
//     //     return;
//     // }

//     //may need more sanitization (xss attack prevention with regex/octet substition)
//     // csrf tokens for same origin verification
//     // session authentication for login 
//     // timeouts for inactivity

//     char full_path[128];
//     sprintf(full_path, "frontend%s", "/index.html");

// #ifdef _WIN32 // dumb windows and their FAT file system
//     char* p = full_path;
//     while (*p) {
//         if (*p == '/') *p = '\\';
//         ++p;
//     }
// #endif

//     FILE* fp = fopen(full_path, "rb"); // open file, set fds to read in bytes

//     if (!fp) {
//         send_404(client); // file does not exist
//         return;
//     }

//     fseek(fp, 0L, SEEK_END); // move to end of file
//     size_t sz = ftell(fp); // get size of file in bytes
//     rewind(fp); // set back to beginning of file

//     // get content-type i.e. text/html, application/json
//     const char* content = get_content_type(full_path); 

//     // #define BSIZE 1024
//     char buffer[4096];

//     sprintf(buffer, "HTTP/1.1 200 OK\r\n");
//     send(client->socket, buffer, strlen(buffer), 0);

//     sprintf(buffer, "Connection: close\r\n");
//     send(client->socket, buffer, strlen(buffer), 0);

//     sprintf(buffer, "Content-Length: %lu\r\n", sz);
//     send(client->socket, buffer, strlen(buffer), 0);

//     sprintf(buffer, "Content-Type: %s\r\n", content);
//     send(client->socket, buffer, strlen(buffer), 0);

//     sprintf(buffer, "\r\n");
//     send(client->socket, buffer, strlen(buffer), 0);

//     // read file contents into multiple 1024 packets
//     int r = fread(buffer, 1, 1024, fp);
//     while (r) {
//         send(client->socket, buffer, r, 0); // send bytes
//         r = fread(buffer, 1, 1024, fp); // read another 1024
//     }

//     fclose(fp); // close file
//     drop_client(client); // close client connection

// }

// extern "C" int main() {

//     // for windows
//     #ifdef _WIN32
//         struct WSAData d;
//         if (WSAStartup(MAKEWORD(2, 2), &d)) {
//             fprintf(stderr, "Failed to initialize.\n");
//         }
//     #endif

//     BOOL web_socket = FALSE;

//     SOCKET server = create_socket("127.0.0.1", "8081"); // creates initial socket
//     char* real_key = (char*)calloc(1, 100);
//     BOOL ws_protocol = FALSE;

//     while (1) {
//         fd_set reads;
//         reads = wait_on_clients(server);

//         if (FD_ISSET(server, &reads)) {
//             struct client_info* client = get_client(-1);

//             client->socket = accept(server, (struct sockaddr*) &(client->address), &(client->address_length));

//             if (!ISVALIDSOCKET(client->socket)) {
//                 fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
//                 return 1;
//             }

//             printf("New connection from %s.\n", get_client_address(client));
//         }

//         struct client_info* client = clients;

//         while (client) {
            
//             // iterate through clients
//             struct client_info* next = client->next;

//             if (FD_ISSET(client->socket, &reads)) {
//                 // max request, minor ddos prevention
//                 if (MAX_REQUEST_SIZE == client->received) {
//                     printf("MAX REQUEST SIZE\n");
//                     send_400(client);
//                     continue;
//                 }

//                 if (web_socket == FALSE) { // need to evolve to mutexes

//                     // receives bytes from client and asserts against request limit
//                     int r = recv(client->socket, client->request + client->received, MAX_REQUEST_SIZE - client->received, 0); 

//                     purple();
//                     printf("%s\n", client->request); // prints incoming client request, helps with concurrency testing
//                     printf("Size of r is: %i", r);
//                     clearcolor();

//                     // the following regex scanners detect if websocket protocol and extracts key if so
//                     #define WS_REGEX "Upgrade: websocket"
//                     regex_t regex;
//                     regmatch_t match[4];
//                     regoff_t off, len;
//                     char* str = client->request;
//                     char* s = str;
//                     char ws_key[100];
//                     int value = regcomp(&regex, WS_REGEX, 0);
//                     if (value) {
//                         fprintf(stderr, "Failed to compile regex!\n");
//                         exit(1);
//                     }

//                     yellow(); 
//                     printf("Regex: %s\n", WS_REGEX);
//                     printf("Matches: \n");
//                     for (int i = 0; ; i++) {
//                         if (regexec(&regex, s, ARRAY_SIZE(match), match, 0))
//                             break;
//                         ws_protocol = TRUE;
//                         off = match[0].rm_so + (s - str);
//                         len = match[0].rm_eo - match[0].rm_so;
//                         printf("#%d:\n", i);
//                         printf("offset = %jd; length = %jd\n", (intmax_t) off,
//                                 (intmax_t) len);
//                         printf("substring = \"%.*s\"\n", len, s + match[0].rm_so);

//                         s += match[0].rm_eo;
//                     }
//                     clearcolor();
//                     regfree(&regex);

//                     if (ws_protocol == TRUE) {
//                         memset(match, 0, ARRAY_SIZE(match)); // clear match array
//                         str = client->request;
//                         s = str;
//                         value = regcomp(&regex, "Sec-WebSocket-Key: (.)+==", REG_EXTENDED);
//                         if (value) {
//                             fprintf(stderr, "Failed to compile regex!\n");
//                             exit(1);
//                         }
//                         yellow(); 
//                         printf("KEY Matches: \n");
//                         for (int i = 0; ; i++) {
//                             if (regexec(&regex, s, ARRAY_SIZE(match), match, 0))
//                                 break;
//                             ws_protocol = TRUE;
//                             off = match[0].rm_so + (s - str);
//                             len = match[0].rm_eo - match[0].rm_so;
//                             printf("#%d:\n", i);
//                             printf("offset = %jd; length = %jd\n", (intmax_t) off,
//                                     (intmax_t) len);
//                             printf("substring = \"%.*s\"\n", len, s + match[0].rm_so);
//                             strncpy(ws_key, s + match[0].rm_so, len);
//                             s += match[0].rm_eo;
//                         }
//                         clearcolor();
//                         regfree(&regex);

//                         printf("WS KEY IS: %s\n", ws_key);
//                         const char* prefix = "Sec-WebSocket-Key: ";
//                         strncpy(real_key, &ws_key[0] + strlen(prefix), sizeof(&ws_key[0]) + strlen(prefix));
//                         printf("DAS KEY: %s\n", real_key);
//                         printf("KEY LENGTH IS: %i\n", (int)strlen(real_key));
//                     }

//                     printf("FUCK ME\n");

//                     if (r < 1) { // no bytes received
//                         red(); printf("Unexpected disconnect from %s.\n", get_client_address(client)); clearcolor();
//                         drop_client(client);
//                     } else {
//                         client->received += r; // increment bytes received
//                         client->request[client->received] = 0; 
//                         char* q = strstr(client->request, "\r\n\r\n");

//                         if (q) {
//                             if (strncmp("GET /", client->request, 5)) {
//                                 send_400(client);
//                             } else {
//                                 char* path = client->request + 4; // removes "GET "
//                                 char* end_path = strstr(path, " "); // finds first occurence of " "
//                                 if (!end_path) {
//                                     send_400(client); // none terminating path
//                                 } else {
//                                     *end_path = 0; // zero out char
//                                     if (ws_protocol) { // if websocket protocol detected
//                                         green(); printf("Starting websocket\n"); clearcolor();
//                                         // test case: "dGhlIHNhbXBsZSBub25jZQ=="
//                                         // web_socket = TRUE;
//                                         handshake(client, path, real_key); // encrypts key and establishes socket connection
//                                         // handshake(client, path, "dGhlIHNhbXBsZSBub25jZQ=="); // test case
//                                         // ws_protocol = FALSE;
//                                     } else {
//                                         serve_resource(client, path); // static page serving
//                                     }
//                                 }
//                             }
//                         }
//                     }
//                 }
//             }
//             client = next;
//         } // while (client)
//         // free(real_key);
//     } // while (1)
//     printf("Closing socket...\n");
//     CLOSESOCKET(server);

// #ifdef _WIN32
//     WSACleanup();
// #endif

//     printf("\033[1;32mTerminating Gracefully.\n\033[0m");

//     return 0;
// }