//
// Created by rodion on 03.11.22.
//
#include "ClientScheduler.h"
#include <unistd.h>
#include <cstring>

ClientScheduler::ClientScheduler() = default;

int ClientScheduler::forming_request(Client *client) {
     Request * request = client->getRequest();
    long read_chars = (read(client->getPoll()->fd, (request->getRequest()) + (request->getReqLen()),
                                         request->getLenBuf() - request->getReqLen()));
    if (read_chars < 0) {
        return REQUEST_RECV_FAILED;
    }
    request->setReqLen(request->getReqLen()+read_chars);

    if (request->getReqLen() == request->getLenBuf())
        request->append_buf();

    //проверка что запрос закончился, возможна оптимизация
    for (long i = 0; i < request->getReqLen() - 1; ++i) {
        if (request->getRequest()[i] == '\n' && request->getRequest()[i + 1] == '\r')
            return REQUEST_RECV_SUCCESS;
    }
    if (read_chars==0)
        return REQUEST_RECV_FAILED;

    return REQUEST_RECV_PROCESS;
}

int ClientScheduler::check_valid_request(Request *request) {
    if (request->getMethodLen() !=3 || strncmp(request->getMethod(),"GET",3) != 0){
        return NO_SUPPORT_METHOD;
    } else if (request->getResourceLen()==0){
        return ERROR_RESOURCE;
    } else if (request->getVersionHttpLen() !=8 || strncmp(request->getVersionHttp(),"HTTP/1.0",8)!=0) {
        return NO_SUPPORT_VERSION_HTTP;
    } else {
        return VALID_REQUEST;
    }
}

int ClientScheduler::send_response(Client *client) {
    Response* response = client->getResponse();
    Server* server = client->getServer();
    switch (response->getType()) {
        case DURING_CASH_RESPONSE:
        case NO_CASH_RESPONSE:
        case END:
        case CASHED_RESPONSE:
        {
            long write_chars = write(client->getPoll()->fd, (response->getResponse()) + (client->getCurrentRecvResponse()),
                                                    response->getLenResponse() - client->getCurrentRecvResponse());
            client->setCurrentRecvResponse(client->getCurrentRecvResponse()+write_chars);

            if (write_chars<0){
                return RESPONSE_RECEIVE_WITH_ERROR;
            }
            if (response->getLenResponse()
                == client->getCurrentRecvResponse()){
                if (response->getStatus() == END){
                    return RESPONSE_RECEIVED;
                }

            }
            if( write_chars==0){
                return RESPONSE_PAUSE_RECEIVE;
            }
            return RESPONSE_PROCESS_RECEIVE;
        }
    }
}