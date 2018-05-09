#include "concurrent_queue.h"
#include <iostream>
#include <string>
#include <thread>
#include <utility>
class animal {
public:
    animal() {}
    animal(std::string n) : name(n) {}
    ~animal() {}

private:
    std::string name;
};
int main() {
    concurrent_queue<animal> animal_queue;
    animal pig("pig");

    animal_queue.push(pig);

    animal fox("fox");
    animal_queue.push(std::move(fox));

    return 0;
}
