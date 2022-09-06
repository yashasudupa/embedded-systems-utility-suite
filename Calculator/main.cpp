#include "CalculatorUI.h"

int main (int argc, char *argv[])
{

    try
    {
        CalculatorUI *m_CalculatorUI = new CalculatorUI();
        if(m_CalculatorUI)
        {
            m_CalculatorUI->GoOnline();
        }
    }
    catch( ... )
    {
        std::cout << "main : Unknown exception occurred" << std::endl;
    }
}