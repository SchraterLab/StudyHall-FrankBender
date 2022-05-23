#include<signal.h>

#include "server/thread_pool.h"
#include "server/server.h"

typedef struct {
  int             num_active;
  pthread_cond_t  thread_exit;
  pthread_mutex_t mutex;
  int             received_shutdown_req; /* 0=false, 1=true */
} ThreadInfo;

ThreadInfo thread_info;

ThreadPool* tpool;
int job_ct = 0;

int received_shutdown_req = 0;
pthread_mutex_t shutdown_lock = PTHREAD_MUTEX_INITIALIZER; 
pthread_t shutdown_thread;

int retVal = 0;
int* exitVal = 0;

volatile bool accepting = true;

void server_create(int argc, char* argv[]);
Any server_destroy(void *arg);

struct Client {
    socklen_t address_length; // ip address length
    struct sockaddr_storage address; // actual address
    SOCKET socket; // socket
    char request[MAX_REQUEST_SIZE + 1]; // request buffer
    int received; // bytes received
    struct Client* next; // next client in list
    SocketState state;
    pthread_mutex_t state_mutex;
    pthread_mutex_t grade_mutex;
    bool grade;
};

typedef struct Client Client;
// #define THREAD_POOL 5000

static Client* clients = 0;

SOCKET socket_create(const char* host, int port, int reuse) {
    // OLD VERSION
    ////////////////////////////////////////////////
    // yellow();
    YEL("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // use AF_INET6 for IPv6 (websockets dont work with IPv6)
    hints.ai_socktype = SOCK_STREAM; // type of socket (use DGRAM for UDP)
    hints.ai_flags = AI_PASSIVE; // idk, something related to allowing the socket bind to be ignored if NULL

    struct addrinfo* bind_address;
    char cport[7];
    snprintf(cport, 7, "%i", port);
    getaddrinfo(host, cport, &hints, &bind_address);  // address to bind to

    // create the socket
    printf("Creating socket...\n");
    SOCKET sock = socket(bind_address->ai_family, bind_address->ai_socktype, 
                                    bind_address->ai_protocol);

    // error check in socket creation
    if (!ISVALIDSOCKET(sock)) {
        PFAIL(ESERVER, "socket() failed."); // maybe change for vargs preprocess PFAIL(...) __VA_ARGS__
    }

    // allow for development and reusable connection
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
        PFAIL(ESERVER, "setsockopt() with resuse flag failed.");
    }

    //  bind the socket to local address
    printf("Binding socket to local address...\n");
    if (bind(sock, bind_address->ai_addr, bind_address->ai_addrlen)) {
        PFAIL(ESERVER, "bind() failed."); // maybe change for vargs preprocess PFAIL(...) __VA_ARGS__
    }
    freeaddrinfo(bind_address);

    // set to listen for a conn
    printf("Listening...\n");
    if (listen(sock, 5) < 0) {
        PFAIL(ESERVER, "listen() failed with thread_pool 5000");
    }
    clearcolor();

    // YEL("Adding socket to server set\n");
    // /* Add the socket to the server's set of active sockets */
    // FD_SET(server_sockets[0], &server_read_set);
    // server_sockets_active++;

    // server_comm_created = TRUE;
    return sock;
}

SocketState client_get_state(Client* client) {
    SocketState state;
    pthread_mutex_lock(&(client->state_mutex));
    state = client->state;
    pthread_mutex_unlock(&(client->state_mutex));
    return state;
}

void client_set_grade(Client* client, bool grade) {
    pthread_mutex_lock(&(client->grade_mutex));
    client->grade = grade;
    pthread_mutex_unlock(&(client->grade_mutex));
}

bool client_get_grade(Client* client) {
    bool grade;
    pthread_mutex_lock(&(client->grade_mutex));
    grade = client->grade;
    pthread_mutex_unlock(&(client->grade_mutex));
    return grade;
}

void client_set_state(Client* client, SocketState state) {
    pthread_mutex_lock(&(client->state_mutex));
    client->state = state;
    pthread_mutex_unlock(&(client->state_mutex));
}

// finds exisiting client or creates new one if does not exist
struct Client* get_client(SOCKET s) {
    struct Client* ci = clients; // pointer to linked list

    while (ci) {
        if (ci->socket == s) break; // found client
        ci = ci->next; // iterate
    }

    if (ci) return ci; // last client

    // allocate 1 new client with block size of Client (calloc actually initializes to 0 unlike malloc)
    struct Client* n = (Client*) calloc(1, sizeof(struct Client)); 

    if (!n) { PFAIL(ESERVER, "Out of memory!"); } // not enough memory

