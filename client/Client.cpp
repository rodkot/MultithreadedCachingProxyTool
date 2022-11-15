//
// Created by rodion on 03.11.22.
//

#include "Client.h"
#include <malloc.h>
#include <netinet/in.h>
#include <arpa/inet.h>

Client::Client() {
    request = new Request();
}

std::string Client::get_name_client() const {
    char *ip = inet_ntoa(((struct sockaddr_in *) addr)->sin_addr);

    return std::string(ip) + ":" + std::to_string(ntohs(((struct sockaddr_in *) (addr))->sin_port));
}

void Client::asking_mode_enable() const {
    if (!(poll->events & POLLIN))
        poll->events ^= POLLIN;
}

void Client::asking_mode_disable() const {
    if ((poll->events & POLLIN))
        poll->events ^= POLLIN;
}

void Client::receiving_mode_enable() const {
    if (!(poll->events & POLLOUT))
        poll->events ^= POLLOUT;
}

void Client::receiving_mode_disable() const {
    if ((poll->events & POLLOUT))
        poll->events ^= POLLOUT;
}

Client::~Client() {
    if (addr != nullptr)
        free(addr);
    if (poll != nullptr)
        free(poll);
    if (request != nullptr) {
        delete (request);
    }

}

int Client::getFd() const {
    return fd;
}

void Client::setFd(int fd) {
    Client::fd = fd;
}

int Client::getStatus() const {
    return status;
}

void Client::setStatus(int status) {
    Client::status = status;
}

sockaddr *Client::getAddr() const {
    return addr;
}

void Client::setAddr(sockaddr *addr) {
    Client::addr = addr;
}

Server *Client::getServer() const {
    return server;
}

void Client::setServer(Server *server) {
    Client::server = server;
}

Request *Client::getRequest() const {
    return request;
}

void Client::setRequest(Request *request) {
    Client::request = request;
}

Response *Client::getResponse() const {
    return response;
}

void Client::setResponse(Response *response) {
    Client::response = response;
}

pollfd *Client::getPoll() const {
    return poll;
}

void Client::setPoll(pollfd *poll) {
    Client::poll = poll;
}

long Client::getCurrentRecvResponse() const {
    return current_recv_response;
}

void Client::setCurrentRecvResponse(long currentRecvResponse) {
    current_recv_response = currentRecvResponse;
}
