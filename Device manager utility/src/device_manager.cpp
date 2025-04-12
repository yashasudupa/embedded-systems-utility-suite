#include "device_manager.hpp"

DeviceManager* DeviceManager::instance = nullptr;
std::mutex DeviceManager::mutex_lock;

DeviceManager* DeviceManager::getInstance() {
    std::lock_guard<std::mutex> lock(mutex_lock);
    if (!instance)
        instance = new DeviceManager();
    return instance;
}

void DeviceManager::addDevice(int id, std::shared_ptr<IDevice> device) {
    devices[id] = device;
}

void DeviceManager::startAllDevices() const {
    for (const auto& pair : devices)
        pair.second->start();
}

void DeviceManager::stopAllDevices() const {
    for (const auto& pair : devices)
        pair.second->stop();
}
