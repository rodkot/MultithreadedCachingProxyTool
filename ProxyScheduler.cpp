//
// Created by rodion on 03.11.22.
//

#include <iostream>
#include <csignal>
#include "ProxyScheduler.h"
#include "client/Client.h"
#include "server/ServerScheduler.h"

#define    lock_resource(response) { \
 pthread_mutex_lock(response->getPthreadMutex());

#define    unlock_response(response) \
     pthread_mutex_unlock(response->getPthreadMutex()); \
  }


ProxyScheduler::ProxyScheduler(ConnectionScheduler &connectionScheduler, ClientScheduler &clientScheduler1,
                               CashScheduler &cashScheduler, Client *c)
        : connectionScheduler(connectionScheduler), clientScheduler(clientScheduler1), cashScheduler(cashScheduler),
          client(c) {
    request = new Request();
    polls = (pollfd *) realloc(polls, 2 * sizeof(pollfd));
    p_poll = (pollfd **) realloc(p_poll, 2 * sizeof(pollfd *));
}


int ProxyScheduler::builder() {
    int current_poll = 0;
    if (client != nullptr) {

        if (client->getStatus() == PAUSE_CLIENT) {
            lock_resource(response)
                if (response->getLenResponse() - client->getCurrentRecvResponse() > 0 ||
                    response->getStatus() == END) {
                    client->setStatus(ACTIVE_CLIENT);
                    client->receiving_mode_enable();
       //             spdlog::warn("responseClientHandler ENABLE client {}", client->get_name_client());
                }
            unlock_response(response)
        }

        if (client->getStatus() == STOP_SUCCESS_CLIENT ||
            client->getStatus() == STOP_FAILED_CLIENT) {
            close(client->getFd());

            if (response != nullptr) {
                lock_resource(response);
                    switch (response->getType()) {
                        case CASHED_RESPONSE:
                        case DURING_CASH_RESPONSE: {
                            cashScheduler.disconnect_to_record(request);
                        }
                        case NO_CASH_RESPONSE: {
                            if (server != nullptr)
                                server->setStatus(STOP_SUCCESS_SERVER);
                        }
                    }
                unlock_response(response);
            }

            if (client->getStatus() == STOP_SUCCESS_CLIENT) {
                spdlog::warn("ProxyScheduler STOP_SUCCESS_CLIENT disconnect client {}",
                             client->get_name_client());


            } else if (client->getStatus() == STOP_FAILED_CLIENT) {
                spdlog::error("ProxyScheduler STOP_FAILED_CLIENT disconnect client {}",
                              client->get_name_client());

            }
            delete client;
            client = nullptr;

        } else {
            polls[current_poll] = *(client->getPoll());
            p_poll[current_poll] = (client->getPoll());
            current_poll++;
        }
    }

    if (server != nullptr) {
        int status = server->getStatus();
        if (status == STOP_SUCCESS_SERVER || status == STOP_FAILED_SERVER) {
            close(server->getFd());
            lock_resource(response);
                if (status == STOP_FAILED_SERVER) {
                    spdlog::error("PollScheduler STOP_FAILED_SERVER disconnect server {}",
                                  server->get_name_server());
                    response->setStatus(FAIL);
                } else {
                    spdlog::warn("PollScheduler STOP_SUCCESS_SERVER disconnect server {}",
                                 server->get_name_server());
                    if (response->getType() == DURING_CASH_RESPONSE)
                        response->setType(CASHED_RESPONSE);
                    response->setStatus(END);
                }
            unlock_response(response);

            delete server;
            server = nullptr;
        } else {
            polls[current_poll] = *(server->getPoll());
            p_poll[current_poll] = (server->getPoll());
            current_poll++;
        }

    }
    if (current_poll == 0) {
        if (response != nullptr) {
            if (response->getType() == NO_CASH_RESPONSE)
                delete response;
            response = nullptr;
        }
        return STOP_PROXY;
    } else
        return ACTIVE_PROXY;
};

