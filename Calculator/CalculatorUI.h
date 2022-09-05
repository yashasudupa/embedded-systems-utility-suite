#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <math.h>
#include <ctime>
#include <functional>
#include <thread>
#include <regex>
#include <mutex>

class CalculatorUI
{
    private:
        std::string InputString;

    public:
        CalculatorUI();
        ~CalculatorUI();
        static void GoOnline(void);
        static void GoOffline(void);
};