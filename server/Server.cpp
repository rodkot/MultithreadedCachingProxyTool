//
// Created by rodion on 03.11.22.
//

#include "Server.h"
#include "../client/Client.h"
#include <netinet/in.h>
#include <arpa/inet.h>

Server::Server(Client *c):request(c->request),client(c){}
void Server::request_mode_enable() const {
    if (poll!= nullptr){
        if (!(poll->events & POLLOUT))
            poll->events ^= POLLOUT;
    }
}

std::string Server::get_name_server() const {
    char * ip = inet_ntoa(((struct sockaddr_in *)addr)->sin_addr);

    return std::string (ip)+":"+std::to_string(ntohs(((struct sockaddr_in*)(addr)) ->sin_port));

}

void Server::request_mode_disable() const {
    if (poll!= nullptr){
        if ((poll->events & POLLOUT))
            poll->events ^= POLLOUT;
    }
}

void Server::response_mode_enable() const {
    if (poll!= nullptr){
        if (!(poll->events & POLLIN))
            poll->events ^= POLLIN;
    }
}
void Server::response_mode_disable() const {
    if (poll!= nullptr){
        if ((poll->events & POLLIN))
            poll->events ^= POLLIN;
    }
}

Server::~Server() {
    if(poll!= nullptr)
        free(poll);
}


