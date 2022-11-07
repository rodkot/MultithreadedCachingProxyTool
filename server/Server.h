//
// Created by rodion on 03.11.22.
//

#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <poll.h>
#include "../connection/Request.h"
#include "../connection/Response.h"

#define ACTIVE 0
#define STOP -1
#define DELETE -2

class Client;

class Server {
public:
    int fd{};
    int status = ACTIVE;
    pollfd *poll = nullptr;
    Request *request = nullptr;
    Response *response = nullptr;
    Client *client = nullptr;

    int current_send_reques = 0;
   // long current_recv_response = 0;
public:
    explicit Server(Client *c);

    void request_mode_enable() const;

    void request_mode_disable() const;

    void response_mode_enable() const;

    void response_mode_disable() const;

    ~Server();
};


#endif //PROXY_SERVER_H
