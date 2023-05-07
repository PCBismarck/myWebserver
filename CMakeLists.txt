cmake_minimum_required(VERSION 3.0.0)
project(webserver VERSION 0.1.0)

include(CTest)
enable_testing()
                # webserver/webserver.h 
                # threadpool/threadpool.h 
                # log/log.h 
                # http/httpconn.h 
                # http/httpinfo.h
                # http/httprequest.h 
                # http/httpresponse.h 
                # http/router.h 
                # timer/timer.h 
                # epoll/epoll.h 

add_executable(webserver 
                # threadpool/threadpool.cpp
                # log/log.cpp
                timer/timer.cpp
                http/httprequest.cpp
                http/router.cpp
                epoll/epoll.cpp
                webserver/webserver.cpp
                main.cpp 
                )
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