    // set address length to size of sockaddr_storage
    n->address_length = sizeof(n->address);
    n->next = clients; // append to front of linked list
    n->state = SOCKST_NULL;
    n->grade = false;
    if (pthread_mutex_init(&n->state_mutex, NULL)) {
        PERR(ESERVER, "Failed to initialize state mutex on client!");
    }

    if (pthread_mutex_init(&n->grade_mutex, NULL)) {
        PERR(ESERVER, "Failed to initialize upgrade mutex on client!");
    }

    clients = n; // set index to front of clients
    return n; 
}

void drop_client(struct Client* client) {
    CLOSESOCKET(client->socket); // kill connection
    struct Client** p = &clients; // pointer-to-pointer
    
    // double pointer helps with case of dropped client at head of list
    while (*p) { 
        if (*p == client) { // if client
            *p = client->next; // set pointer to client
            free(client); // free memory, was allocated on heap
            return;
        }
        p = &(*p)->next; // iterate
    }

    PFAIL(ESERVER, "Error dropping client -- client not found!");
}

// blocking wait until new client or all packets from client received
fd_set wait_on_clients(SOCKET server) {
    fd_set reads; // set of file descriptors
    FD_ZERO(&reads); // set zero bits for all descriptors
    FD_SET(server, &reads); // server fd bit
    SOCKET max_socket = server; // current num of sockets
    struct Client* ci = clients; // list of clients

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

// not re-entrant safe with global variable
void client_get_address(struct Client* ci, char* dest) {
    char address_buffer[16]; // char array to store IP address, static so erased after function termination
    getnameinfo((struct sockaddr*)&ci->address, ci->address_length, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
    strncpy(dest, address_buffer, INET_ADDRSTRLEN);
}

void serve_resource(Client* conn, const char* path) {
    char addr_buffer[16];
    client_get_address(conn, addr_buffer);
    printf("serve_resource %s %s\n", addr_buffer, path);

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
    // drop_client(conn); // close conn connection
    // close thread
}


void parse_response(Client* client) {
    BGRE("AH AH AH AH STAYIN ALIVE\n");
    if (strncmp("GET /", client->request, 5)) {
        send_400(client->socket);
    } else {
        char* path = client->request + 4; // removes "GET "
        char* end_path = strstr(path, " "); // finds first occurence of " "
        if (!end_path) {
            send_400(client->socket); // none terminating path
        } else {
            *end_path = 0; // zero out char
            // if (ws_protocol) { // if websocket protocol detected
            //     green(); printf("Starting websocket\n"); clearcolor();
            //     // test case: "dGhlIHNhbXBsZSBub25jZQ=="
            //     // web_socket = TRUE;
            //     handshake(client, path, real_key); // encrypts key and establishes socket connection
            //     // handshake(client, path, "dGhlIHNhbXBsZSBub25jZQ=="); // test case
            //     // ws_protocol = FALSE;
            // } else {
                serve_resource(client, path); // static page serving
            // }
        }
    }
}

void open(Conn* conn) {
    PLOG(LSERVER, "Opening Connection!");
    char ipBuffer[16];
    conn_get_address(conn, ipBuffer);
    BGRE("Opening Connection! IP Address: %s\n", ipBuffer);
}

void close(Conn* conn) {
    PLOG(LSERVER, "Closing Connection!");
    char ipBuffer[16];
    conn_get_address(conn, ipBuffer);
    BRED("Closing Connection! IP Address: %s\n", ipBuffer);
    Client* client = get_client(conn->socket);
    drop_client(client);
}

void message(Conn* conn, const unsigned char* message, uint64_t size, int type) {
    PLOG(LSERVER, "Message attempt!");
    char addr_buffer[16];
    conn_get_address(conn, addr_buffer);
	printf("I receive a message: %s (size: %ld, type: %d), from: %s\n", message, size, type, addr_buffer);
}

int main(int argc, char* argv[]) {
    server_create(argc, argv);
    // for windows
    #ifdef _WIN32
        struct WSAData d;
        if (WSAStartup(MAKEWORD(2, 2), &d)) {
            fprintf(stderr, "Failed to initialize.\n");
        }
    #endif

    BOOL web_socket = FALSE;

    SOCKET server = socket_create("127.0.0.1", 8082, 1); // creates initial socket
    // char* real_key = (char*)calloc(1, 100);
    BOOL ws_protocol = FALSE;

    while (1) {
        fd_set reads;
        reads = wait_on_clients(server);

        if (FD_ISSET(server, &reads)) {
            struct Client* client = get_client(-1);

            client->socket = accept(server, (struct sockaddr*) &(client->address), &(client->address_length));

            if (!ISVALIDSOCKET(client->socket)) {
                fprintf(stderr, "accept() failed. (%d)\n", SOCKERR());
                return 1;
            }

            char address_buffer[16];
            client_get_address(client, address_buffer);
            printf("New connection from %s.\n", address_buffer);
        }

        struct Client* client = clients;

        while (client) {
            
            // iterate through clients
            struct Client* next = client->next;

            if (FD_ISSET(client->socket, &reads)) {
                // max request, minor ddos prevention
                if (MAX_REQUEST_SIZE == client->received) {
                    printf("MAX REQUEST SIZE\n");
                    send_400(client->socket);
                    continue;
                }

                // receives bytes from client and asserts against request limit
                int r = recv(client->socket, client->request + client->received, MAX_REQUEST_SIZE - client->received, 0); 

                purple();
                // printf("%s\n", client->request); // prints incoming client request, helps with concurrency testing
                // printf("Size of r is: %i", r);
                clearcolor();

                // may want to shutdown if r == 0

                if (r > 0) { // no bytes received

                    client->received += r; // increment bytes received
                    client->request[client->received] = 0; 
                    char* q = strstr(client->request, "\r\n\r\n");

                    // pthread_mutex_lock(&global_mutex);
                    if (q) {

                        BMAG("Request: %s\n", client->request);


                        if (scan("Connection: keep-alive", client->request)) {
                            client_set_state(client, SOCKST_ALIVE);
                        } else if (scan("Connection: closed", client->request)) {
                            client_set_state(client, SOCKST_CLOSING);
                        } else if (scan("Connection: Upgrade", client->request)) {
                            client_set_state(client, SOCKST_UPGRADING);
                        } else {
                            BRED("NULL REQUEST\n");
                        }

                        SocketState state;
                        switch(state = client_get_state(client)) {
                            case SOCKST_ALIVE:
                                parse_response(client);
                                memset(client->request, 0, strlen(client->request));
                                break;
                            case SOCKST_CLOSING:
                                drop_client(client);
                                break;
                            case SOCKST_UPGRADING: 
                                BGRE("ATTEMPTING TO UPGRADE TO WEBSOCKET!\n");
                                Conn conn;
                                printf("Socket fd: %i\n", client->socket);
                                conn_create(&conn, client->socket, client->request, client->received);
                                client_set_state(client, SOCKST_OPEN_WS);
                                thread_pool_add(tpool, connect, (void*)&conn);
                                memset(client->request, 0, strlen(client->request));
                                break;
                            default:
                                break;
                        }
                    }
                    // pthread_mutex_unlock(&global_mutex);
                }
            }
            client = next;
        }
    }


    return 0;
}


void server_create(int argc, char* argv[]) {
    const char* host = "127.0.0.1";
    const char* port = "8082";
    int reuse = 1;
    int retVal = 0;

    sigset_t signals_to_block;
    // value to thread to avoid deadlock
    sigemptyset(&signals_to_block);
    sigaddset(&signals_to_block, SIGINT);
    pthread_sigmask(SIG_BLOCK, &signals_to_block, NULL);

    /* create thread to catch shutdown signal */  
    pthread_create(&shutdown_thread,  
            NULL,
            server_destroy,  
            NULL);

    event_manager.open = &open;
    event_manager.close = &close;
    event_manager.message = &message;

    if (argc < 2) {
        red(); printf("Usage: ./thread_pool <number of threads> <directory>\n"); clearcolor();
        exit(1);
    }

    pthread_t thread;
    void* term;

    yellow(); printf("Creating thread pool...\n"); clearcolor();
    
    printf("Number of threads: %i\n", MAX_CONNECTIONS);

    tpool = thread_pool_create(MAX_CONNECTIONS);
    // vals = (int*)calloc(num, sizeof(int));
    printf("Thread pool created!\n");
}

Any server_destroy(void *arg) {
    sigset_t signals_to_catch;
    int caught;
    
    // Wait for SIGUSR1 
    sigemptyset(&signals_to_catch);
    sigaddset(&signals_to_catch, SIGINT);

    sigwait(&signals_to_catch, &caught);

    // got SIGUSR1 -- start shutdown
    pthread_mutex_lock(&thread_info.mutex);
    BYEL("\nShutting down...\n");
    accepting = false;
    thread_info.received_shutdown_req = 1;

    // Wait for in-progress requests threads to finish */
    while (thread_info.num_active > 0) {
        pthread_cond_wait(&thread_info.thread_exit, 
                &thread_info.mutex);
    }

    thread_pool_wait(tpool);

    // free(vals);
    thread_pool_destroy(tpool);

    pthread_mutex_unlock(&thread_info.mutex);
    
    BGRE("Gracefully Terminated!\n");
    PLOG(LSERVER, "Gracefully terminated!");
    exit(0);

    return NULL;
}