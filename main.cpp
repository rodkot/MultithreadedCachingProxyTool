#include <iostream>
#include <unistd.h>
#include "ProxyScheduler.h"
#include "connection/ConnectionScheduler.h"
#include "client/ClientScheduler.h"
#include <spdlog/spdlog.h>
#include <csignal>
#include "Configuration.h"
#include "ThreadScheduler.h"

ConnectionScheduler connectionScheduler(PROXY_ADDR, PROXY_PORT);
ClientScheduler clientScheduler;
ThreadScheduler threadScheduler;
CashScheduler cashScheduler;


sigset_t mask;
int stop_event_fd[2];

pthread_t tid_signal_thread;

void destroy(void *arguments) {
    auto *proxy = static_cast<ProxyScheduler *>(arguments);
    threadScheduler.delete_thread(pthread_self());
    proxy->destroy();
    proxy->builder();
    delete proxy;
    spdlog::critical("proxy delete");
}

void signal_thread_destroy(void *arguments) {
    write(stop_event_fd[1], "stop", 5);
    spdlog::critical("signal_thread_destroy");
}


void *signal_thread(void *arguments) {
    int err, signo;
    pthread_cleanup_push(signal_thread_destroy, (void *) nullptr);
        for (;;) {
            err = sigwait(&mask, &signo);
            if (err != 0)
                pthread_exit((void *) 1);
            switch (signo) {
                case SIGTERM:
                    write(stop_event_fd[1], "stop", 5);
                    break;
                case SIGALRM:
                    write(cashScheduler.cash_event_fd[1], "alarm", 5);
                    alarm(PERIOD_CASH_EVENT);
                    break;
                default:
                    printf("получен непредвиденный сигнал %d\n", signo);
                    pthread_exit((void *) 1);
            }
        }
    pthread_cleanup_pop(1);
    pthread_exit((void *) 0);
}

void *new_connection(void *arguments) {
    auto *client = static_cast<Client *>(arguments);
    auto *proxyScheduler = new ProxyScheduler(connectionScheduler, clientScheduler, cashScheduler, client);
    pthread_cleanup_push(destroy, (void *) proxyScheduler);
        while (proxyScheduler->builder() == ACTIVE_PROXY) {
            proxyScheduler->executor();
        }
    pthread_cleanup_pop(1);
    pthread_exit((void *) nullptr);
}

int init_listen_socket_poll(pollfd *p) {
    int socket = connectionScheduler.listen_socket();
    switch (socket) {
        case OPEN_CONNECTION_FAILED: {
            return OPEN_CONNECTION_FAILED;
        }
        default: {
            p->fd = socket;
            p->events = POLLIN;
            return OPEN_CONNECTION_SUCCESS;
        }
    }
}

void init_cash_poll(pollfd *p) {
    p->fd = cashScheduler.cash_event_fd[0];
    p->events = POLLIN;
    alarm(PERIOD_CASH_EVENT);
}

void init_stop_poll(pollfd *p) {
    pipe(stop_event_fd);
    p->fd = stop_event_fd[0];
    p->events = POLLIN;
}

int addClientConnectionHandler() {
    auto *new_client = new Client();

    switch (connectionScheduler.fresh_connection(new_client)) {
        case CLIENT_CONNECTION_SUCCESS: {
            if (threadScheduler.check_availability() != ALLOWED) {
                spdlog::critical("addClientConnectionHandler out count thread for client {}",
                                 new_client->get_name_client());
                close(new_client->getFd());
                delete new_client;
                return CLIENT_CONNECTION_FAILED;
            }
            pthread_t ntid;
            if((pthread_create(&ntid, nullptr, new_connection, (void *) new_client)!=0)){
                spdlog::critical("addClientConnectionHandler error create thread for client {}",
                                 new_client->get_name_client());
                close(new_client->getFd());
                delete new_client;
                return CLIENT_CONNECTION_FAILED;
            }
            threadScheduler.add_new_thread(ntid);
            new_client->asking_mode_enable();
            spdlog::info("connect success  client {}", new_client->get_name_client());
            return CLIENT_CONNECTION_SUCCESS;
        }
        case CLIENT_CONNECTION_FAILED:
            spdlog::error("connect error client {}", new_client->get_name_client());
            return CLIENT_CONNECTION_FAILED;
    }
}

void eventCashHandler() {
    spdlog::info("eventCashHandler");

    char buf[10];
    read(cashScheduler.cash_event_fd[0], buf, 10);
    cashScheduler.clean_unnecessary_cash();
}

void stop_proxy() {
    threadScheduler.cancel_all();
    pthread_cancel(tid_signal_thread);
}

void eventStopHandler() {
    spdlog::critical("eventStopHandler");

    char buf[10];
    read(stop_event_fd[0], buf, 10);
    stop_proxy();
    connectionScheduler.disconnect();
    cashScheduler.destroy();
    pthread_exit((void *) EXIT_SUCCESS);
}


int main() {
    pollfd polls[3];
    sigset_t oldmask;

    if (init_listen_socket_poll(&(polls[0])) == OPEN_CONNECTION_FAILED) {
        exit(EXIT_FAILURE);
    }

    init_cash_poll(&(polls[1]));
    init_stop_poll(&(polls[2]));

    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGTERM);

    if (pthread_sigmask(SIG_BLOCK, &mask, &oldmask) != 0)
        stop_proxy();

    if (pthread_create(&tid_signal_thread, nullptr, signal_thread, nullptr) != 0)
        stop_proxy();

    while (true) {
        switch (poll(polls, 3, TIME_OUT_POLL)) {
            case 0:
            case -1:
                stop_proxy();
            default: {
                if (polls[0].revents & POLLIN) {
                    addClientConnectionHandler();
                }
                if (polls[1].revents & POLLIN) {
                    eventCashHandler();
                    alarm(PERIOD_CASH_EVENT);
                }
                if (polls[2].revents & POLLIN) {
                    eventStopHandler();
                }
            }
        }
    }
    pthread_exit((void *) EXIT_SUCCESS);
}
