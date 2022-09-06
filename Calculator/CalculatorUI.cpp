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
        std::cout << "CalculatorUI::BracketsExtraction m.str() : " << parsed_string << std::endl;
        std::smatch m;
        if(std::regex_search(InputString, m, reg_a))
        {
            std::smatch temp_m;
            parsed_string = m.str();
            std::cout << "CalculatorUI::BracketsExtraction parsed_string : " << parsed_string.c_str() << std::endl;
            return 1;
        }
        std::cout << "CalculatorUI::BracketsExtraction m.str() : " << m.str() << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return -1;
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
            std::cout << "CalculatorUI::OperatorExtraction op : " << op << std::endl;
            return 1;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }  
    return -1;
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

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }  
    return "";
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

            std::cout << "CalculatorUI::ArgumentsExtraction m.str() : " << m.str() << std::endl;
            std::regex reg_first_arg ("(((.*)[+-*%/])+");
            
            a = std::stoi(temp_m.str());
            
            std::regex reg_second_arg ("[+-*%/0-9]+");
            if(!std::regex_search(parsed_string, temp_m, reg_second_arg))
            {
                std::cout << "CalculatorUI::ArgumentsExtraction temp_m.str() : " << temp_m.str() << std::endl;
                return -3;
            }
            b = std::stoi(temp_m.str());

            std::cout << "a : " << std::to_string(a) << std::endl;
            std::cout << "b : " << std::to_string(b) << std::endl;

            return 1;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }        
    return -1;
}

std::string CalculatorUI::InititateCalculatorApp(std::string &ipString)
{

    try
    {
        std::string op;
        std::string result;
        std::int64_t status;
        while(1)
        {
            std::string parsed_string;
            std::regex reg_bracket_1 ("\\((.*)\\)+");
            std::regex reg_bracket_2 ("[{0-9^(.*?)0-9}]+");
            std::regex reg_bracket_3 ("[[0-9^(.*?)0-9]]+");
            std::regex reg_a ("[0-9^(.*?)0-9]+");

            std::regex buffer;
            std::smatch m;
            if(BracketsExtraction(reg_bracket_1, parsed_string) == 1)
            {
                std::cout << "reg_bracket_1:m.str() - " << parsed_string << std::endl;
                buffer = reg_bracket_1;
            }
            else if(BracketsExtraction(reg_bracket_2, parsed_string) == 1)
            {
                std::cout << "reg_bracket_2:m.str() - " << parsed_string << std::endl;
                buffer = reg_bracket_2;
            }
            else if(BracketsExtraction(reg_bracket_3, parsed_string) == 1)
            {
                std::cout << "reg_bracket_3:m.str() - " << parsed_string << std::endl;
                buffer = reg_bracket_3;
            }

            if (!parsed_string.empty())
            {
                status = ArgumentsExtraction(first_argument, second_argument, buffer);
                if (status != 1) { std::cout << "Invalid input" << std::endl; return "INVALID";}

                status = OperatorExtraction(parsed_string, op);
                if (status != 1) { std::cout << "Invalid input" << std::endl; return "INVALID";}

                result = CalculatorOperation(first_argument, second_argument, op);
                std::regex_replace(InputString, buffer, result);
            }
            else
            {
                ArgumentsExtraction(first_argument, second_argument, reg_a);
                if (status != 1) { std::cout << "Invalid input" << std::endl; return "INVALID";}

                OperatorExtraction(InputString, op);
                if (status != 1) { std::cout << "Invalid input" << std::endl; return "INVALID";}

                result = CalculatorOperation(first_argument, second_argument, op);   
                std::regex_replace(InputString, buffer, result);
            }   
            if(result.empty())
            {
                return OutputString;
            }

            //OutputString = InititateCalculatorApp(InputString);             
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return "";
}

void CalculatorUI::GoOnline(void)
    {

        try
        {
            while(1)
            {
                std::cin >> InputString;        
                InititateCalculatorApp(InputString);
                std::cout << " Enter the input" << std::endl;
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
