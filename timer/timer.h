#ifndef __TIMER_H__
#define __TIMER_H__

#include <bits/types/time_t.h>
#include <functional>
#include <unordered_map>

struct BaseTimer {
    using TimeoutCallBack = std::function<void()>;
    int fd;
    time_t expire;
    TimeoutCallBack callback;
    BaseTimer* prev;
    BaseTimer* next;
    BaseTimer(int fd_, time_t expire_, TimeoutCallBack cb_)
        : fd(fd_), expire(expire_), callback(cb_), prev(nullptr), next(nullptr)
    {
    }
    // 不设置过期时间，由tiumerlist计算
    BaseTimer(int fd_, TimeoutCallBack cb_)
        : fd(fd_), callback(cb_), prev(nullptr), next(nullptr)
    {
    }
};

// 过期时间按升序排列的timerlist
// 不加锁，由主线程负责添加和删除定时器
class TimerList
{
    using TimerIndex = std::unordered_map<int, struct BaseTimer*>;
    using ExpireTime = std::function<time_t()>;

public:
    TimerList(int size);
    ~TimerList();

    // 返回下一个定时器的过期时间(S)，全部过期则返回-1
    time_t tick();
    bool addTimer(BaseTimer* timer);
    void refreshTimer(BaseTimer* timer)
    {
        if (!timer)
            return;
        moveToEnd(timer);
    }
    void enableAutoExpire(ExpireTime fresh);
    void disableAutoExpire();
    BaseTimer* getTimer(int connfd) const
    {
        if (m_timers.find(connfd) == m_timers.end())
            return nullptr;
        return m_timers.at(connfd);
    }

private:
    void moveToEnd(BaseTimer* timer);
    void removeTimer(BaseTimer* timer);
    void addToEnd(BaseTimer* timer);

private:
    bool is_auto_expire = false;   // 是否由TimeList计算过期时间
    int m_size;
    TimerIndex m_timers;   // socketfd -> timer
    BaseTimer* m_start;
    BaseTimer* m_end;
    ExpireTime m_expire;   // 计算过期时间的函数
};

#endif   //__TIMER_H__