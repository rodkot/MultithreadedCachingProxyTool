cmake_minimum_required(VERSION 3.23)
project(proxy)

add_executable(proxy main.cpp client/Client.cpp client/Client.h client/ClientScheduler.cpp client/ClientScheduler.h connection/ConnectionScheduler.cpp connection/ConnectionScheduler.h ProxyScheduler.cpp ProxyScheduler.h connection/Request.cpp connection/Request.h server/Server.cpp server/Server.h server/ServerScheduler.cpp server/ServerScheduler.h connection/Response.cpp connection/Response.h cash/CashScheduler.cpp cash/CashScheduler.h cash/CashRecord.cpp cash/CashRecord.h Configuration.h ThreadScheduler.cpp ThreadScheduler.h)



