//
// Created by rodion on 03.11.22.
//

#ifndef PROXY_POLLSCHEDULER_H
#define PROXY_POLLSCHEDULER_H

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

#define TIME_OUT -1


class PollScheduler {
private:
    ConnectionScheduler &connectionScheduler;
    ClientScheduler &clientScheduler;
    CashScheduler &cashScheduler;

    std::vector<Client *> clients = {};
    std::vector<Server *> servers = {};
    pollfd *polls = nullptr;
    pollfd *connection = nullptr;
    pollfd **p_polls{};

public:
    PollScheduler(ConnectionScheduler &, ClientScheduler &, CashScheduler &);

    int builder();

    int executor();

    int open_connect();

    int add_client(Client* client);

    int add_server(Server *server);

    int check_valid_request(Request req);


    int addClientConnectionHandler();

    int addServerConnectionHandler(Client *client);

    int requestClientHandler(Client *client);

    int requestServerHandler(Server *server);

    int responseServerHandler(Server *server);

    static int responseClientHandler(Client *client);

    int getCountHandler() {
        return 1 + clients.size() + servers.size();
    }

};


#endif //PROXY_POLLSCHEDULER_H
