#ifndef FAN_HPP
#define FAN_HPP

#include "device_interface.hpp"
#include <iostream>

class Fan : public IDevice {
public:
    Fan();
    Fan(const Fan&);
    void start() override;
    void stop() override;
};