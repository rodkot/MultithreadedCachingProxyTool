//
// Created by rodion on 03.11.22.
//

#include "Client.h"
#include <malloc.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

Client::Client() {
//    poll = (pollfd *) calloc(1, sizeof(pollfd));
//    poll->fd = fd;
    //len_buf = BUF_STEP_SIZE;
    request = new Request();
}

std::string Client::get_name_client() {

    struct sockaddr_in *sock = ( struct sockaddr_in*)&addr;
    int port = ntohs(sock->sin_port);


    struct in_addr in  = sock->sin_addr;
    char address[INET_ADDRSTRLEN]; //INET_ADDRSTRLEN this macro system default definition 16
    //If successful, the IP address is stored in the str string.
    inet_ntop(AF_INET,&in, address, sizeof(address));
    return std::string (address)+":"+std::to_string(port);
   // printf("ip:port  %s : %d",str,port);

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
    close(fd);
    if (addr != nullptr)
        free(addr);
    if (poll!= nullptr)
        free(poll);
    if (request!= nullptr)
        free(request);
}