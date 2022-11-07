//
// Created by rodion on 05.11.22.
//

#include "CashScheduler.h"

CashScheduler::CashScheduler() = default;

void CashScheduler::add_record(Request *request, Response *response) {
    response->type = CASHED_RESPONSE;
    CashRecord cashRecord(response);
    std::string host(request->host, request->host_len);
    std::string resource(request->resource, request->resource_len);
    std::string full = host + resource;
    cash.insert(std::pair(full, cashRecord));
}

int CashScheduler::connect_to_record(Request *request, Response **pResponse) {
    std::string host(request->host, request->host_len);
    std::string resource(request->resource, request->resource_len);
    std::string full = host + resource;
    auto iter_record = cash.find(full);
    if (iter_record!=cash.end()){
        *pResponse = (*iter_record).second.response;
        (*iter_record).second.connect_client();
        return HAVE_RECORD;
    } else{
        return NO_RECORD;
    }
}

void CashScheduler::disconnect_to_record(Request *request) {
    std::string host(request->host, request->host_len);
    std::string resource(request->resource, request->resource_len);
    std::string full = host + resource;
    auto iter_record = cash.find(full);
    if (iter_record!=cash.end()){
        (*iter_record).second.disconnect_client();
    }
}


