#include <iostream>
#include <cstdint>
#include <functional>
#include <vector>
#include <unordered_map>
#include "dispatcher.h"
#include "Measurement_handler.h"


int main (int argc, char *argv[])
{
    dispatcher dispatcher;

    uint8_t data[17] = {0}; 

    data[0] = 0x00;
    data[1] = 0x00;
    data[2] = 0x00;
    data[3] = 0x00;

    data[4] = 0x00;
    data[5] = 0x7f;


    data[6] = 0x00;
    data[7] = 0x01;
    data[8] = 0x00;
    data[9] = 0x01;
    data[10] = 0x00;
    data[11] = 0x01;
    data[12] = 0x00;
    data[13] = 0x01;

    dispatcher.dispatch(data, 36);
}
