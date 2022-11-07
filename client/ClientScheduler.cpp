//
// Created by rodion on 03.11.22.
//
#include "ClientScheduler.h"
#include <iostream>
#include <unistd.h>
#include <cstring>




ClientScheduler::ClientScheduler() = default;

int ClientScheduler::forming_request(Client *client) {
     Request * request = client->request;
    long read_chars = (read_chars = read(client->poll->fd, (request->request) + (request->req_len),
                                         request->len_buf - request->req_len));
    if (read_chars < 0) {
        return (int)read_chars;
    }
//request->add(read_chars);
    (request->req_len) += read_chars;
    if (request->req_len == request->len_buf)
        request->append_buf();
    //проверка что запрос закончился, возможна оптимизация
    for (long i = 0; i < request->req_len - 1; ++i) {
        if (request->request[i] == '\n' && request->request[i + 1] == '\r')
            return 0;
    }

    return (int)read_chars;
}

int ClientScheduler::check_valid_request(Request *request) {
    if (request->method_len !=3 || strncmp(request->method,"GET",3) != 0){
        return NO_SUPPORT_METHOD;
    } else if (request->resource_len==0){
        return ERROR_RESOURCE;
    } else if (request->version_http_len !=8 || strncmp(request->version_http,"HTTP/1.0",8)!=0) {
        return NO_SUPPORT_VERSION_HTTP;
    } else {
        return VALID_REQUEST;
    }
}

int ClientScheduler::send_response(Client *client) {
    Response* response = client->response;
    Server* server = client->server;
    switch (response->type) {
        case DURING_CASH_RESPONSE:
        case NO_CASH_RESPONSE:
        case CASHED_RESPONSE:
        {
            long write_chars = write(client->poll->fd, (response->response) + (client->current_recv_response),
                                                    response->len_response - client->current_recv_response);
            client->current_recv_response+=write_chars;

            if (write_chars<0){
                return RESPONSE_RECEIVE_WITH_ERROR;
            }
            if ((response->len_response
            == client->current_recv_response)
            && response->status == END){
                return RESPONSE_RECEIVED;
            }
            return RESPONSE_PROCESS_RECEIVE;
        }
    }
}