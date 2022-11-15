//
// Created by rodion on 04.11.22.
//

#include "Request.h"
#include "../Configuration.h"
#include <malloc.h>
#include <signal.h>

Request::Request() {
    len_buf = BUF_STEP_SIZE_REQUEST;
    request = (char *) calloc(len_buf, sizeof(char));
    if (request == nullptr) {
        raise(SIGTERM);
    }
}

void Request::append_buf() {
    len_buf += BUF_STEP_SIZE_REQUEST;
    request = (char *) realloc(request, len_buf * sizeof(char));
    if (request == nullptr) {
        raise(SIGTERM);
    }
}

void Request::resolve() {
    method = request;
    for (; request[method_len] != ' '; ++method_len) {}
    resource = method + method_len + 1;
    for (resource_len = 0; resource[resource_len] != ' '; ++resource_len) {}
    version_http = resource + resource_len + 1;
    for (version_http_len = 0; version_http[version_http_len] != '\r'; ++version_http_len) {}
    host = (version_http + version_http_len);
    for (int j = 0; j < req_len - resource_len - method_len - version_http_len - 7; j++) {
        host += 1;
        if ((host[0] == 'H' || host[0] == 'h')
            && (host[1] == 'O' || host[1] == 'o')
            && (host[2] == 'S' || host[2] == 's')
            && (host[3] == 'T' || host[3] == 't')
            && (host[4] == ':')) {
            host += 5;
            break;
        }
    }
    for (; host[0] == ' '; host++) {}
    for (host_len = 0; host[host_len] != ' ' && host[host_len] != '\r'; host_len++) {}
}

Request::~Request() {
    if (request != nullptr)
        free(request);
}

char *Request::getRequest() const {
    return request;
}

void Request::setRequest(char *request) {
    Request::request = request;
}

long Request::getReqLen() const {
    return req_len;
}

void Request::setReqLen(long reqLen) {
    req_len = reqLen;
}

long Request::getLenBuf() const {
    return len_buf;
}

void Request::setLenBuf(long lenBuf) {
    len_buf = lenBuf;
}

char *Request::getMethod() const {
    return method;
}

void Request::setMethod(char *method) {
    Request::method = method;
}

long Request::getMethodLen() const {
    return method_len;
}

void Request::setMethodLen(long methodLen) {
    method_len = methodLen;
}

char *Request::getResource() const {
    return resource;
}

void Request::setResource(char *resource) {
    Request::resource = resource;
}

long Request::getResourceLen() const {
    return resource_len;
}

void Request::setResourceLen(long resourceLen) {
    resource_len = resourceLen;
}

char *Request::getVersionHttp() const {
    return version_http;
}

void Request::setVersionHttp(char *versionHttp) {
    version_http = versionHttp;
}

long Request::getVersionHttpLen() const {
    return version_http_len;
}

void Request::setVersionHttpLen(long versionHttpLen) {
    version_http_len = versionHttpLen;
}

char *Request::getHost() const {
    return host;
}

void Request::setHost(char *host) {
    Request::host = host;
}

long Request::getHostLen() const {
    return host_len;
}

void Request::setHostLen(long hostLen) {
    host_len = hostLen;
}
