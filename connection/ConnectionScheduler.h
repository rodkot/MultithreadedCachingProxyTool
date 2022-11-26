//
// Created by rodion on 03.11.22.
//

#ifndef PROXY_CONNECTIONSCHEDULER_H
#define PROXY_CONNECTIONSCHEDULER_H

#include <sys/socket.h>
#include "../Configuration.h"
#include "../client/Client.h"

#define OPEN_CONNECTION_FAILED -1
#define OPEN_CONNECTION_SUCCESS 1

#define CONNECTION_SERVER_FAILED -1
#define CONNECTION_SERVER_SUCCESS 1

#define CLIENT_CONNECTION_FAILED -1
#define CLIENT_CONNECTION_SUCCESS 1

class ConnectionScheduler {
private:
    int fd_connect{};
    struct sockaddr* proxy_sockaddr = nullptr;
    char *address;
    int port;

public:

    ConnectionScheduler(char *address, int port);

    int connect_to_server(Server* server);

    int listen_socket();

    int fresh_connection(Client *client) const;

    void disconnect() const;

};


#endif //PROXY_CONNECTIONSCHEDULER_H