int ProxyScheduler::addServerConnectionHandler() {

    switch (ClientScheduler::check_valid_request(request)) {
        case NO_SUPPORT_METHOD: {
            spdlog::error("addServerConnectionHandler NO_SUPPORT_METHOD request client {}",
                          client->get_name_client());
            response = cashScheduler.response_405;
            client->receiving_mode_enable();
            //http 405
            return NO_SUPPORT_METHOD;
        }
        case NO_SUPPORT_VERSION_HTTP: {
            spdlog::error("addServerConnectionHandler NO_SUPPORT_VERSION_HTTP request client {}",
                          client->get_name_client());
            response = cashScheduler.response_505;
            client->receiving_mode_enable();
            // http 505
            return NO_SUPPORT_VERSION_HTTP;
        }
        case ERROR_RESOURCE: {
            spdlog::error("addServerConnectionHandler ERROR_RESOURCE request client {}", client->get_name_client());
            response = cashScheduler.response_409;
            client->receiving_mode_enable();
            // http 409
            return ERROR_RESOURCE;
        }
        case VALID_REQUEST: {
            spdlog::info("addServerConnectionHandler VALID_REQUEST request client {}", client->get_name_client());
            switch (cashScheduler.connect_to_record(request, &response)) {
                case HAVE_RECORD:
                    spdlog::info("addServerConnectionHandler HAVE_RECORD cache for request client {}",
                                 client->get_name_client());
                    client->receiving_mode_enable();
                    return HAVE_RECORD;
                case NO_RECORD: {
                    spdlog::info("addServerConnectionHandler NO_RECORD cache for request client {}",
                                 client->get_name_client());
                    auto new_server = new Server();
                    switch (connectionScheduler.connect_to_server(new_server,request)) {
                        case CONNECTION_SERVER_FAILED: {
                            spdlog::error(
                                    "addServerConnectionHandler CONNECTION_SERVER_FAILED for request client {}",
                                    client->get_name_client());
                            delete new_server;
                            response = cashScheduler.response_409;
                            client->receiving_mode_enable();
                            return CONNECTION_SERVER_FAILED;
                        }
                        case CONNECTION_SERVER_SUCCESS: {
                            spdlog::info(
                                    "addServerConnectionHandler CONNECTION_SERVER_SUCCESS for request client {}",
                                    client->get_name_client());
                            server = new_server;
                            server->request_mode_enable();

                            return CONNECTION_SERVER_SUCCESS;
                        }
                    }
                }
            }
        }
    }
}

int ProxyScheduler::requestClientHandler() {
    switch (ClientScheduler::forming_request(client,request)) {
        case REQUEST_RECV_SUCCESS:
            spdlog::info("requestClientHandler REQUEST_RECV_SUCCESS client {}", client->get_name_client());
            client->asking_mode_disable();
            request->resolve();
            addServerConnectionHandler();
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

int ProxyScheduler::requestServerHandler() {
    switch (ServerScheduler::send_request(server,request)) {
        case REQUEST_HAS_BEEN_SEND: {
            spdlog::info("requestServerHandler REQUEST_HAS_BEEN_SEND server {}", server->get_name_server());
            server->request_mode_disable();
            response = new Response();
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

int ProxyScheduler::responseServerHandler() {
    switch (ServerScheduler::recv_response(server,response)) {
        case RESPONSE_RECEIVED: {
                spdlog::info("responseServerHandler RESPONSE_RECEIVED server {}", server->get_name_server());

                if (response->getType() == DURING_CASH_RESPONSE)
                    response->setType(CASHED_RESPONSE);

                server->setStatus(STOP_SUCCESS_SERVER);
                server->response_mode_disable();
            return RESPONSE_RECEIVED;
        }
        case RESPONSE_RECEIVED_HEADER: {
            switch (response->getCode()) {
                case HTTP_CODE_OK:
                    response->setType(DURING_CASH_RESPONSE);
                    cashScheduler.add_record(request, response);
                    client->receiving_mode_enable();
                    return RESPONSE_RECEIVE_PROCESS;
                case HTTP_CODE_NO_OK:
                    response->setType(NO_CASH_RESPONSE);
                    client->receiving_mode_enable();
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

int ProxyScheduler::responseClientHandler() {
    switch (ClientScheduler::send_response(client,response)) {
        case RESPONSE_RECEIVED: {
            spdlog::info("responseClientHandler RESPONSE_RECEIVED client {}", client->get_name_client());
            client->setStatus(STOP_SUCCESS_CLIENT);
            client->receiving_mode_disable();
            return RESPONSE_RECEIVED;
        }
        case RESPONSE_PAUSE_RECEIVE: {
           // spdlog::warn("responseClientHandler RESPONSE_PAUSE_RECEIVE client {}", client->get_name_client());
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

int ProxyScheduler::executor() {
    switch (poll(polls, getCountHandler(), 1000)) {
        case 0: {
            return 0;
        }
        case -1: {
            return -1;
        }
        default: {
            for (int i = 0; i < getCountHandler(); ++i) {
                *(p_poll[i]) = polls[i];
            }
            if (server != nullptr) {
                if ((server->getPoll()->revents) & POLLHUP) {
                    spdlog::critical("executor POLLHUP servers {}", server->get_name_server());
                }
                if ((server->getPoll()->revents & POLLOUT)) {
                    requestServerHandler();

                }
                if (server->getPoll()->revents & POLLIN) {
                    lock_resource(response)
                    responseServerHandler();
                    unlock_response(response);
                }
            }

            if (client != nullptr) {
                if (client->getPoll()->revents & POLLHUP) {
                    spdlog::critical("POLLHUP client");
                    client->receiving_mode_disable();
                    client->asking_mode_disable();
                    client->setStatus(STOP_FAILED_CLIENT);
                }

                if (client->getPoll()->revents & POLLIN) {
                    requestClientHandler();
                }
                if (client->getPoll()->revents & POLLOUT) {
                    lock_resource(response);
                    responseClientHandler();
                    unlock_response(response)
                }
            }


        }

    }
    return 0;
};

void ProxyScheduler::destroy() {
    if (client != nullptr)
        client->setStatus(STOP_FAILED_CLIENT);
    if (server != nullptr)
        server->setStatus(STOP_FAILED_SERVER);
}

ProxyScheduler::~ProxyScheduler() {
    delete polls;
    delete p_poll;
}
