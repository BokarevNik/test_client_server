#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>
#include <arpa/inet.h>

#define SERVER_ARGS_NUM 3
#define INIT_DIR_IDX 1
#define PORT_IDX 2

#define NAME_MAX 255    
#define PATH_MAX 4096
#define CMD_SIZE 7

#define CMD_REMOVE "remove"
#define CMD_CREATE "create"

void createFile(const char *path) {
    std::ofstream file(path);
    if (!file) {
            std::cerr << "Error: failed to create file on server" << std::endl;
            std::cerr << path << std::endl;
            exit(1);
    }
    file.close();
}

void removeFile(const char *path) {
    if (remove(path)) {
        std::cerr << "Server failed to remove file" << std::endl;
        exit(1);
    }
}

int main(int argc, char *argv[]) {
     if (argc != SERVER_ARGS_NUM) {
        std::cerr << "Not right amount of server arguments" << std::endl;
        return -1;
    }

    char buffer[NAME_MAX + PATH_MAX + CMD_SIZE] = "";
    char cmd[CMD_SIZE] = "";
    char relative_path[PATH_MAX] = "";


    int master_socket;
    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(atoi(argv[PORT_IDX]));
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    master_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (master_socket == -1) {
         std::cerr << "Error creating server socket" << std::endl;
         return -1;
    }
    
    int optval = 1;
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) {
        std::cerr << "Error: server setsockopt()" << std::endl;
        return -1;
    }

    if (bind(master_socket, (struct sockaddr *) (&sock_addr), sizeof(sock_addr))) {
        std::cerr << "Error: server bind()" << std::endl;
        return -1;
    }

    if (listen(master_socket, SOMAXCONN)) {
        std::cerr << "Error: server listen()" << std::endl;
        return -1;
    }

    while (true) {
        int slave_socket = 0;
        if ((slave_socket = accept(master_socket, 0, 0)) == -1) {
            std::cerr << "Error: server accept()" << std::endl;
            return -1;
        }

        char path[PATH_MAX] = "";

        if (recv(slave_socket, buffer, sizeof(buffer), MSG_NOSIGNAL) == -1) {
            std::cerr << "Error: server recv()" << std::endl;
            return -1;
        }

        //parsing incoming buffer
        strncpy(cmd, buffer, CMD_SIZE);
        cmd[CMD_SIZE - 1] = '\0';
        strncpy(relative_path, buffer + CMD_SIZE - 1, PATH_MAX);
        //creating full filepath
        strcat(path, argv[INIT_DIR_IDX]);
        strcat(path, relative_path);

        if (!strcmp(cmd, CMD_CREATE)) {
            createFile(path);
        } else if (!strcmp(cmd, CMD_REMOVE)){
            removeFile(path);
        }

        if (shutdown(slave_socket, SHUT_RDWR)) {
            std::cerr << "Error: server shutdown()" << std::endl;
            return -1;
        }

        if (close(slave_socket)) {
            std::cerr << "Error: server close()" << std::endl;
            return -1;
        }
    }

    if (shutdown(master_socket, SHUT_RDWR)) {
            std::cerr << "Error: server shutdown()" << std::endl;
            return -1;
    }

    if (close(master_socket)) {
            std::cerr << "Error: server close()" << std::endl;
            return -1;
    }

    return 0;
}