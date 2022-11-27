//
// Created by rodion on 27.11.22.
//

#include <algorithm>
#include "ThreadScheduler.h"

void ThreadScheduler::cancel_all() {
    pthread_mutex_lock(&mutex_list_proxy);
    for (auto iter_proxy = list_proxy.begin();
         iter_proxy != list_proxy.end();) {
        auto proxy = *iter_proxy;
        if (pthread_tryjoin_np(proxy, nullptr) != 0) {
            pthread_cancel(proxy);
        }
        iter_proxy = list_proxy.erase(iter_proxy);
    }
    pthread_mutex_unlock(&mutex_list_proxy);
}

void ThreadScheduler::try_join() {
    for (auto iter_proxy = list_proxy.begin();
         iter_proxy != list_proxy.end();) {
        auto proxy = *iter_proxy;
        if (pthread_tryjoin_np(proxy, nullptr) == 0) {
            iter_proxy = list_proxy.erase(iter_proxy);
            continue;
        }
        iter_proxy++;
    }
}

int ThreadScheduler::check_availability() {
    pthread_mutex_lock(&mutex_list_proxy);
    try_join();
    if (list_proxy.size() >= MAX_COUNT_THREAD) {
        pthread_mutex_unlock(&mutex_list_proxy);
        return DENIED;
    } else {
        pthread_mutex_unlock(&mutex_list_proxy);
        return ALLOWED;
    }
}

void ThreadScheduler::delete_thread(pthread_t tid) {
    pthread_mutex_lock(&mutex_list_proxy);
    std::vector<pthread_t>::const_iterator iter;
    if ((iter = std::find(list_proxy.begin(), list_proxy.end(), tid)) != list_proxy.end()) {
        list_proxy.erase(iter);
    }

    pthread_mutex_unlock(&mutex_list_proxy);
}

void ThreadScheduler::add_new_thread(pthread_t tid) {
    pthread_mutex_lock(&mutex_list_proxy);
    try_join();
    list_proxy.push_back(tid);
    pthread_mutex_unlock(&mutex_list_proxy);

}