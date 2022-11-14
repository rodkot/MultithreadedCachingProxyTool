//
// Created by rodion on 05.11.22.
//

#include <cstdlib>
#include "Response.h"
#include "../Configuration.h"
#include <signal.h>
#include <cerrno>


Response::Response() {
    len_buf = BUF_STEP_SIZE_RESPONSE;
    response = (char *) calloc(len_buf, sizeof(char));
    if (response==nullptr){
        raise(SIGTERM);
    }
}

Response::Response(char *res, long res_len, int status, int type) : response(res), len_response(res_len),
                                                                    status(status), type(type) {

}

void Response::append_buf() {
    len_buf += BUF_STEP_SIZE_RESPONSE;
    response = (char *) realloc(response, len_buf * sizeof(char));
    if (response==nullptr){
        raise(SIGTERM);
    }
}

void Response::resolve() {
    char *http_version = response;
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
