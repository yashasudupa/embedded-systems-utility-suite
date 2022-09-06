#include "CalculatorUI.h"


CalculatorUI::CalculatorUI()
    {
        InputString = "0";
        result = 0;
        std::cout << InputString << std::endl;
        first_argument = 0;
        second_argument = 0;
    }

CalculatorUI::~CalculatorUI()
    {
    
    }


std::int8_t CalculatorUI::BracketsExtraction(std::regex reg_a, std::string &parsed_string)
{

    try
    {
        std::smatch m;
        if(std::regex_search(InputString, m, reg_a))
        {
            std::smatch temp_m;
            std::string parsed_string = m.str();
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

}


std::int8_t CalculatorUI::OperatorExtraction(std::string parsed_string, std::string &op)
{
    try
    {
        std::smatch m;
        std::regex reg_a("[0-9^(.*?)0-9]");
        if(std::regex_search(parsed_string, m, reg_a))
        {
            std::smatch temp_m;
            op = m.str();
            return 1;
        }
        return -1;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }  
}

//Module comments

std::string CalculatorUI::CalculatorOperation(std::double_t &a, std::double_t &b, std::string op)
{
    try
    {
        double result;
        if (op == "/") { result = a/b; return std::to_string(result);};
        if (op == "*") { result = a*b; return std::to_string(result);};
        //if (op == "%") { result = a%b; return std::to_string(result);};
        if (op == "+") { result = a+b; return std::to_string(result);};
        if (op == "-") { result = a-b; return std::to_string(result);};

        return "";
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }  
}

std::int8_t CalculatorUI::ArgumentsExtraction(std::double_t &a, std::double_t &b, std::regex reg_a)
{
    try
    {
        std::smatch m;
        if(std::regex_search(InputString, m, reg_a))
        {
            std::smatch temp_m;
            std::string parsed_string = m.str();

            std::regex reg_first_arg ("[0-9+-*%/]+");
            std::regex_search(parsed_string, temp_m, reg_first_arg);
            a = std::stoi(temp_m.str());
            
            std::regex reg_second_arg ("[+-*%/0-9]+");
            std::regex_search(parsed_string, temp_m, reg_second_arg);
            b = std::stoi(temp_m.str());
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }        
}

std::string CalculatorUI::InititateCalculatorApp(std::string &ipString)
{

    try
    {
        std::string op;
        std::string result;
        //while(1)
        //{
            

            std::string parsed_string;
            std::regex reg_bracket_1 ("[(0-9^(.*?)0-9)]+");
            std::regex reg_bracket_2 ("[{0-9^(.*?)0-9}]+");
            std::regex reg_bracket_3 ("[[0-9^(.*?)0-9]]+");
            std::regex reg_a ("[0-9^(.*?)0-9]+");

            std::regex buffer;
            std::smatch m;
            if(std::regex_search(InputString, m, reg_bracket_1))
            {
                parsed_string = m.str();
                buffer = reg_bracket_1;
            }
            if(std::regex_search(InputString, m, reg_bracket_1))
            {
                parsed_string = m.str();
                buffer = reg_bracket_2;
            }
            if(std::regex_search(InputString, m, reg_bracket_1))
            {
                parsed_string = m.str();
                buffer = reg_bracket_3;
            }

            if (!parsed_string.empty())
            {
                ArgumentsExtraction(first_argument, second_argument, reg_a);
                OperatorExtraction(parsed_string, op);
                result = CalculatorOperation(first_argument, second_argument, op);
                std::regex_replace(InputString, buffer, result);
            }
            else
            {
                ArgumentsExtraction(first_argument, second_argument, reg_a);
                OperatorExtraction(InputString, op);
                result = CalculatorOperation(first_argument, second_argument, op);
                std::regex_replace(InputString, buffer, result);
            }   
            if(result.empty())
            {
                return OutputString;
            }

            OutputString = InititateCalculatorApp(InputString);             
        //}
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

}

void CalculatorUI::GoOnline(void)
    {

        try
        {
            while(1)
            {
                std::cin >> InputString;        
                InititateCalculatorApp(InputString);
            }
            

            /* code */

            //1st priority : Bracket (), {}, []
            //2nd priroty : Multiplication / Division / %
            //3rd priority : + -

            

        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
    }
