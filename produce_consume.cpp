#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

template <class Data>
class Buffer {
public:
    explicit Buffer(int capacity)
        : m_capacity(capacity), m_count(0), m_front(0), m_rear(0) {
        buffer = new Data[capacity];
    }

    ~Buffer() { delete[] buffer; }
    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    void deposit(const Data &data) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_notfull.wait(lock, [this] { return m_count < m_capacity; });
            buffer[m_rear++] = data;
            m_rear           = (m_rear) % m_capacity;
            ++m_count;
        }
        m_notempty.notify_one();
    }

    void fetch(Data &data) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_notempty.wait(lock, [this] { return m_count > 0; });
            data    = std::move(buffer[m_front++]);
            m_front = m_front % m_capacity;
            --m_count;
        }
        m_notfull.notify_one();
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_notfull;
    std::condition_variable m_notempty;
    Data *buffer;
    int m_capacity;
    int m_count;
    int m_front;
    int m_rear;
};
template <class Data>
void consume(int id, Buffer<Data> &buffer) {  // NOLINT
    for (int i = 0; i < 50; ++i) {
        Data data;
        buffer.fetch(data);
        std::cout << "consume " << id << " fetched " << data << '\n';
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}
template <class Data>
void produce(int id, Buffer<Data> &buffer) {  // NOLINT
    for (int i = 0; i < 75; i++) {
        buffer.deposit(i);
        std::cout << "product " << id << " produced " << i << '\n';
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    std::thread thread_produce[5];
    std::thread thread_consume[5];

    Buffer<int> buffer(200);

    for (int i = 0; i < 5; ++i) {
        thread_produce[i] = std::thread(produce<int>, i, std::ref(buffer));
        thread_consume[i] = std::thread(consume<int>, i, std::ref(buffer));
    }

    for (auto &th_c : thread_consume) th_c.join();
    for (auto &th_p : thread_produce) th_p.join();

    return 0;
}
