//
// Created by rodion on 03.11.22.
//

#ifndef PROXY_CLIENTSCHEDULER_H
#define PROXY_CLIENTSCHEDULER_H
#include "Client.h"

#define NO_SUPPORT_METHOD -2
#define NO_SUPPORT_VERSION_HTTP -1
#define ERROR_RESOURCE -3
#define VALID_REQUEST 0

#define RESPONSE_RECEIVED 0
#define RESPONSE_PROCESS_RECEIVE 2
#define RESPONSE_RECEIVE_WITH_ERROR -1

class ClientScheduler {
public:
    ClientScheduler();
    /*
     * Формирование запроса клиента
     * 1 - формирование не закончено
     * 0 - формирование успешное
     *  -1 - ошибка во время формирования
     */
    static int forming_request(Client*);

    static int check_valid_request(Request * request);

    static int send_response(Client* client);
};


#endif //PROXY_CLIENTSCHEDULER_H
