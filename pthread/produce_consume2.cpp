#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include "concurrent_queue.h"
std::mutex mutex;
concurrent_queue<int> product;
const int kItemsToProduce = 100;
std::condition_variable cond_cqueue_empty;
int ConsumeItem(concurrent_queue<int> &product, int id) {  // NOLINT
    std::unique_lock<std::mutex> lock(mutex);
    cond_cqueue_empty.wait(lock, [&] { return !product.empty(); });
    std::cout << "cousmeItem" << id << ":  consume" << product.front()
              << std::endl;
    int item = product.pop();
    return item;
}

void ProduceItem(concurrent_queue<int> &product, int id) {  // NOLINT
    {
        std::lock_guard<std::mutex> lock(mutex);
        int i = rand();
        product.push(i);
        std::cout << "producer" << id << ":    produce" << i << std::endl;
    }
    cond_cqueue_empty.notify_one();
}

void ProducerTask() {
    for (int i = 1; i <= kItemsToProduce; ++i) {
        std::cout << "Produce the " << i << "^th item..." << std::endl;
        ProduceItem(product, i);
    }
}

void ConsumerTask() {
    static int cnt = 0;
    while (1) {
        int item = ConsumeItem(product, cnt + 1);
        std::cout << "Consume the " << item << "^th item" << std::endl;
        if (++cnt == kItemsToProduce) break;
    }
}
int main() {
    std::thread thread_produce;
    std::thread thread_consume;

    thread_produce = std::thread(ProducerTask);
    thread_consume = std::thread(ConsumerTask);

    thread_produce.join();
    thread_consume.join();

    return 0;
}
