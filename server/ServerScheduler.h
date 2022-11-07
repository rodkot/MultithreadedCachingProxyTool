//
// Created by rodion on 04.11.22.
//

#ifndef PROXY_SERVERSCHEDULER_H
#define PROXY_SERVERSCHEDULER_H
#include "Server.h"

#define CONNECTION_SERVER_FAILED -1
#define CONNECTION_SERVER_SUCCESS 1

#define REQUEST_HAS_BEEN_SEND 0
#define REQUEST_SEND_WITH_ERROR -1

#define RESPONSE_RECEIVED 0
#define RESPONSE_RECEIVE_PROCESS 11
#define RESPONSE_RECEIVE_WITH_ERROR -1
#define RESPONSE_RECEIVED_HEADER 1

class ServerScheduler {
public:

    static int connect_to_server(Server* server);
    static int send_request(Server* server);
    static int recv_response(Server* server);




};


#endif //PROXY_SERVERSCHEDULER_H
