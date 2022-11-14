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

    char *url = server->request->host;

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
            server->fd = fd;
            server->addr = result->ai_addr;
            server->poll = (pollfd *) calloc(1, sizeof(pollfd));
            server->poll->fd = fd;
            server->poll->events = POLLHUP | POLLERR;
            return CONNECTION_SERVER_SUCCESS;
        }
        res = res->ai_next;
    }

    return CONNECTION_SERVER_FAILED;
}

int ServerScheduler::send_request(Server *server) {
    Request *request = server->request;
    long write_chars = write(server->poll->fd, (request->request) + (server->current_send_reques),
                             request->req_len - server->current_send_reques);


    if (write_chars < 0) {

        return REQUEST_SEND_WITH_ERROR;
    }
    if (write_chars != request->req_len - server->current_send_reques)
        spdlog::critical("ServerScheduler send_request {} from ", write_chars,
                         request->req_len - server->current_send_reques);
    server->current_send_reques += write_chars;
    if (request->req_len == request->len_buf)
        request->append_buf();

    if (server->current_send_reques == request->req_len)

        return REQUEST_HAS_BEEN_SEND;

    return (int) write_chars;


}


int ServerScheduler::recv_response(Server *server) {
    Response *response = server->response;
    long read_chars = read(server->fd, (response->response) + (response->len_response),
                           response->len_buf - response->len_response);
        if (read_chars < 0) {
        return RESPONSE_RECEIVE_WITH_ERROR;
    }

    if (read_chars == 0) {
        response->status = END;
        return RESPONSE_RECEIVED;
    }

    (response->len_response) += read_chars;
    if (response->len_response == response->len_buf)
        response->append_buf();


    if (response->status == HEADERS) {
        for (long i = 0; i < response->len_response - 1; ++i) {
            if (response->response[i] == '\n' && response->response[i + 1] == '\r') {
                response->status = BODY;
                response->resolve();
                return RESPONSE_RECEIVED_HEADER;
            }
        }
    }

    return (int) read_chars;

}