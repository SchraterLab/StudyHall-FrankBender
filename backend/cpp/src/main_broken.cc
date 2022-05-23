// #include<signal.h>

// #include "server/thread_pool.h"
// #include "server/server.h"

// typedef struct {
//   int             num_active;
//   pthread_cond_t  thread_exit;
//   pthread_mutex_t mutex;
//   int             received_shutdown_req; /* 0=false, 1=true */
// } ThreadInfo;

// ThreadInfo thread_info;

// ThreadPool* tpool;
// int job_ct = 0;

// int received_shutdown_req = 0;
// pthread_mutex_t shutdown_lock = PTHREAD_MUTEX_INITIALIZER; 
// pthread_t shutdown_thread;

// int retVal = 0;
// int* exitVal = 0;

// volatile bool accepting = true;

// void server_create(int argc, char* argv[]);
// Any server_destroy(void *arg);

// void open(Conn* conn) {
//     PLOG(LSERVER, "Opening Connection!");
//     char ipBuffer[16];
//     conn_get_address(conn, ipBuffer);
//     BGRE("Opening Connection! IP Address: %s\n", ipBuffer);
// }

// void close(Conn* conn) {
//     PLOG(LSERVER, "Closing Connection!");
//     char ipBuffer[16];
//     conn_get_address(conn, ipBuffer);
//     BRED("Closing Connection! IP Address: %s\n", ipBuffer);
// }

// void message(Conn* conn, const unsigned char* message, uint64_t size, int type) {
//     PLOG(LSERVER, "Message attempt!");
// }

// InetAddr socket_name;
// static int socket_name_len;

// #define COMM_BUF_SIZE 100

// static int    server_sockets_active;
// static int    server_sockets[MAX_CONNECTIONS];
// static fd_set server_read_set;

// static bool    server_comm_created = false;

// void handle_new_connection(void) {
//     BCYA("Processing new connection...\n");
//   int i;

//   if (server_sockets_active < MAX_CONNECTIONS) {
//     /* Find a free socket entry to use */ 
//     for (i = 1; i < MAX_CONNECTIONS; i++) {
//       if (!(FD_ISSET(server_sockets[i], &server_read_set))) {
//         if ((server_sockets[i] = accept (server_sockets[0], 
//                     (Addr*) &socket_name,
//                     (SockLen*)&socket_name_len)) < 0) {
//         PFAIL(ESOCK, "Error accepting socket!");
//         }
//         server_sockets_active++;
//         FD_SET(server_sockets[i], &server_read_set);
//         break;
//       }
//     }
//   } else {
//     int black_widow_sock;
//     fprintf(stderr, 
// 	    "Supported number of simultainious connections exceeded\n");
//     fprintf(stderr, "Ignoring client connect request\n");
//     /* There has to be a better way to ignore a connection request,..
//        when I get my hands on a sockets wiz I'll modify this */
//     black_widow_sock  = accept (server_sockets[0], 
// 				(Addr*)&socket_name,
// 				(SockLen*)&socket_name_len);
//     close(black_widow_sock);
//   }
// }

// /************************************************************************** 
//  *
//  * Local routine, reads a request off a socket indicated by a select set.
//  **************************************************************************/
// int handle_new_request(fd_set read_selects, int **connp, char **req_bufp) {
//     YEL("Handling new request!\n");
//     int i;
//     int br;

//     /* Find the descriptor */
//     for (i = 1; i < MAX_CONNECTIONS; i++) {
//         if (FD_ISSET(server_sockets[i], &read_selects)) break;
//     } 

//     /* Read from it */
//     if ((br = recv(server_sockets[i], 
//             *req_bufp, COMM_BUF_SIZE, 0)) != 0) {
//         /* Handle non-data read cases */
//         if (br == 0) {
//             /* Close socket down */
//             // FD_CLR(server_sockets[i], &server_read_set);
//             // // close(server_sockets[i]);
//             // // server_sockets_active--;
//             // return 0;
//         } else if (br < 0) {
//             PFAIL(ESOCK, "Read fail on new request!");
//         } else {
//             fprintf(stderr, 
//                 "Read, data < request buf size, ignoring data\n");
//             BMAG("r: %s\n", *req_bufp);
//         }
//         return -1;

//     } else {
//         BGRE("BREAKING!\n");
//     /* Ok, have a live one, A real data req buf has been obtained */
//     **connp =  i;
//     return 0;
//     }
// }

// void server_get_request(int *conn, char *req_buf, int port) {
//   int i, nr, not_done = 1;
//   fd_set read_selects;

