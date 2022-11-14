//
// Created by rodion on 03.11.22.
//

#ifndef PROXY_CLIENTSCHEDULER_H
#define PROXY_CLIENTSCHEDULER_H

#include "Client.h"
#include <spdlog/spdlog.h>

#define NO_SUPPORT_METHOD -2
#define NO_SUPPORT_VERSION_HTTP -1
#define ERROR_RESOURCE -3
#define VALID_REQUEST 0

#define REQUEST_RECV_FAILED -1
#define REQUEST_RECV_PROCESS 2
#define REQUEST_RECV_SUCCESS 1

#define RESPONSE_RECEIVED 0
#define RESPONSE_PROCESS_RECEIVE 2
#define RESPONSE_RECEIVE_WITH_ERROR -1
#define RESPONSE_PAUSE_RECEIVE 3

class ClientScheduler {
public:
    ClientScheduler();

    static int forming_request(Client *);

    static int check_valid_request(Request *request);

    static int send_response(Client *client);
};


#endif //PROXY_CLIENTSCHEDULER_H
