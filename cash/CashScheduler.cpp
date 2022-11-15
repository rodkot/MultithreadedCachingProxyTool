//
// Created by rodion on 05.11.22.
//

#include <cstring>
#include "CashScheduler.h"


CashScheduler::CashScheduler() {
    pipe(cash_event_fd);

    std::string res_405 = "HTTP/1.0 405 Method Not Allowed\r\n\r\n";
    char *c_res_405 = new char[res_405.length() + 1];
    strcpy(c_res_405, res_405.c_str());
    response_405 = new Response(c_res_405, res_405.length(), END, CASHED_RESPONSE);

    std::string res_505 = "HTTP/1.0 505 HTTP Version Not Supported\r\n\r\n";
    char *c_res_505 = new char[res_505.length() + 1];
    strcpy(c_res_505, res_505.c_str());
    response_505 = new Response(c_res_505, res_505.length(), END, CASHED_RESPONSE);

    std::string res_409 = "HTTP/1.0 409 Conflict\r\n\r\n";
    char *c_res_409 = new char[res_409.length() + 1];
    strcpy(c_res_409, res_409.c_str());
    response_409 = new Response(c_res_409, res_409.length(), END, CASHED_RESPONSE);
};

void CashScheduler::clean_unnecessary_cash() {
    for (auto iter_cash = cash.begin();
         iter_cash != cash.end();) {
        CashRecord record = (*iter_cash).second;
        std::string resource = (*iter_cash).first;
        if ((record.count_active == 0 && record.count == 0)) {
            spdlog::warn("CashScheduler clean_unnecessary_cash remove cash {}", resource);
            delete record.response;
            iter_cash = cash.erase(iter_cash);
        } else if (record.response->getStatus() == FAIL) {
            spdlog::error("CashScheduler clean_unnecessary_cash remove FAIL cash {}", resource);
            delete record.response;
            iter_cash = cash.erase(iter_cash);
        } else {
            record.clean_count();
            iter_cash = cash.insert_or_assign(iter_cash, resource, record);
            ++iter_cash;
        }

        //  cash["dd"]=(std::pair(resource, record));
    }
}

void CashScheduler::add_record(Request *request, Response *response) {
    response->setType(CASHED_RESPONSE);
    CashRecord cashRecord(response);
    std::string host(request->getHost(), request->getHostLen());
    std::string resource(request->getResource(), request->getResourceLen());
    std::string full = host + resource;
    cashRecord.connect_client();
    spdlog::info("CashScheduler CASHED_RESPONSE {}", full);
    cash.insert(std::pair(full, cashRecord));
}

int CashScheduler::connect_to_record(Request *request,Client* client) {
    std::string host(request->getHost(), request->getHostLen());
    std::string resource(request->getResource(), request->getResourceLen());
    std::string full = host + resource;
    auto iter_record = cash.find(full);
    if (iter_record != cash.end() && iter_record->second.response->getStatus() != FAIL) {
        client->setResponse((*iter_record).second.response);
        (*iter_record).second.connect_client();
        return HAVE_RECORD;
    } else {
        return NO_RECORD;
    }
}

void CashScheduler::disconnect_to_record(Request *request) {
    std::string host(request->getHost(), request->getHostLen());
    std::string resource(request->getResource(), request->getResourceLen());
    std::string full = host + resource;
    auto iter_record = cash.find(full);
    if (iter_record != cash.end()) {
        (*iter_record).second.disconnect_client();
    }
}

void CashScheduler::destroy() {
    for (auto iter_cash = cash.begin();
         iter_cash != cash.end();) {
        CashRecord record = (*iter_cash).second;
        std::string resource = (*iter_cash).first;
        spdlog::warn("CashScheduler clean_unnecessary_cash remove cash {}", resource);
        delete record.response;
        iter_cash = cash.erase(iter_cash);
    }
}

CashScheduler::~CashScheduler() {
    delete response_405;
    delete response_409;
    delete response_505;
}