//   if (!server_comm_created) socket_create(port, 1);

//   /* loop, processing new connection requests until a client buffer
//      is read in on an existing connection. */

//   while (not_done) {
//     printf("Polling!\n");
//     /* Set up the socket descriptor mask for the select.
//        copy srv_read_set, into the local copy */

//     FD_ZERO(&read_selects);
//     for (i = 0; i < MAX_CONNECTIONS; i++) {
//       if (FD_ISSET(server_sockets[i], &server_read_set)) {
//         FD_SET(server_sockets[i], &read_selects);
//       } 
//     }
    
//     printf("Blocking\n");
//     /* Poll active connections using select() */
//     if ((nr = select(FD_SETSIZE,
// 	   &read_selects, 
// 	   0,
// 	   0,
// 	   (struct timeval *)NULL)) <= 0) {
//            PFAIL(ESOCK, "Error selecting socket from fd_set!");
//     }
//     printf("Unblocking!\n");

//     if (FD_ISSET(server_sockets[0], &read_selects)) {

//        /* Handle the case of a new connection request on the named socket */
//       handle_new_connection();

//     } else {

//       /* Read data from client specific descriptor */
//       if (handle_new_request(read_selects, &conn, &req_buf) == 0) not_done = 0;
//     }

//   } /* While not_done */
//     BBLU("Returning request!\n");
// }

// void socket_create(int port, int reuse) {
//     // OLD VERSION
//     ////////////////////////////////////////////////
//     // yellow();
//     YEL("Configuring local address...\n");
//     struct addrinfo hints;
//     memset(&hints, 0, sizeof(hints));
//     hints.ai_family = AF_INET; // use AF_INET6 for IPv6 (websockets dont work with IPv6)
//     hints.ai_socktype = SOCK_STREAM; // type of socket (use DGRAM for UDP)
//     hints.ai_flags = AI_PASSIVE; // idk, something related to allowing the socket bind to be ignored if NULL

//     struct addrinfo* bind_address;
//     char cport[7];
//     snprintf(cport, 7, "%i", port);
//     getaddrinfo("127.0.0.1", cport, &hints, &bind_address);  // address to bind to

//     // create the socket
//     printf("Creating socket...\n");
//     server_sockets[0] = socket(  bind_address->ai_family, bind_address->ai_socktype, 
//                                     bind_address->ai_protocol);

//     // error check in socket creation
//     if (!ISVALIDSOCKET(server_sockets[0])) {
//         PFAIL(ESERVER, "socket() failed."); // maybe change for vargs preprocess PFAIL(...) __VA_ARGS__
//     }

//     // allow for development and reusable connection
//     if (setsockopt(server_sockets[0], SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
//         PFAIL(ESERVER, "setsockopt() with resuse flag failed.");
//     }

//     //  bind the socket to local address
//     printf("Binding socket to local address...\n");
//     if (bind(server_sockets[0], bind_address->ai_addr, bind_address->ai_addrlen)) {
//         PFAIL(ESERVER, "bind() failed."); // maybe change for vargs preprocess PFAIL(...) __VA_ARGS__
//     }
//     freeaddrinfo(bind_address);

//     // set to listen for a conn
//     printf("Listening...\n");
//     if (listen(server_sockets[0], THREAD_POOL) < 0) {
//         PFAIL(ESERVER, "listen() failed with thread_pool 5000");
//     }
//     clearcolor();
//     // return socket_listen;

//     // int i;
//     // struct hostent *host;
//     // char hostname[21]; 
    
//     // /* Initialize the connections db */
//     // server_sockets_active = 0;
//     // FD_ZERO(&server_read_set);

//     // YEL("Creating socket...\n");
//     // /* Create the socket to receive initial connection requests on */
//     // if ((server_sockets[0] = socket(AF_INET, SOCK_STREAM, 0)) < 0)  {
//     //     PFAIL(ESOCK, "Error creating socket!");
//     // }

//     // // allow for development and reusable connection
//     // if (setsockopt(server_sockets[0], SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
//     //     PFAIL(ESERVER, "setsockopt() with resuse flag failed.");
//     // }
    
//     // socket_name_len = sizeof(socket_name);

//     // /* Bind the socket to the Internet domain and a name */
//     // socket_name.sin_family = AF_INET;
//     // socket_name.sin_port = port;
//     // socket_name.sin_addr.s_addr = INADDR_ANY; 

//     // YEL("Binding socket...\n");
//     // if ((bind(server_sockets[0], (Addr*)&socket_name, sizeof(InetAddr))) < 0) {
//     //     PFAIL(ESOCK, "Error binding socket to name!");
//     // }

