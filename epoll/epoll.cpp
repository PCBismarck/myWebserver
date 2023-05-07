#include "epoll.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include "../log/log.h"

Epoll::Epoll(int max_events)
    : m_epollfd(epoll_create(512)), m_events(max_events)
{
    assert(m_epollfd > 0);
    assert(max_events > 0);
}

Epoll::~Epoll() { close(m_epollfd); }

void Epoll::addFd(int fd_toadd, uint32_t events)
{
    assert(fd_toadd > 0);
    epoll_event event{0};
    event.data.fd = fd_toadd;
    event.events = events;
    assert(0 == epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd_toadd, &event));
    // epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd_toadd, &event);
    // LOG_ERROR("ADDFD: %d  error:%s", fd_toadd, strerror(errno));
    // assert(ret == 0);
    // assert(0 == epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd_todel, &event));
}

void Epoll::delFd(int fd_todel)
{
    assert(fd_todel > 0);
    epoll_event event{0};
    // epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd_todel, &event);
    // LOG_INFO("DELFD: %d  error:%s", fd_todel, strerror(errno));
    // assert(ret == 0);
    assert(0 == epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd_todel, &event));
}

void Epoll::modFd(int fd_tomod, uint32_t events)
{
    assert(fd_tomod > 0);
    epoll_event event{0};
    event.data.fd = fd_tomod;
    event.events = events;
    assert(0 == epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd_tomod, &event));
}

int Epoll::wait(int timeoutMS)
{
    return epoll_wait(m_epollfd, &m_events[0], m_events.size(), timeoutMS);
}

uint32_t Epoll::getEvents(size_t i) const
{
    assert(i < m_events.size());
    return m_events[i].events;
}

int Epoll::getEventFd(size_t i) const
{
    assert(i < m_events.size());
    return m_events[i].data.fd;
}