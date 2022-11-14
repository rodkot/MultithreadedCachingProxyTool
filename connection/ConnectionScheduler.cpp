//
// Created by rodion on 03.11.22.
//

#include "ConnectionScheduler.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <termcolor.hpp>

ConnectionScheduler::ConnectionScheduler(char *address, int port) : address(address), port(port) {}

int ConnectionScheduler::listen_socket() {
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    struct sockaddr_in mysock{};

    mysock.sin_family = AF_INET;
    mysock.sin_port = htons(port);
    mysock.sin_addr.s_addr = inet_addr(address);


    if (bind(fd, (sockaddr *) (&mysock), sizeof(mysock)) < 0) {
        return OPEN_CONNECTION_FAILED;
    }
    if (listen(fd, MAX_CLIENT)) {
        return OPEN_CONNECTION_FAILED;
    }
    fd_connect = fd;
    return fd;
}

int ConnectionScheduler::fresh_connection(Client *client) const {
    client->addr = (sockaddr *) calloc(1, sizeof(sockaddr));
    socklen_t add_size = sizeof(*(client->addr));
    int client_fd = accept(fd_connect, client->addr, &add_size);
    if (client_fd < 0) {
        return CLIENT_CONNECTION_FAILED;
    }
    client->fd = client_fd;
    client->poll = (pollfd *) calloc(1, sizeof(pollfd));
    client->poll->fd = client_fd;
    client->poll->events = POLLHUP;

    return CLIENT_CONNECTION_SUCCESS;
}

void ConnectionScheduler::disconnect() const {
    close(fd_connect);
}