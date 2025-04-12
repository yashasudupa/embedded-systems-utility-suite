#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "device_interface.hpp"
#include <iostream>

class Light : public IDevice {
public:
    void start() override;
    void stop() override;
};

#endif // LIGHT_HPP