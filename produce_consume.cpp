#include <memory.h>

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
          m_buffer(new Data[m_capacity]) {}

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

    void Fetch(Data& data) {
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
void Consume(Buffer<Data>& buffer) {
    for (int i = 0; i < 50; ++i) {
        Data data;
        buffer.Fetch(data);
        std::cout << "consumer "
                  << " comsume " << data << '\n';
    }
}

template <class Data>
void Produce(Buffer<Data>& buffer) {  // NOLINT
    for (int i = 0; i < 50; i++) {
        buffer.Deposit(i);
        std::cout << "product  produced " << i << '\n';
    }
}

int main() {
    std::thread thread_consume[5];
    std::thread thread_produce[5];

    Buffer<int> buffer(50);
    for (int i = 0; i < 5; ++i) {
        thread_consume[i] = std::thread(Consume<int>, std::ref(buffer));

        thread_produce[i] = std::thread(Produce<int>, std::ref(buffer));
    }

    for (auto& th_c : thread_consume) th_c.join();
    for (auto& th_p : thread_produce) th_p.join();
    return 0;
}
