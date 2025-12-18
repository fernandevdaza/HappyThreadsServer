#include "server.h"
#include "http.h"
#include "logger.h"
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

int main(int argc, char *argv[])
{
//    signal(SIGPIPE, SIG_IGN);
    
    int enable_logging = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--log") == 0 || strcmp(argv[i], "-l") == 0) {
            enable_logging = 1;
            break;
        }
    }
    
    if (enable_logging) {
        init_logger();
    }
    
    init_thread_pool();
    int server_fd = create_socket_and_listen(8000);

    if (!is_logging_enabled()) {
        printf("Servidor HTTP escuchando en puerto 8000...\n");
    }

    while (1)   
    {
        log_event(ID_PRODUCER, -1, "Sleep", "Waiting for connection");
        int client_fd = accept_connection(server_fd);
        if (client_fd < 0)
            continue;

        log_event(ID_PRODUCER, client_fd, "Running", "New connection accepted");
        enqueue_client(client_fd);
        log_event(ID_PRODUCER, client_fd, "Ready", "Client added to queue");
    }
    return 0;
}