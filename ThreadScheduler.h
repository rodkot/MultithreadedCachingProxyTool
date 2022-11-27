//
// Created by rodion on 27.11.22.
//

#ifndef PROXY_THREADSCHEDULER_H
#define PROXY_THREADSCHEDULER_H

#include <vector>
#include <pthread.h>
#include "Configuration.h"

#define DENIED -1
#define ALLOWED 1


class ThreadScheduler {
private:
    std::vector <pthread_t> list_proxy;
    pthread_mutex_t mutex_list_proxy = PTHREAD_MUTEX_INITIALIZER;
public:
    int check_availability();
    void  add_new_thread(pthread_t );
    void  cancel_all();
    void delete_thread(pthread_t);
private:
    void try_join();
};


#endif //PROXY_THREADSCHEDULER_H
