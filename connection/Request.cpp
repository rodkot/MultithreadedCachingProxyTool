//
// Created by rodion on 04.11.22.
//

#include "Request.h"
#include <malloc.h>

Request::Request(){
    len_buf = BUF_STEP_SIZE;
    request = (char *)calloc(len_buf, sizeof (char));
}

int Request::append_buf() {
    len_buf += BUF_STEP_SIZE;
    request = (char *)realloc(request,len_buf*sizeof(char));
}

int Request::resolve() {
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
    for (;host[0]==' ';host++) {}
    for (host_len = 0; host[host_len]!=' ' && host[host_len]!='\r' ; host_len++) {}
    return 0;
}
