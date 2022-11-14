//
// Created by rodion on 05.11.22.
//

#ifndef PROXY_RESPONSE_H
#define PROXY_RESPONSE_H


#define DURING_CASH_RESPONSE 10
#define CASHED_RESPONSE 11
#define NO_CASH_RESPONSE 12



#define HEADERS 1
#define BODY 2
#define FAIL -1
#define END 3


#define HTTP_CODE_OK 200
#define HTTP_CODE_NO_OK 201

class Response {
public:
    int type;

    char *response;
    long len_response = 0;

    int code = 0;
    int status = HEADERS;

    long len_buf = 0;

    Response();
    Response(char* res, long res_len,int status,int type);

    void append_buf();
    void resolve();
};


#endif //PROXY_RESPONSE_H
