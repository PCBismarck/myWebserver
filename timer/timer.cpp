#include "timer.h"
#include <bits/types/time_t.h>
#include <cassert>
#include <ctime>
// #include "../log/log.h"

TimerList::TimerList(int size)
    : m_size(size), m_start(new BaseTimer(0, 0, nullptr)),
      m_end(new BaseTimer(0, 0, nullptr)), m_expire(nullptr)
{
    assert(m_size > 0);
    assert(m_start != nullptr);
    assert(m_end != nullptr);
    m_start->next = m_end;
    m_end->prev = m_start;
}

TimerList::~TimerList()
{
    BaseTimer* timer = m_start->next;
    while (timer != m_end) {
        BaseTimer* to_del = timer;
        timer = timer->next;
        delete to_del;
    }
    delete m_start;
    delete m_end;
}

// 处理到期的定时器并返回距离下一个定时器的超时时间(S)
time_t TimerList::tick()
{
    assert(m_start != nullptr);
    assert(m_end != nullptr);
    // assert(m_start->next != nullptr);
    if (m_start->next == nullptr)
        return -1;
    BaseTimer* timer = m_start->next;
    time_t cur = time(nullptr);
    while (timer != m_end) {
        if (cur < timer->expire) {
            break;
        }
        timer->callback();
        BaseTimer* to_del = timer;
        timer = timer->next;
        removeTimer(to_del);
        m_timers.erase(to_del->fd);
        delete to_del;
    }
    if (timer == m_end) {   // 定时器全部过期
        return -1;
    }
    return timer->expire - cur;
}

bool TimerList::addTimer(BaseTimer* timer)
{
    assert(timer != nullptr);
    if (m_timers.size() >= m_size) {
        return false;
    }
    // LOG_DEBUG("add timer, fd = %d, expire in: %d", timer->fd, timer->expire);
    addToEnd(timer);
    m_timers[timer->fd] = timer;
    return true;
}

void TimerList::enableAutoExpire(ExpireTime expire)
{
    assert(expire != nullptr);
    is_auto_expire = true;
    m_expire = expire;
}

void TimerList::disableAutoExpire()
{
    is_auto_expire = false;
    m_expire = nullptr;
}

void TimerList::moveToEnd(BaseTimer* timer)
{
    assert(timer != nullptr);
    if (is_auto_expire) {
        assert(m_expire != nullptr);
        timer->expire = m_expire();
    }
    removeTimer(timer);
    addToEnd(timer);
}

void TimerList::removeTimer(BaseTimer* timer)
{
    assert(timer != nullptr);
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
}

void TimerList::addToEnd(BaseTimer* timer)
{
    assert(timer != nullptr);
    timer->next = m_end;
    timer->prev = m_end->prev;
    m_end->prev->next = timer;
    m_end->prev = timer;
    if (is_auto_expire) {
        assert(m_expire != nullptr);
        timer->expire = m_expire();
        // LOG_INFO("add timer, fd = %d, expire = %ld", timer->fd,
        // timer->expire);
    }
}