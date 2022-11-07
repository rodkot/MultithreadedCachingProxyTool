//
// Created by rodion on 03.11.22.
//

#include <iostream>
#include <csignal>
#include <complex.h>
#include "PollScheduler.h"
#include "client/Client.h"
#include "connection/Request.h"
#include "sys/socket.h"
#include "server/ServerScheduler.h"


//
//PollScheduler::PollScheduler(int fd_server) {
//    polls = (pollfd *) calloc(1, sizeof(pollfd));
//    polls[0].fd = fd_server;
//    polls[0].events = POLLIN;
//}
PollScheduler::PollScheduler(ConnectionScheduler &connectionScheduler, ClientScheduler &clientScheduler1,
                             CashScheduler &cashScheduler)
        : connectionScheduler(connectionScheduler), clientScheduler(clientScheduler1), cashScheduler(cashScheduler) {
    polls = (pollfd *) realloc(polls, (1 + 2 * MAX_CLIENT) * sizeof(pollfd));
    p_polls = (pollfd **) realloc(p_polls, (1 + 2 * MAX_CLIENT) * sizeof(pollfd *));
}

int PollScheduler::builder() {
    polls[0] = *connection;
    p_polls[0] = connection;
    for (auto iter_client = clients.begin();
         iter_client != clients.end();) {

        if ((*iter_client)->status == STOP) {
            delete (*iter_client)->server;
            //(*iter_client)->server->status = DELETE;
            if (((*iter_client)->response->type != CASHED_RESPONSE)
                && ((*iter_client)->response->type != DURING_CASH_RESPONSE)) {
                free((*iter_client)->response);
            } else
                cashScheduler.disconnect_to_record((*iter_client)->request);

            delete (*iter_client);
            iter_client = clients.erase(iter_client);
        } else {
            ++iter_client;
        }
    }

    for (int i = 0; i < clients.size(); ++i) {
        (polls[i + 1]) = *(clients[i]->poll);
        (p_polls[i + 1]) = (clients[i]->poll);
    }


    for (auto iter_server = servers.begin();
         iter_server != servers.end();) {

        if ((*iter_server)->status == STOP) {
            close((*iter_server)->fd);
            // delete (*iter_server);
            iter_server = servers.erase(iter_server);
        } else {
            ++iter_server;
        }
    }

    for (int i = 0; i < servers.size(); ++i) {
        (polls[i + clients.size() + 1]) = *(servers[i]->poll);
        (p_polls[i + clients.size() + 1]) = (servers[i]->poll);
    }
    return 0;
};

int PollScheduler::open_connect() {
    int socket = connectionScheduler.listen_socket();
    switch (socket) {
        case OPEN_CONNECTION_FAILED: {
            return OPEN_CONNECTION_FAILED;
        }
        default: {
            connection = (pollfd *) calloc(1, sizeof(pollfd));
            //p_polls = (pollfd **) calloc(1, sizeof(pollfd *));
            connection->fd = socket;
            connection->events = POLLIN;
            //p_polls = &(polls[0]);
            return OPEN_CONNECTION_SUCCESS;
        }
    }

}

int PollScheduler::addClientConnectionHandler() {
    auto *new_client = new Client();
    //auto *addr = (sockaddr *) calloc(1, sizeof(sockaddr));
    //int socket_connect = connectionScheduler.fresh_connection(new_client);
    switch (connectionScheduler.fresh_connection(new_client)) {
        case CLIENT_CONNECTION_SUCCESS:
        {
            new_client->asking_mode_enable();
            spdlog::info("connect success client {}",new_client->get_name_client());
            add_client(new_client);
            return CLIENT_CONNECTION_SUCCESS;
        }
        case CLIENT_CONNECTION_FAILED:
            return CLIENT_CONNECTION_FAILED;
    }
}

int PollScheduler::addServerConnectionHandler(Client *client) {

    /*
     * Проверка корректности запроса
     * Если некорректный, то создание response на основе сущ-ющих ответов ERROR
     * И запуск ответа к клиенту
     */
    switch (ClientScheduler::check_valid_request(client->request)) {
        case NO_SUPPORT_METHOD: {
            std::cout << "NO_SUPPORT_METHOD" << std::endl;
            //http 405
        }
            break;
        case NO_SUPPORT_VERSION_HTTP: {
            std::cout << "NO_SUPPORT_VERSION_HTTP" << std::endl;
            // http 505
        }
            break;
        case ERROR_RESOURCE: {
            std::cout << "ERROR_RESOURCE" << std::endl;
            // http 409
        }
            break;
        case VALID_REQUEST: {
            switch (cashScheduler.connect_to_record(client->request, &(client->response))) {
                case HAVE_RECORD:
                    client->receiving_mode_enable();
                    break;
                case NO_RECORD: {
                    Server *server = new Server(client);
                    client->server = server;
                    switch (ServerScheduler::connect_to_server(server)) {
                        case CONNECTION_SERVER_FAILED: {
                            // http 409
                        }
                            break;
                        case CONNECTION_SERVER_SUCCESS: {
                            server->request_mode_enable();
                            add_server(server);
                        }
                            break;

                    }
                }
                    break;
            }


            std::cout << "VALID_REQUEST" << std::endl;
        }
            break;
    }
    /* Если корректный, то проверка на сущ-ние такой записи в кеш
     * Если есть, то создание response на основе сущ-ющих кеш
     * И запуск ответа к клиенту
     */

    /* Если нет, то начинается поиск ip домена
    * Если нет такого домена, то создание response на основе сущ-ющих ответов ERROR
    * И запуск ответа к клиенту
    */

    /* Если есть, то создание сокета
     * Создание Response
     * И запуск отправки запроса
     */

}


