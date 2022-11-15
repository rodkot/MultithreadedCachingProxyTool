//
// Created by rodion on 04.11.22.
//

#ifndef PROXY_REQUEST_H
#define PROXY_REQUEST_H


class Request {
private:
    char *request;
    long req_len = 0;
    long len_buf = 0;

    char *method = nullptr;
    long method_len = 0;

    char *resource = nullptr;
    long resource_len = 0;

    char *version_http = nullptr;
    long version_http_len = 0;

    char *host = nullptr;
    long host_len = 0;

public:
    explicit Request();

    void resolve();

    void append_buf();

    char *getRequest() const;

    void setRequest(char *request);

    long getReqLen() const;

    void setReqLen(long reqLen);

    long getLenBuf() const;

    void setLenBuf(long lenBuf);

    char *getMethod() const;

    void setMethod(char *method);

    long getMethodLen() const;

    void setMethodLen(long methodLen);

    char *getResource() const;

    void setResource(char *resource);

    long getResourceLen() const;

    void setResourceLen(long resourceLen);

    char *getVersionHttp() const;

    void setVersionHttp(char *versionHttp);

    long getVersionHttpLen() const;

    void setVersionHttpLen(long versionHttpLen);

    char *getHost() const;

    void setHost(char *host);

    long getHostLen() const;

    void setHostLen(long hostLen);

    ~Request();

};


#endif //PROXY_REQUEST_H
