#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <unistd.h>
#include "PollScheduler.h"
#include "connection/ConnectionScheduler.h"
#include "client/ClientScheduler.h"
#include <spdlog/spdlog.h>

int main() {


    ConnectionScheduler connectionScheduler("127.0.0.1", 8081);
    ClientScheduler clientScheduler;
    CashScheduler cashScheduler;
    PollScheduler pollScheduler(connectionScheduler, clientScheduler, cashScheduler);

    switch (pollScheduler.open_connect()) {
        case OPEN_CONNECTION_SUCCESS: {
            spdlog::info("PROXY START");
            while (true) {
                pollScheduler.builder();
                pollScheduler.executor();
            }
        }
        case OPEN_CONNECTION_FAILED: {
            spdlog::critical("PROXY FAILED START");
            //std::cout<<termcolor::red<<"OPEN_CONNECTION_FAILED"<<std::endl;
        }
    }








//    struct pollfd* polls = nullptr;
//    polls= static_cast<pollfd *>(calloc(1, sizeof(struct pollfd)));
//
//    int fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
//    polls[0].fd=fd;
//    polls[0].events=POLLIN;
//    struct sockaddr_in mysock{};
//
//    mysock.sin_family = AF_INET;
//    mysock.sin_port = htons(8080);
//    mysock.sin_addr.s_addr = inet_addr("127.0.0.1");
//
//    if(bind(fd,(sockaddr *)(&mysock), sizeof (mysock))<0){
//        perror("error");
//    }
//    if(listen(fd,10)){
//        perror("listen connect");
//    }
//    if (poll(polls,1,-1)>0  ){
//        char buf[100];
//        int d = read(fd,buf,100);
//        perror("hah");
//    } else
//    {
//        perror("error_connect");
//    }
//
//    return 0;


}
