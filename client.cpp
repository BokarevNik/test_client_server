#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CLIENT_ARGS_NUM 6
#define ADDR_IDX 1
#define PORT_IDX 2
#define CMD_IDX 3
#define DIR_IDX 4
#define FNAME_IDX 5

#define CMD_REMOVE "remove"
#define CMD_CREATE "create"

#define NAME_MAX 255    
#define PATH_MAX 4096
#define CMD_SIZE 7

int main(int argc, char *argv[]) {
    if (argc != CLIENT_ARGS_NUM) {
        std::cerr << "Not right amount of client arguments" << std::endl;
        return -1;
    }

    //one buffer is sent to socker, starting with the command of fixed size and then filepath
    char buffer[NAME_MAX + PATH_MAX + CMD_SIZE] = "";
    strcat(buffer, argv[CMD_IDX]);
    if (strcmp(argv[DIR_IDX], "/")) {
        strcat(buffer, argv[DIR_IDX]);
    }
    strcat(buffer, "/");
    strcat(buffer, argv[FNAME_IDX]);

    int socket_fd;
    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(atoi(argv[PORT_IDX]));
    if (!inet_aton(argv[ADDR_IDX], &sock_addr.sin_addr)) {
        std::cerr << "inet_aton: invalid IP" << std::endl;
        return -1;
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == -1) {
        std::cerr << "Error creating client socket" << std::endl;
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr *) (&sock_addr), sizeof(sock_addr))) {
        std::cerr << "Error: creating client connect" << std::endl;
        return -1;
    }

    if (send(socket_fd, buffer, sizeof(buffer), MSG_NOSIGNAL) == -1) {
        std::cerr << "Error: client send()" << std::endl;
        return -1;
    }

    if (shutdown(socket_fd, SHUT_RDWR)) {
        std::cerr << "Error: client shutdown()" << std::endl;
        return -1;
    }

    if (close(socket_fd)) {
        std::cerr << "Error: client shutdown()" << std::endl;
        return -1;
    }

    return 0;
}