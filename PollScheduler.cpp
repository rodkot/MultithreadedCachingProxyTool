//
// Created by rodion on 03.11.22.
//

#include <iostream>
#include <csignal>
#include "PollScheduler.h"
#include "client/Client.h"
#include "server/ServerScheduler.h"


PollScheduler::PollScheduler(ConnectionScheduler &connectionScheduler, ClientScheduler &clientScheduler1,
                             CashScheduler &cashScheduler)
        : connectionScheduler(connectionScheduler), clientScheduler(clientScheduler1), cashScheduler(cashScheduler) {
    polls = (pollfd *) realloc(polls, (2 + 2 * MAX_CLIENT) * sizeof(pollfd));
    p_polls = (pollfd **) realloc(p_polls, (2 + 2 * MAX_CLIENT) * sizeof(pollfd *));

}

int PollScheduler::builder() {
    polls[0] = *connection;
    p_polls[0] = connection;

    polls[1] = *cash;
    p_polls[1] = cash;

    for (auto iter_client = clients.begin();
         iter_client != clients.end();) {
        Client *client = *iter_client;

        if (client->getStatus() == PAUSE_CLIENT) {
            if (client->getResponse()->getLenResponse() - client->getCurrentRecvResponse() > 0 ||
                client->getResponse()->getStatus() == END) {
                client->setStatus(ACTIVE_CLIENT);
                client->receiving_mode_enable();
                spdlog::warn("responseClientHandler ENABLE client {}", client->get_name_client());
            }
            ++iter_client;
            continue;
        }
        if (client->getStatus() != ACTIVE_CLIENT) {
            close(client->getFd());

            if (client->getResponse() != nullptr) {
                if ((client->getResponse()->getType() == CASHED_RESPONSE)
                    || (client->getResponse()->getType() == DURING_CASH_RESPONSE)) {
                    cashScheduler.disconnect_to_record((*iter_client)->getRequest());
                }
                if (client->getResponse()->getType() == NO_CASH_RESPONSE) {
                    delete client->getResponse();
                }
            }

            if (client->getStatus() == STOP_SUCCESS_CLIENT) {
                spdlog::warn("PollScheduler STOP_SUCCESS_CLIENT disconnect client {}",
                             (*iter_client)->get_name_client());
                delete client;
                iter_client = clients.erase(iter_client);

            } else if (client->getStatus() == STOP_FAILED_CLIENT) {
                spdlog::error("PollScheduler STOP_FAILED_CLIENT disconnect client {}",
                              (*iter_client)->get_name_client());
                delete client;
                iter_client = clients.erase(iter_client);

            }
        } else {
            ++iter_client;
        }


    }

    for (int i = 0; i < clients.size(); ++i) {
        Client *client = clients[i];
        (polls[i + 2]) = *(client->getPoll());
        (p_polls[i + 2]) = (client->getPoll());
    }

    for (auto iter_server = servers.begin();
         iter_server != servers.end();) {
        Server *server = *iter_server;
        int status = server->getStatus();
        if (status != ACTIVE_SERVER) {
            close(server->getFd());
            Response *response = server->getResponse();
            if (response->getType() == DURING_CASH_RESPONSE
                || response->getType() == CASHED_RESPONSE) {
                cashScheduler.disconnect_to_record(server->getRequest());
            }
            if (status == STOP_FAILED_SERVER) {
                spdlog::error("PollScheduler STOP_FAILED_SERVER disconnect server {}",
                              server->get_name_server());
                response->setType(FAIL);
                delete server;
                iter_server = servers.erase(iter_server);
            } else if (status == STOP_SUCCESS_SERVER) {
                spdlog::warn("PollScheduler STOP_SUCCESS_SERVER disconnect server {}",
                             server->get_name_server());
                delete server;
                iter_server = servers.erase(iter_server);
            }
        } else {
            ++iter_server;
        }
    }

    for (int i = 0; i < servers.size(); ++i) {
        (polls[i + clients.size() + 2]) = *(servers[i]->getPoll());
        (p_polls[i + clients.size() + 2]) = (servers[i]->getPoll());
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
            connection->fd = socket;
            connection->events = POLLIN;

            cash = (pollfd *) calloc(1, sizeof(pollfd));
            cash->fd = cashScheduler.cash_event_fd[0];
            cash->events = POLLIN;

            return OPEN_CONNECTION_SUCCESS;
        }
    }

}

int PollScheduler::addClientConnectionHandler() {
    auto *new_client = new Client();

    switch (connectionScheduler.fresh_connection(new_client)) {
        case CLIENT_CONNECTION_SUCCESS: {
            new_client->asking_mode_enable();
            spdlog::info("connect success #{} client {}", clients.size() + 1, new_client->get_name_client());
            add_client(new_client);
            return CLIENT_CONNECTION_SUCCESS;
        }
        case CLIENT_CONNECTION_FAILED:
            spdlog::error("connect error client {}", new_client->get_name_client());
            return CLIENT_CONNECTION_FAILED;
    }
}

