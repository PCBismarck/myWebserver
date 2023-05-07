#include "threadpool.h"
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

template <typename T>
void Channel<T>::close()
{
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        is_closed = true;
    }
    m_cond.notify_all();
}

template <typename T>
void Channel<T>::put(T&& x)
{
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_queue.emplace(std::forward<T>(x));
    }
    m_cond.notify_one();
}

// 从Channel取出去一个元素，如果没有元素，就阻塞等待
// 如果已经关闭，就返回空nullptr
template <typename T>
bool Channel<T>::take(T& x)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cond.wait(lock, [this] {
        return !m_queue.empty() || is_closed;
    });
    if (m_queue.empty())
        return false;
    assert(!m_queue.empty());
    x = std::move(m_queue.front());
    m_queue.pop();
    return true;
}

template <typename T>
size_t Channel<T>::size()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    return m_queue.size();
}

Threadpool::Threadpool(int thread_num)
    : m_thread_num(thread_num), m_channel(std::make_shared<Channel<Task>>())
{
    for (int i = 0; i < m_thread_num; ++i) {
        std::thread([Channel = m_channel] {
            while (true) {
                if (Channel->isClosed())
                    break;
                Task task;
                if (Channel->take(task))
                    task();
            }
        }).detach();
    }
}

Threadpool::~Threadpool()
{
    if (static_cast<bool>(m_channel)) {
        m_channel->close();
    }
}

void Threadpool::add(Task&& x)
{
    assert(m_channel != nullptr);
    m_channel->put(std::forward<Task>(x));
}