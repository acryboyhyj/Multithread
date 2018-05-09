#ifndef CONCURRENT_QUEUE_H_
#define CONCURRENT_QUEUE_H_

#include <condition_variable>  // NOLINT
#include <iostream>
#include <mutex>  // NOLINT
#include <queue>
#include <utility>

template <class Data>
class concurrent_queue {
public:
    concurrent_queue() {}
    ~concurrent_queue() {}

    concurrent_queue(const concurrent_queue&) = delete;
    concurrent_queue& operator=(const concurrent_queue&) = delete;

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
    Data pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition_variable.wait(lock,
                                  [this] { return !this->m_queue.empty(); });
        auto data = m_queue.front();
        m_queue.pop();
    }
    void pop(Data& data) {  // NOLINT
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition_variable.wait(lock,
                                  [this] { return !this->m_queue.empty(); });
        data = std::move(m_queue.front());
        m_queue.pop();
    }
    Data const& front() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.front();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

private:
    mutable std::mutex m_mutex;
    mutable std::condition_variable m_condition_variable;
    std::queue<Data> m_queue;
};

#endif  // CONCURRENT_QUEUE_H_