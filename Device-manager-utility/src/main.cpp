#include "device_manager.hpp"
#include "fan.hpp"
#include "light.hpp"
#include <condition_variable>
#include <thread>
#include <vector>
#include <type_traits>
#include <stdexcept>

// Protected Inheritance
class ProtectedDevice : protected IDevice {
protected:
    void start() override {
        std::cout << "Protected Device started (Restricted Access)\n";
    }

    void stop() override {
        std::cout << "Protected Device stopped\n";
    }
};

static_assert(std::is_same<decltype(DeviceManager::getInstance()), DeviceManager* (*)()>::value,
              "DeviceManager::getInstance must return a Singleton pointer");

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void workerThread() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [] { return ready; });
    std::cout << "Worker thread processing...\n";
}

inline int square(int num) {
    return num * num;
}

constexpr int cube(int num) {
    return num * num * num;
}

void typeCastingDemo(IDevice* device) {
    Fan* fanPtr = dynamic_cast<Fan*>(device);
    if (fanPtr) {
        std::cout << "Dynamic Cast Successful: Fan Detected\n";
    } else {
        std::cout << "Dynamic Cast Failed\n";
    }

    void* voidPtr = static_cast<void*>(device);
    IDevice* devicePtr = reinterpret_cast<IDevice*>(voidPtr);
    devicePtr->start();
}

void exceptionDemo() {
    try {
        throw std::runtime_error("Custom Exception: Device Error");
    } catch (const std::exception& e) {
        std::cout << "Exception Caught: " << e.what() << "\n";
    }
}

void containerDemo() {
    std::vector<int> numbers = {10, 20, 30, 40};

    std::cout << "Using Iterator: ";
    for (std::vector<int>::iterator it = numbers.begin(); it != numbers.end(); ++it)
        std::cout << *it << " ";
    std::cout << "\n";

    std::cout << "Using Range-based For Loop: ";
    for (const auto& num : numbers)
        std::cout << num << " ";
    std::cout << "\n";
}

int main() {
    DeviceManager* manager = DeviceManager::getInstance();

    std::shared_ptr<IDevice> fan = std::make_shared<Fan>();
    std::shared_ptr<IDevice> light = std::make_shared<Light>();

    manager->addDevice(1, fan);
    manager->addDevice(2, light);

    manager->startAllDevices();
    manager->stopAllDevices();

    std::cout << square(5) << "\n";
    std::cout << cube(3) << "\n";

    std::thread worker(workerThread);
    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
    }
    cv.notify_one();
    worker.join();

    typeCastingDemo(fan.get());
    exceptionDemo();
    containerDemo();

    return 0;
}
