#include "fan.hpp"

Fan::Fan() {
    std::cout << "Fan Object Created\n";
}

Fan::Fan(const Fan&) {
    std::cout << "Fan Copy Constructor Called\n";
}

void Fan::start() {
    std::cout << "Fan started\n";
}

void Fan::stop() {
    std::cout << "Fan stopped\n";
}
