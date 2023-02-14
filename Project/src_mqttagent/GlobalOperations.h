
#ifndef GlobalOperations_h
#define GlobalOperations_h 1

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <nlohmann/json.hpp>
#include <sys/time.h>
#include <unistd.h>

std::string GetLastToken ( std::string value, std::string seprator );
nlohmann::json ReadAndSetConfiguration (std::string fileName );
long GetProcessIdByName( std::string processName );
bool WriteConfiguration( std::string fileName, nlohmann::json contentObj );
std::string GetTimeStamp();

class GlobalOperations
{

};

#endif