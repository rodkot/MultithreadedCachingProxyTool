//
// Created by rodion on 04.11.22.
//

#include "ServerScheduler.h"
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <spdlog/spdlog.h>


int ServerScheduler::connect_to_server(Server *server) {
    struct addrinfo hints, *res, *result;

    char *url = server->getRequest()->getHost();

    int port = 80;
    char host_name[100];
    if (sscanf(url, "%99[^:]:%i[^\r]", host_name, &port) != 2) {
        sscanf(url, "%99[^\r]", host_name);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    int errcode = getaddrinfo(host_name, std::to_string(port).c_str(), &hints, &result);
    if (errcode != 0) {
        return CONNECTION_SERVER_FAILED;
    }

    res = result;
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    while (res != nullptr) {
        if (connect(fd, result->ai_addr, result->ai_addrlen) == 0) {
            server->setFd(fd);
            server->setAddr(result->ai_addr);
            server->setPoll((pollfd *) calloc(1, sizeof(pollfd)));
            server->getPoll()->fd = fd;
            server->getPoll()->events = POLLHUP | POLLERR;
            return CONNECTION_SERVER_SUCCESS;
        }
        res = res->ai_next;
    }

    return CONNECTION_SERVER_FAILED;
}

int ServerScheduler::send_request(Server *server) {
    Request *request = server->getRequest();
    long write_chars = write(server->getFd(), (request->getRequest()) + (server->getCurrentSendReques()),
                             request->getReqLen() - server->getCurrentSendReques());


    if (write_chars < 0) {

        return REQUEST_SEND_WITH_ERROR;
    }
    if (write_chars != request->getReqLen() - server->getCurrentSendReques())
        spdlog::warn("ServerScheduler send_request {} from {} ", write_chars,
                     (request->getReqLen() - server->getCurrentSendReques()));
    server->setCurrentSendReques(server->getCurrentSendReques() + write_chars);
    if (request->getReqLen() == request->getLenBuf())
        request->append_buf();

    if (server->getCurrentSendReques() == request->getReqLen())

        return REQUEST_HAS_BEEN_SEND;

    return (int) write_chars;


}


int ServerScheduler::recv_response(Server *server) {
    Response *response = server->getResponse();
    long read_chars = read(server->getFd(), (response->getResponse()) + (response->getLenResponse()),
                           response->getLenBuf() - response->getLenResponse());
    if (read_chars < 0) {
        return RESPONSE_RECEIVE_WITH_ERROR;
    }

    if (read_chars == 0) {
        response->setStatus(END);
        return RESPONSE_RECEIVED;
    }

    response->setLenResponse(response->getLenResponse() + read_chars);
    if (response->getLenResponse() == response->getLenBuf())
        response->append_buf();


    if (response->getStatus() == HEADERS) {
        for (long i = 0; i < response->getLenResponse() - 1; ++i) {
            if (response->getResponse()[i] == '\n' && response->getResponse()[i + 1] == '\r') {
                response->setStatus(BODY);
                response->resolve();
                return RESPONSE_RECEIVED_HEADER;
            }
        }
    }

    return (int) read_chars;

}