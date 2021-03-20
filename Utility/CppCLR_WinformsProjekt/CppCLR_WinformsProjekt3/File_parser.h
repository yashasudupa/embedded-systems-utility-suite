#pragma once
/*
* File_parser.h - File parser
*/

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <string>
#include <map>
#include <unordered_map>
#include <chrono>

#define LENGTH 50

namespace fparser
{   
    using namespace System::ComponentModel;
    using namespace System::ComponentModel;

    typedef std::vector <std::pair<std::string, int> > PairCounter;

    int parser(const std::string&, PairCounter &, BackgroundWorker^);
};

