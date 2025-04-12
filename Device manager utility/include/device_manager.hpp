#ifndef DEVICE_MANAGER_HPP
#define DEVICE_MANAGER_HPP

#include "device_interface.hpp"
#include <map>
#include <memory>
#include <mutex>

class DeviceManager {
private:
    static DeviceManager* instance;
    static std::mutex mutex_lock;
    std::map<int, std::shared_ptr<IDevice>> devices;

    DeviceManager() {}

public:
    static DeviceManager* getInstance();
    void addDevice(int id, std::shared_ptr<IDevice> device);
    void startAllDevices() const;
    void stopAllDevices() const;
};