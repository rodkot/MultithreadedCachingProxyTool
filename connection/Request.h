//
// Created by rodion on 04.11.22.
//

#ifndef PROXY_REQUEST_H
#define PROXY_REQUEST_H
#define BUF_STEP_SIZE 100

class Request {
public:
    char *request;
    long req_len = 0;
    long len_buf = 0;

    char *method{};
    long method_len = 0;

    char *resource{};
    long resource_len = 0;

    char *version_http{};
    long version_http_len = 0;

    char *host{};
    long host_len = 0;

    explicit Request();

    int resolve();

    int append_buf();


};


#endif //PROXY_REQUEST_H
