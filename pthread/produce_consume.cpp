#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

std::mutex mutex;
std::condition_variable condition_variable;

void consume(std::queue<int> &product, int id) {  // NOLINT
    std::unique_lock<std::mutex> lock(mutex);
    condition_variable.wait(lock, [&] { return !product.empty(); });
    std::cout << "consumer" << id << ":  consume" << product.front()
              << std::endl;
    product.pop();
}

void produce(std::queue<int> &product, int id) {  // NOLINT
    {
        std::lock_guard<std::mutex> lgk(mutex);
        std::cout << "producer" << id << ":" << std::endl;
        for (int i = 0; i < 3; ++i) {
            product.push(i);
        }
    }
    condition_variable.notify_one();
}

int main() {
    std::thread thread_produce[5];
    std::thread thread_consume[5];

    std::queue<int> product;
    for (int i = 0; i < 5; ++i) {
        thread_produce[i] = std::thread(produce, std::ref(product), i + 1);
        thread_consume[i] = std::thread(consume, std::ref(product), i + 1);
    }

    for (auto &th_c : thread_consume) th_c.join();
    for (auto &th_p : thread_produce) th_p.join();

    return 0;
}
