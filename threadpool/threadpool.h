#ifndef __THREADPOLL_H__
#define __THREADPOLL_H__

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>

// using T = std::function<void()>;
template <typename T>
class Channel
{
public:
    // 将一个元素放入channel中
    void put(T&& x);

    // 从channel中取出去一个元素，如果没有元素，就阻塞等待
    // 如果已经关闭，就返回nullptr，ok表示是否成功取出
    bool take(T& x);

    void notify_one() { m_cond.notify_one(); }
    bool isClosed()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return is_closed;
    }
    void close();
    size_t size();
    bool empty()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_queue.empty();
    }

private:
    std::mutex m_mtx;
    std::condition_variable m_cond;
    std::queue<T> m_queue;
    bool is_closed = false;
};

class Threadpool
{
    using Task = std::function<void()>;

public:
    Threadpool(int thread_num);
    ~Threadpool();

    void add(Task&& x);

private:
    int m_thread_num;
    std::shared_ptr<Channel<Task>> m_channel;
};

#endif   // __THREADPOLL_H__