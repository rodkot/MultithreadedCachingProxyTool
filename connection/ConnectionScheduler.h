//
// Created by rodion on 03.11.22.
//

#ifndef PROXY_CONNECTIONSCHEDULER_H
#define PROXY_CONNECTIONSCHEDULER_H
#include <sys/socket.h>
#include "../Configuration.h"
#include "../client/Client.h"

#define OPEN_CONNECTION_FAILED -1
#define OPEN_CONNECTION_SUCCESS 1

#define CLIENT_CONNECTION_FAILED -1
#define CLIENT_CONNECTION_SUCCESS 1

class ConnectionScheduler {
private:
    int fd_connect{};
    char * address;
    int port;
public:

ConnectionScheduler(char* address,int port);

/*
 * Создает сокет, на котором мы слушаем новые соединение с сервером
 */
int listen_socket();

/*
 * Делает accept прослушивающего сокета и возвращает сокет для общение с клиентом;
 */
int fresh_connection(Client* client) const;


};


#endif //PROXY_CONNECTIONSCHEDULER_H
