#ifndef __EPOLL_H__
#define __EPOLL_H__

#include <cstdint>
#include <memory>
#include <sys/epoll.h>
#include <vector>

class Epoll
{
    using EventList = std::vector<struct epoll_event>;

public:
    Epoll(int max_events = 1024);
    ~Epoll();

    void addFd(int fd, uint32_t events);
    void delFd(int fd);
    void modFd(int fd, uint32_t events);
    int wait(int timeoutMS = -1);
    uint32_t getEvents(size_t i) const;
    int getEventFd(size_t i) const;

private:
    int m_epollfd;
    EventList m_events;
};

#endif   // __EPOLL_H__