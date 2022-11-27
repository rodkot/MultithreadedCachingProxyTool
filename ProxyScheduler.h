//
// Created by rodion on 03.11.22.
//

#ifndef PROXY_PROXYSCHEDULER_H
#define PROXY_PROXYSCHEDULER_H

#include <poll.h>
#include <malloc.h>
#include "client/Client.h"
#include "connection/ConnectionScheduler.h"
#include "client/ClientScheduler.h"
#include <utility>
#include <vector>
#include "connection/Request.h"
#include "server/Server.h"
#include "cash/CashScheduler.h"
#include <spdlog/spdlog.h>
#include <signal.h>
#include "Configuration.h"

#define STOP_PROXY -1
#define ACTIVE_PROXY 1

class ProxyScheduler {
private:
    ConnectionScheduler &connectionScheduler;
    ClientScheduler &clientScheduler;
    CashScheduler &cashScheduler;

    Client *client = nullptr;
    Server *server = nullptr;
    Response* response = nullptr;
    Request* request = nullptr;

    pollfd *polls = nullptr;
    // pollfd *connection = nullptr;
    // pollfd *cash = nullptr;
    pollfd **p_poll = nullptr;

public:
    ProxyScheduler(ConnectionScheduler &, ClientScheduler &, CashScheduler &, Client *);

    int builder();

    int executor();

    // int open_connect();

    //void add_client(Client *client);

    //void add_server(Server *server);

    //int addClientConnectionHandler();

    int addServerConnectionHandler();

    int requestClientHandler();

    int requestServerHandler();

    int responseServerHandler();

    int responseClientHandler();

    //   void eventCashHandler();

    void destroy();

    int getCountHandler() {
        int l = 0;
        if (client != nullptr) {
            l++;
        }
        if (server != nullptr) {
            l++;
        }
        return l;
    }

    ~ProxyScheduler();
};


#endif //PROXY_PROXYSCHEDULER_H
