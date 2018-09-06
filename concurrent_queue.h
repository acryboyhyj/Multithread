#ifndef CONCURREN_QUEUE_H_
#define CONCURREN_QUEUE_H_

#include <condition_variable>  // NOLINData
#include <iostream>
#include <mutex>  // NOLINData
#include <queue>
#include <utility>

template <class Data>
class ConcurrentQueue {
public:
    ConcurrentQueue() {}
    ~ConcurrentQueue() {}

    ConcurrentQueue(const ConcurrentQueue&) = delete;
    ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

    size_t Size() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    void push(const Data& data) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(data);
        }
        m_condition_variable.notify_one();
    }

    void push(Data&& data) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(std::move(data));
        }
        m_condition_variable.notify_one();
    }

    void pop(Data& data) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition_variable.wait(
                lock, [this] { return !this->m_queue.empty(); });
            data = std::move(m_queue.front());
            m_queue.pop();
        }
    }
    Data pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition_variable.wait(lock,
                                  [this] { return !this->m_queue.empty(); });
        auto data = m_queue.front();
        m_queue.pop();
        return data;
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_condition_variable;
    std::queue<Data> m_queue;
};
#endif  // CONCURRENData_QUEUE_H_
