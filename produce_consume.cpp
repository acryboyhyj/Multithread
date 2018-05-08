#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
std::mutex mutex;
std::condition_variable condition_variable;
void consume(std::queue<int> &product) {  // NOLINT
    std::unique_lock<std::mutex> lock(mutex);
    condition_variable.wait(lock, [&] { return !product.empty(); });
    while (!product.empty()) {
        std::cout << product.front() << std::endl;
        product.pop();
    }
}

void produce(std::queue<int> &product) {  // NOLINT
    std::lock_guard<std::mutex> lgk(mutex);
    for (int i = 0; i < 5; ++i) {
        product.push(i);
    }
}

int main() {
    std::thread thread_produce[5];
    std::thread thread_consume[5];

    std::queue<int> product;
    for (int i = 0; i < 5; ++i) {
        thread_produce[i] = std::thread(produce, std::ref(product));
        thread_consume[i] = std::thread(consume, std::ref(product));
    }

    for (auto &th_c : thread_consume) th_c.join();
    for (auto &th_p : thread_produce) th_p.join();

    return 0;
}
