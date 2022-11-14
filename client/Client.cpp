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