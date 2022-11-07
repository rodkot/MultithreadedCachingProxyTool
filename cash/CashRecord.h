//
// Created by rodion on 06.11.22.
//

#ifndef PROXY_CASHRECORD_H
#define PROXY_CASHRECORD_H
#include "../connection/Response.h"


class CashRecord {
public:
    Response* response;
    int count_active;
    int count;
    explicit CashRecord(Response*);
    void connect_client();
    void disconnect_client();
};


#endif //PROXY_CASHRECORD_H
