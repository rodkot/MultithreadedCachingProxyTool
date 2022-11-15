//
// Created by rodion on 03.11.22.
//

#include "Server.h"
#include "../client/Client.h"
#include <netinet/in.h>
#include <arpa/inet.h>

Server::Server(Client *c):request(c->getRequest()),client(c){}
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

int Server::getFd() const {
    return fd;
}

void Server::setFd(int fd) {
    Server::fd = fd;
}

int Server::getStatus() const {
    return status;
}

void Server::setStatus(int status) {
    Server::status = status;
}

pollfd *Server::getPoll() const {
    return poll;
}

void Server::setPoll(pollfd *poll) {
    Server::poll = poll;
}

Request *Server::getRequest() const {
    return request;
}

void Server::setRequest(Request *request) {
    Server::request = request;
}

Response *Server::getResponse() const {
    return response;
}

void Server::setResponse(Response *response) {
    Server::response = response;
}

Client *Server::getClient() const {
    return client;
}

void Server::setClient(Client *client) {
    Server::client = client;
}

sockaddr *Server::getAddr() const {
    return addr;
}

void Server::setAddr(sockaddr *addr) {
    Server::addr = addr;
}

int Server::getCurrentSendReques() const {
    return current_send_reques;
}

void Server::setCurrentSendReques(int currentSendReques) {
    current_send_reques = currentSendReques;
}


