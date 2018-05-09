#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx;
void PrintEven(int x) {
    if (x % 2 == 0) std::cout << x << " is even!" << std::endl;
}
void PrintThreadId(int id) {
    std::lock_guard<std::mutex> lkg(mtx);
    PrintEven(id);
}

int main() {
    std::thread thread[10];
    for (int i = 0; i < 10; ++i) {
        thread[i] = std::thread(PrintThreadId, i + 1);
    }

    for (auto &th : thread) th.detach();

    return 0;
}
