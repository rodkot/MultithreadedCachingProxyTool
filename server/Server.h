//
// Created by rodion on 03.11.22.
//

#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <poll.h>
#include <string>
#include <sys/socket.h>
#include "../connection/Request.h"
#include "../connection/Response.h"


#define ACTIVE_SERVER 1
#define STOP_SUCCESS_SERVER 2
#define STOP_FAILED_SERVER -2

class Client;

class Server {
private:
    int fd{};
    int status = ACTIVE_SERVER;
    pollfd *poll = nullptr;
    Request *request = nullptr;
    Response *response = nullptr;
    Client *client = nullptr;

    sockaddr *addr = nullptr;
    int current_send_reques = 0;


public:
    explicit Server(Client *c);

    std::string get_name_server() const;

    void request_mode_enable() const;

    void request_mode_disable() const;

    void response_mode_enable() const;

    void response_mode_disable() const;

    int getFd() const;

    void setFd(int fd);

    int getStatus() const;

    void setStatus(int status);

    pollfd *getPoll() const;

    void setPoll(pollfd *poll);

    Request *getRequest() const;

    void setRequest(Request *request);

    Response *getResponse() const;

    void setResponse(Response *response);

    Client *getClient() const;

    void setClient(Client *client);

    sockaddr *getAddr() const;

    void setAddr(sockaddr *addr);

    int getCurrentSendReques() const;

    void setCurrentSendReques(int currentSendReques);


    ~Server();
};


#endif //PROXY_SERVER_H
