//
// Created by rodion on 03.11.22.
//

#include <malloc.h>
#include <unistd.h>
#include "Server.h"
#include "../client/Client.h"

//Server::Server(Request *req):request(req) {}
Server::Server(Client *c):request(c->request),client(c){}
void Server::request_mode_enable() const {
    if (poll!= nullptr){
        if (!(poll->events & POLLOUT))
            poll->events ^= POLLOUT;
    }
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
    //close(fd);
    //if(poll!= nullptr)
     //   free(poll);
}