int PollScheduler::check_valid_request(Request req) {
    return -1;
}


int PollScheduler::requestClientHandler(Client *client) {
    int res = ClientScheduler::forming_request(client);
    if (res == 0) {
        client->asking_mode_disable();
        client->request->resolve();
        addServerConnectionHandler(client);
    }
    return res;
}

int PollScheduler::requestServerHandler(Server *server) {
    switch (ServerScheduler::send_request(server)) {
        case REQUEST_HAS_BEEN_SEND: {
            server->request_mode_disable();
            auto *response = new Response();
            server->response = response;
            server->client->response = response;
            server->response_mode_enable();

        }
            break;
        case REQUEST_SEND_WITH_ERROR: {

        }
            break;
    }
}

int PollScheduler::responseServerHandler(Server *server) {
    switch (ServerScheduler::recv_response(server)) {
        case RESPONSE_RECEIVED: {

            switch (server->response->type) {
                case DURING_CASH_RESPONSE: {
                    cashScheduler.add_record(server->request, server->response);
                }
                case NO_CASH_RESPONSE: {
                    //cashScheduler.add_record(server->request,server->response);

                }
                case CASHED_RESPONSE: {

                }
            }
            server->status = STOP;
            server->response_mode_disable();
            return RESPONSE_RECEIVED;
        }

        case RESPONSE_RECEIVED_HEADER: {
            switch (server->response->code) {
                case HTTP_CODE_OK:
                    server->response->type = DURING_CASH_RESPONSE;
                    cashScheduler.add_record(server->request, server->response);
                    server->client->receiving_mode_enable();
                    return RESPONSE_RECEIVE_PROCESS;
                case HTTP_CODE_NO_OK:
                    server->response->type = NO_CASH_RESPONSE;
                    server->client->receiving_mode_enable();
                    return RESPONSE_RECEIVE_PROCESS;
            }
        }
            break;
        case RESPONSE_RECEIVE_WITH_ERROR: {
            server->status = STOP;
            server->response_mode_disable();
            return RESPONSE_RECEIVE_WITH_ERROR;
        }

    }
}

int PollScheduler::responseClientHandler(Client *client) {
    switch (ClientScheduler::send_response(client)) {
        case RESPONSE_RECEIVED: {
            client->status = STOP;
            client->receiving_mode_disable();
            return RESPONSE_RECEIVED;
        }
        case RESPONSE_RECEIVE_WITH_ERROR: {
            client->status = STOP;
            client->receiving_mode_disable();
            return RESPONSE_RECEIVE_WITH_ERROR;
        }

    }

}

int PollScheduler::add_client(Client* client) {
    clients.push_back(client);
    return 0;
}

int PollScheduler::add_server(Server *server) {
    servers.push_back(server);
}

int PollScheduler::executor() {
    //builder();
    int res = poll(polls, getCountHandler(), TIME_OUT);
    switch (res) {
        case 0: {
            return 0;
        }
        case -1: {
            return -1;
        }
        default: {
            for (int i = 0; i < getCountHandler(); ++i) {
                *(p_polls[i]) = polls[i];
            }
            if (polls[0].revents == POLLIN) {
                switch (addClientConnectionHandler()) {
                    case CONNECTION_SERVER_SUCCESS:
                        break;
                    case CONNECTION_SERVER_FAILED:
                        break;

                }
            }

            for (int i = 0; i < clients.size(); ++i) {
                if (clients[i]->poll->revents & POLLIN) {
                    requestClientHandler(clients[i]);
                }
                if (clients[i]->poll->revents & POLLOUT) {
                    responseClientHandler(clients[i]);
                }
            }
            for (int i = 0; i < servers.size(); ++i) {
                if ((servers[i]->poll->revents & POLLOUT)) {
                    requestServerHandler(servers[i]);
                }
                if ((servers[i]->poll->revents) & POLLIN) {
                    responseServerHandler(servers[i]);
                }
                if ((servers[i]->poll->revents) & POLLHUP) {
                    std::cout << "Плохо дело ";
                }
            }

        }
    }
    return 0;
};
