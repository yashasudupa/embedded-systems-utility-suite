#ifndef DEVICE_INTERFACE_HPP
#define DEVICE_INTERFACE_HPP

class IDevice {
public:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual ~IDevice() {}
};

#endif // DEVICE_INTERFACE_HPP