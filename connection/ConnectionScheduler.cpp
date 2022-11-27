//
// Created by rodion on 03.11.22.
//

#include "ConnectionScheduler.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <termcolor.hpp>
#include <netdb.h>
#include <cstring>

ConnectionScheduler::ConnectionScheduler(char *address, int port) : address(address), port(port) {
    proxy_sockaddr = (sockaddr *) calloc(1, sizeof(sockaddr));
}

int ConnectionScheduler::listen_socket() {
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    auto *sock = (sockaddr_in *) proxy_sockaddr;
    sock->sin_family = AF_INET;
    sock->sin_port = htons(port);
    sock->sin_addr.s_addr = inet_addr(address);

    if (bind(fd, (proxy_sockaddr), sizeof(*proxy_sockaddr)) < 0) {
        return OPEN_CONNECTION_FAILED;
    }
    if (listen(fd, MAX_CLIENT)) {
        return OPEN_CONNECTION_FAILED;
    }
    fd_connect = fd;
    return fd;
}

int ConnectionScheduler::connect_to_server(Server *server,Request* request) {
    struct addrinfo hints, *res, *result;

    char *url = request->getHost();

    int port_server = 80;
    char host_name[100];
    if (sscanf(url, "%99[^:]:%i[^\r]", host_name, &port_server) != 2) {
        sscanf(url, "%99[^\r]", host_name);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    int errcode = getaddrinfo(host_name, std::to_string(port_server).c_str(), &hints, &result);

    if (errcode != 0) {
        return CONNECTION_SERVER_FAILED;
    }
    auto *server_sockaddr_in = (sockaddr_in *) (result->ai_addr);
    auto *proxy_sockadd_in = (sockaddr_in *) (proxy_sockaddr);

    if (ntohl(server_sockaddr_in->sin_addr.s_addr) == ntohl(proxy_sockadd_in->sin_addr.s_addr)
        && ntohs(server_sockaddr_in->sin_port) == ntohs(proxy_sockadd_in->sin_port)) {
        return CONNECTION_SERVER_FAILED;
    }

    res = result;
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    while (res != nullptr) {
        if (connect(fd, result->ai_addr, result->ai_addrlen) == 0) {
            server->setFd(fd);
            server->setAddr(result->ai_addr);
            server->setPoll((pollfd *) calloc(1, sizeof(pollfd)));
            server->getPoll()->fd = fd;
            server->getPoll()->events = POLLHUP | POLLERR;
            return CONNECTION_SERVER_SUCCESS;
        }
        res = res->ai_next;
    }
    return CONNECTION_SERVER_FAILED;
}

int ConnectionScheduler::fresh_connection(Client *client) const {
    client->setAddr((sockaddr *) calloc(1, sizeof(sockaddr)));
    socklen_t add_size = sizeof(*(client->getAddr()));
    int client_fd = accept(fd_connect, client->getAddr(), &add_size);
    if (client_fd < 0) {
        return CLIENT_CONNECTION_FAILED;
    }
    client->setFd(client_fd);
    auto *p = (pollfd *) calloc(1, sizeof(pollfd));

    p->fd = client_fd;
    p->events = POLLHUP;

    client->setPoll(p);

    return CLIENT_CONNECTION_SUCCESS;
}

void ConnectionScheduler::disconnect() const {
    close(fd_connect);
    delete proxy_sockaddr;
}