int PollScheduler::addServerConnectionHandler(Client *client) {

    switch (ClientScheduler::check_valid_request(client->getRequest())) {
        case NO_SUPPORT_METHOD: {
            spdlog::error("addServerConnectionHandler NO_SUPPORT_METHOD request client {}", client->get_name_client());
            client->setResponse(cashScheduler.response_405);
            client->receiving_mode_enable();
            //http 405
            return NO_SUPPORT_METHOD;
        }
        case NO_SUPPORT_VERSION_HTTP: {
            spdlog::error("addServerConnectionHandler NO_SUPPORT_VERSION_HTTP request client {}",
                          client->get_name_client());
            client->setResponse(cashScheduler.response_505);
            client->receiving_mode_enable();
            // http 505
            return NO_SUPPORT_VERSION_HTTP;
        }
        case ERROR_RESOURCE: {
            spdlog::error("addServerConnectionHandler ERROR_RESOURCE request client {}", client->get_name_client());
            client->setResponse(cashScheduler.response_409);
            client->receiving_mode_enable();
            // http 409
            return ERROR_RESOURCE;
        }
        case VALID_REQUEST: {
            spdlog::info("addServerConnectionHandler VALID_REQUEST request client {}", client->get_name_client());
            switch (cashScheduler.connect_to_record(client->getRequest(), client)) {
                case HAVE_RECORD:
                    spdlog::info("addServerConnectionHandler HAVE_RECORD cache for request client {}",
                                 client->get_name_client());
                    client->receiving_mode_enable();
                    return HAVE_RECORD;
                case NO_RECORD: {
                    spdlog::info("addServerConnectionHandler NO_RECORD cache for request client {}",
                                 client->get_name_client());
                    auto *server = new Server(client);
                    client->setServer(server);
                    switch (ServerScheduler::connect_to_server(server)) {
                        case CONNECTION_SERVER_FAILED: {
                            spdlog::error("addServerConnectionHandler CONNECTION_SERVER_FAILED for request client {}",
                                          client->get_name_client());
                            client->setResponse(cashScheduler.response_409);
                            client->receiving_mode_enable();
                            return CONNECTION_SERVER_FAILED;
                        }
                        case CONNECTION_SERVER_SUCCESS: {
                            spdlog::info("addServerConnectionHandler CONNECTION_SERVER_SUCCESS for request client {}",
                                         client->get_name_client());
                            server->request_mode_enable();
                            add_server(server);
                            return CONNECTION_SERVER_SUCCESS;
                        }
                    }
                }
            }
        }
    }
}

int PollScheduler::requestClientHandler(Client *client) {
    switch (ClientScheduler::forming_request(client)) {
        case REQUEST_RECV_SUCCESS:
            spdlog::info("requestClientHandler REQUEST_RECV_SUCCESS client {}", client->get_name_client());
            client->asking_mode_disable();
            client->getRequest()->resolve();
            addServerConnectionHandler(client);
            return REQUEST_RECV_SUCCESS;
        case REQUEST_RECV_PROCESS:
            spdlog::info("requestClientHandler REQUEST_RECV_PROCESS client {}", client->get_name_client());
            return REQUEST_RECV_PROCESS;
        case REQUEST_RECV_FAILED:
            client->asking_mode_disable();
            client->setStatus(STOP_FAILED_CLIENT);
            spdlog::error("requestClientHandler REQUEST_RECV_FAILED client {}", client->get_name_client());
            return REQUEST_RECV_FAILED;

    }
}

int PollScheduler::requestServerHandler(Server *server) {
    switch (ServerScheduler::send_request(server)) {
        case REQUEST_HAS_BEEN_SEND: {
            spdlog::info("requestServerHandler REQUEST_HAS_BEEN_SEND server {}", server->get_name_server());
            server->request_mode_disable();
            auto *response = new Response();
            server->setResponse(response);
            server->getClient()->setResponse(response);
            server->response_mode_enable();
            return REQUEST_HAS_BEEN_SEND;
        }
        case REQUEST_SEND_WITH_ERROR:
            spdlog::error("requestServerHandler REQUEST_SEND_WITH_ERROR server {}", server->get_name_server());
            server->request_mode_disable();
            server->setStatus(STOP_FAILED_SERVER);
            return REQUEST_SEND_WITH_ERROR;
    }
}

