//
// Created by rodion on 03.11.22.
//

#ifndef PROXY_CLIENT_H
#define PROXY_CLIENT_H

#include <poll.h>
#include <sys/socket.h>
#include <string>
#include "../connection/Request.h"
#include "../server/Server.h"

class Client {
public:
    int fd = 0 ;
    int status = ACTIVE;
    sockaddr *addr = nullptr;
    Server *server = nullptr;
    Request *request = nullptr;
    Response *response = nullptr;
    pollfd *poll = nullptr;

    long current_recv_response = 0;

public:
    explicit Client();

    std::string get_name_client();

    void asking_mode_enable() const;

    void asking_mode_disable() const;

    void receiving_mode_enable() const;

    void receiving_mode_disable() const;

    ~Client();
};


#endif //PROXY_CLIENT_H
