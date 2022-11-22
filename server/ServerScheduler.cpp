//
// Created by rodion on 04.11.22.
//

#include "ServerScheduler.h"
#include <unistd.h>
#include <spdlog/spdlog.h>


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