// #include "printcolor.hpp"
// #include "algo.hpp"
// #include "file_system.hpp"
// #include "error.hpp"
// #include "test.hpp"
// #include "lib.hpp"
// // #include "server.h"

// #include <stdio.h>
// #include <signal.h>
// #include <unistd.h>

// #define DEBUG

// extern "C" void run() {

// }

// void sig_handler(int signo)
// {
//     if (signo == SIGUSR1) {
//         printf("received SIGUSR1\n");
//         exit(1);
//     } else if (signo == SIGKILL) {
//         printf("received SIGKILL\n");
//         exit(1);
//     } else if (signo == SIGSTOP) {
//         printf("received SIGSTOP\n");
//         exit(1);
//     }
// }

// extern "C" int main(int argc, char* argv[]) {
//     yellow();
//     printf("Starting prizm...\n");
//     clearcolor();

//     if (signal(SIGKILL, sig_handler) == SIG_ERR)
//         printf("Can't catch signal!");

//     if (signal(SIGUSR1, sig_handler) == SIG_ERR)
//         printf("Can't catch signal!");

//     if (signal(SIGSTOP, sig_handler) == SIG_ERR)
//         printf("Can't catch signal!");

//     if (argc > 1) {

// // #ifdef DEBUG
// //     printf("--------------------------------\n");
// //     printf("            DEBUGGING           \n");
// //     printf("--------------------------------\n");
// // #endif
//     printf("1\n");
//     int port = std::atoi(argv[1]);
//     printf("2\n");
//     std::string webDir = std::string(argv[2]);
//     printf("3\n");
//     // WebServer<Server> server(port, webDir);
//     while (true) {
//         // server.service();
//         printf("Polling -- %i - %s\n", port, webDir.c_str());
//     }
    
//     }
//     else {
//         std::cout << "Usage: ./bin/ExampleServer 8081 web" << std::endl;
//     }
//     return 0;
// }