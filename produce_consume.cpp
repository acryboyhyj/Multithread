#include <memory.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
template <class Data>
class Buffer {
private:
    int m_capacity;
    int m_front;
    int m_rear;
    int m_size;

    Data* m_buffer;
    std::mutex m_mutex;
    std::condition_variable m_not_full;
    std::condition_variable m_not_empty;

public:
    explicit Buffer(int capacity)
        : m_capacity(capacity),
          m_front(0),
          m_rear(0),
          m_size(0),
          m_buffer(new Data(capacity)) {}

    ~Buffer() { delete[] m_buffer; }
    Buffer operator=(const Buffer&) = delete;
    Buffer(const Buffer&)           = delete;

    void Deposit(const Data& data) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_not_full.wait(lock, [this] { return m_size < m_capacity; });
            m_buffer[m_rear] = data;
            m_rear           = (m_rear + 1) % m_capacity;
            m_size++;
        }
        m_not_empty.notify_one();
    }

    void fetch(Data& data) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_not_empty.wait(lock, [this] { return m_size > 0; });
            data    = std::move(m_buffer[m_front]);
            m_front = (m_front + 1) % m_capacity;
            m_size--;
        }
        m_not_full.notify_one();
    }
};

template <class Data>
class Consumer {
public:
    explicit Consumer(Buffer<Data>& buffer) : m_buffer(buffer) {}
    ~Consumer();

    void Consume() {
        for (int i = 0; i < 50; ++i) {
            Data data;
            m_buffer.fetch(data);
            std::cout << "consumer "
                      << " comsume " << data << '\n';
        }
    }

private:
    Buffer<Data>& m_buffer;
};

template <class Data>
class Producer {
public:
    explicit Producer(Buffer<Data>& buffer) : m_buffer(buffer) {}
    ~Producer();

    void Produce() {  // NOLINT
        for (int i = 0; i < 50; i++) {
            m_buffer.Deposit(i);
            std::cout << "product  produced " << i << '\n';
        }
    }

private:
    Buffer<Data>& m_buffer;
};

int main() {
    std::thread thread_consume[5];
    std::thread thread_produce[5];

    Buffer<int> buffer(200);
    std::vector<Producer<int>> producers{Producer<int>(buffer)};
    for (int i = 0; i < 5; ++i) {
        thread_consume[i] = std::thread();
        thread_produce[i] = std::thread();
    }
    for (auto& th_c : thread_consume) th_c.join();
    for (auto& th_p : thread_produce) th_p.join();
    return 0;
}