//     // YEL("Listening on socket...\n");
//     // /* Indicate to system to start listening on the socket */
//     // if ((listen(server_sockets[0], 5)) < 0) {
//     //     PFAIL(ESOCK, "Error listening on socket!");
//     // }
    
//     YEL("Adding socket to server set\n");
//     /* Add the socket to the server's set of active sockets */
//     FD_SET(server_sockets[0], &server_read_set);
//     server_sockets_active++;

//     server_comm_created = TRUE;
// }

// int main(int argc, char* argv[]) {
//     DEBUG(  "============================\n"
//             "          DEBUG MODE        \n"
//             "============================\n")

//     server_create(argc, argv);

//     // SOCKET server = socket_create(host, port, reuse);
//     // printf("Server socket fd: %i\n", server);
    
//     // InetAddr addr;
//     // SockLen len = sizeof(InetAddr);
//     SOCKET conn;
//     char request[2048];
//     while (1) {
//         printf("LISTENING!\n");
//         server_get_request(&conn, request, 8082);

//         BMAG("Request received! : %s\n", request);
//         // pthread_mutex_lock(&global_mutex);
//         //     accepting = false;
//         //     Conn conn;
//         //     conn_create(&conn, server_accept);
//         //     job_ct++;
//         // // for (size_t i = 0; i < num; i++) {
//         //     // vals[i] = i;
//         //     thread_pool_add(tpool, connect, (void*)&conn);
//         //     accepting = true;
//         // pthread_mutex_unlock(&global_mutex);

//         // if (getpeername(server_accept, (Addr*)&addr, (SockLen*)sizeof(InetAddr)) < 0) {
//         //     PERR(ESOCK, "Invalid Peer Name (%d)", SOCKERR());
//         // }

//         //get socket ip address as char*
//         // char ip_buffer[INET_ADDRSTRLEN];
//         // inet_ntop(AF_INET, &addr.sin_addr, ip_buffer, INET_ADDRSTRLEN);

//         // BCYA("Server socket IP Address: %s\n", ip_buffer);
//     }

//     return 0;
// }

// void server_create(int argc, char* argv[]) {
//     const char* host = "127.0.0.1";
//     const char* port = "8082";
//     int reuse = 1;
//     int retVal = 0;

//     sigset_t signals_to_block;
//     // value to thread to avoid deadlock
//     sigemptyset(&signals_to_block);
//     sigaddset(&signals_to_block, SIGINT);
//     pthread_sigmask(SIG_BLOCK, &signals_to_block, NULL);

//     /* create thread to catch shutdown signal */  
//     pthread_create(&shutdown_thread,  
//             NULL,
//             server_destroy,  
//             NULL);

//     event_manager.open = &open;
//     event_manager.close = &close;
//     event_manager.message = &message;

//     if (argc < 2) {
//         red(); printf("Usage: ./thread_pool <number of threads> <directory>\n"); clearcolor();
//         exit(1);
//     }

//     // int num = atoi(argv[0]);
//     pthread_t thread;
//     void* term;

//     yellow(); printf("Creating thread pool...\n"); clearcolor();
    
//     printf("Number of threads: %i\n", MAX_CONNECTIONS);

//     tpool = thread_pool_create(MAX_CONNECTIONS);
//     // vals = (int*)calloc(num, sizeof(int));
//     printf("Thread pool created!\n");
// }

// Any server_destroy(void *arg) {
//     sigset_t signals_to_catch;
//     int caught;
    
//     // Wait for SIGUSR1 
//     sigemptyset(&signals_to_catch);
//     sigaddset(&signals_to_catch, SIGINT);

//     sigwait(&signals_to_catch, &caught);

//     // got SIGUSR1 -- start shutdown
//     pthread_mutex_lock(&thread_info.mutex);
//     BYEL("\nShutting down...\n");
//     accepting = false;
//     thread_info.received_shutdown_req = 1;

//     // Wait for in-progress requests threads to finish */
//     while (thread_info.num_active > 0) {
//         pthread_cond_wait(&thread_info.thread_exit, 
//                 &thread_info.mutex);
//     }

//     thread_pool_wait(tpool);

//     // free(vals);
//     thread_pool_destroy(tpool);

//     pthread_mutex_unlock(&thread_info.mutex);
    
//     BGRE("Gracefully Terminated!\n");
//     PLOG(LSERVER, "Gracefully terminated!");
//     exit(0);

//     return NULL;
// }