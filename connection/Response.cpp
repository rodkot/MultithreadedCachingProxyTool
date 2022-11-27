//
// Created by rodion on 05.11.22.
//

#include <cstdlib>
#include "Response.h"
#include "../Configuration.h"
#include <csignal>
#include <pthread.h>

Response::Response() {
    pthreadMutex=(pthread_mutex_t*) calloc(1,sizeof(pthread_mutex_t));
    pthread_mutex_init(pthreadMutex, nullptr);
    len_buf = BUF_STEP_SIZE_RESPONSE;
    response = (char *) calloc(len_buf, sizeof(char));
    if (response==nullptr){
        raise(SIGTERM);
    }
}

Response::Response(char *res, long res_len, int status, int type) : response(res), len_response(res_len),
                                                                    status(status), type(type) {
    pthreadMutex=(pthread_mutex_t*) calloc(1,sizeof(pthread_mutex_t));
    pthread_mutex_init(pthreadMutex, nullptr);

}

void Response::append_buf() {
    len_buf += BUF_STEP_SIZE_RESPONSE;
    response = (char *) realloc(response, len_buf * sizeof(char));
    if (response==nullptr){
        raise(SIGTERM);
    }
}

void Response::resolve() {
    int len_http_version = 0;
    for (; response[len_http_version] != ' '; len_http_version++) {}
    char *http_code = response + len_http_version + 1;
    int len_http_code = 0;
    for (; http_code[len_http_code] != ' '; len_http_code++) {}
    if (http_code[0] == '2' &&
        http_code[1] == '0' &&
        http_code[2] == '0') {
        code = HTTP_CODE_OK;
    } else {
        code = HTTP_CODE_NO_OK;
    }
}

int Response::getType() const {
    return type;
}

void Response::setType(int type) {
    Response::type = type;
}

char *Response::getResponse() const {
    return response;
}

void Response::setResponse(char *response) {
    Response::response = response;
}

long Response::getLenResponse() const {
    return len_response;
}

void Response::setLenResponse(long lenResponse) {
    len_response = lenResponse;
}

int Response::getCode() const {
    return code;
}

void Response::setCode(int code) {
    Response::code = code;
}

int Response::getStatus() const {
    return status;
}

void Response::setStatus(int status) {
    Response::status = status;
}

long Response::getLenBuf() const {
    return len_buf;
}

void Response::setLenBuf(long lenBuf) {
    len_buf = lenBuf;
}
Response::~Response() {
    delete response;
    pthread_mutex_destroy(pthreadMutex);
    delete pthreadMutex;
}
pthread_mutex_t *Response::getPthreadMutex() const {
    return pthreadMutex;
}
