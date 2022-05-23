#include "server/response.h"

void send_400(SOCKET socket) {
    const char* c400 = "HTTP/1.1 400 Bad Request\r\n"
                        "Connection: close\r\n"
                        "Content-length: 11\r\n\r\nBad Request";
    send(socket, c400, strlen(c400), 0);
    // drop_client(client);
}

void send_404(SOCKET socket) {
    const char* c404 = "HTTP/1.1 404 Not Found\r\n"
                    "Connection: close\r\n"
                    "Content-length: 9\r\n\r\nNot Found";
    send(socket, c404, strlen(c404), 0);
    // drop_client(client);
}