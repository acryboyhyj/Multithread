#include <cstdlib>
#include <iostream>
#include <queue>

#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex;
void* consume(void* ptr) {
    std::queue<int>* product = (std::queue<int>*)ptr;
    for (int i = 0; i < 10; ++i) {
        pthread_mutex_lock(&mutex);
        if (product->empty()) {
            pthread_mutex_unlock(&mutex);
            continue;
        }
        std::cout << product->front() << std::endl;
        product->pop();
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    return (void*)0;
}

void* produce(void* ptr) {
    std::queue<int>* product = (std::queue<int>*)ptr;

    for (int i = 0; i < 10; ++i) {
        pthread_mutex_lock(&mutex);
        product->push(i);
        pthread_mutex_unlock(&mutex);
    }
    return (void*)0;
}

int main(int argc, char* argv[]) {
    pthread_mutex_init(&mutex, NULL);
    std::queue<int> production;
    std::queue<int>* p = &production;
    pthread_t tid1;
    pthread_t tid2;

    pthread_create(&tid1, NULL, consume, (void*)p);
    pthread_create(&tid2, NULL, produce, (void*)p);

    void* retval;
    pthread_join(tid1, &retval);
    pthread_join(tid2, &retval);
    return 0;
}
