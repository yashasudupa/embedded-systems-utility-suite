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
        std::string OutputString;
        std::int64_t result;
        std::double_t first_argument;
        std::double_t second_argument;

    public:
        CalculatorUI();
        ~CalculatorUI();
        void GoOnline(void);
        std::int8_t ArgumentsExtraction(std::double_t &a, std::double_t &b, std::regex reg_a);
        std::int8_t OperatorExtraction(std::string parsed_string, std::string &op);
        std::int8_t BracketsExtraction(std::regex reg_a, std::string &parsed_string);
        std::string CalculatorOperation(std::double_t &a, std::double_t &b, std::string op);
        std::string InititateCalculatorApp(std::string &ipString);
        void GoOffline(void);
};