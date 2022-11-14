//
// Created by rodion on 05.11.22.
//

#ifndef PROXY_CASHSCHEDULER_H
#define PROXY_CASHSCHEDULER_H

#include "../connection/Response.h"
#include <functional>
#include <string>
#include "../client/Client.h"
#include "../connection/Response.h"
#include "CashRecord.h"
#include <spdlog/spdlog.h>

#define HAVE_RECORD 2
#define NO_RECORD 1

class CashScheduler {
public:
    int cash_event_fd[2]{};
    Response *response_505;
    Response *response_405;
    Response *response_409;
    std::unordered_map<std::string, CashRecord> cash;

    int connect_to_record(Request *request, Response **pResponse);

    void add_record(Request *request, Response *response);

    void disconnect_to_record(Request *request);

    void clean_unnecessary_cash();

    void destroy();

    CashScheduler();

    ~CashScheduler();

};


#endif //PROXY_CASHSCHEDULER_H