void PollScheduler::eventCashHandler() {
    char buf[10];
    read(cashScheduler.cash_event_fd[0], buf, 10);
    cashScheduler.clean_unnecessary_cash();

    spdlog::info("PollScheduler eventCashHandler");
}

int PollScheduler::responseServerHandler(Server *server) {
    switch (ServerScheduler::recv_response(server)) {
        case RESPONSE_RECEIVED: {
            spdlog::info("responseServerHandler RESPONSE_RECEIVED server {}", server->get_name_server());
            switch (server->getResponse()->getType()) {
                case DURING_CASH_RESPONSE: {
                    cashScheduler.add_record(server->getRequest(), server->getResponse());
                }
            }
            server->setStatus(STOP_SUCCESS_SERVER);
            server->response_mode_disable();
            return RESPONSE_RECEIVED;
        }
        case RESPONSE_RECEIVED_HEADER: {
            switch (server->getResponse()->getCode()) {
                case HTTP_CODE_OK:
                    server->getResponse()->setType(DURING_CASH_RESPONSE);
                    cashScheduler.add_record(server->getRequest(), server->getResponse());
                    server->getClient()->receiving_mode_enable();
                    return RESPONSE_RECEIVE_PROCESS;
                case HTTP_CODE_NO_OK:
                    server->getResponse()->setType(NO_CASH_RESPONSE);
                    server->getClient()->receiving_mode_enable();
                    return RESPONSE_RECEIVE_PROCESS;
            }
        }

        case RESPONSE_RECEIVE_WITH_ERROR: {
            server->setStatus(STOP_FAILED_SERVER);
            server->response_mode_disable();
            return RESPONSE_RECEIVE_WITH_ERROR;
        }
    }
}

int PollScheduler::responseClientHandler(Client *client) {
    switch (ClientScheduler::send_response(client)) {
        case RESPONSE_RECEIVED: {
            spdlog::info("responseClientHandler RESPONSE_RECEIVED client {}", client->get_name_client());
            client->setStatus(STOP_SUCCESS_CLIENT);
            client->receiving_mode_disable();
            return RESPONSE_RECEIVED;
        }
        case RESPONSE_PAUSE_RECEIVE: {
            spdlog::warn("responseClientHandler RESPONSE_PAUSE_RECEIVE client {}", client->get_name_client());
            client->setStatus(PAUSE_CLIENT);
            client->receiving_mode_disable();
            return RESPONSE_PAUSE_RECEIVE;
        }
        case RESPONSE_RECEIVE_WITH_ERROR: {
            spdlog::error("responseClientHandler RESPONSE_RECEIVE_WITH_ERROR client {}", client->get_name_client());
            client->setStatus(STOP_FAILED_CLIENT);
            client->receiving_mode_disable();
            return RESPONSE_RECEIVE_WITH_ERROR;
        }
    }
}

void PollScheduler::add_client(Client *client) {
    clients.push_back(client);
}

void PollScheduler::add_server(Server *server) {
    servers.push_back(server);
}

int PollScheduler::executor() {

    switch (poll(polls, getCountHandler(), TIME_OUT_POLL)) {
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
                addClientConnectionHandler();

            }
            if (polls[1].revents & POLLIN) {
                eventCashHandler();
            }
            for (int i = 0; i < servers.size(); ++i) {
                if ((servers[i]->getPoll()->revents) & POLLHUP) {
                    spdlog::critical("executor POLLHUP servers {}", servers[i]->get_name_server());
                }
                if ((servers[i]->getPoll()->revents & POLLOUT)) {
                    requestServerHandler(servers[i]);

                }
                if ((servers[i]->getPoll()->revents) & POLLIN) {
                    responseServerHandler(servers[i]);
                }
            }
            for (int i = 0; i < clients.size(); ++i) {
                if ((clients[i]->getPoll()->revents) & POLLHUP) {
                    spdlog::critical("POLLHUP client");
                    clients[i]->receiving_mode_disable();
                    clients[i]->asking_mode_disable();
                    clients[i]->setStatus(STOP_FAILED_CLIENT);
                    continue;
                }

                if (clients[i]->getPoll()->revents & POLLIN) {
                    requestClientHandler(clients[i]);
                }
                if (clients[i]->getPoll()->revents & POLLOUT) {
                    responseClientHandler(clients[i]);
                }
            }


        }
    }
    return 0;
};

void PollScheduler::destroy() {
    for (int i = 0; i < clients.size(); ++i) {
        Client *client = clients[i];
        client->setStatus(STOP_FAILED_CLIENT);
    }
    for (int i = 0; i < servers.size(); ++i) {
        Server *server = servers[i];
        server->setStatus(STOP_FAILED_SERVER);
    }
}

PollScheduler::~PollScheduler() {
    delete polls;
    delete p_polls;
}
