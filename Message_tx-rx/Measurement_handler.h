#include <iostream>
#include <cstdint>
#include <functional>
#include <vector>
#include <unordered_map>

class measurement_handler{

    public:
    struct weatherdata
    {
        uint32_t last_update;
        float    pressure;
        float    temperature;
        float    humidity;
    };

  private:

  public:
    measurement_handler();
    ~measurement_handler();

    bool handlerfunc (uint8_t *rawdata, size_t len);

    struct weatherdata get_actual_data() const;
};