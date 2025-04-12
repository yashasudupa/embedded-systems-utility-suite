
#ifndef GlobalOperations_h
#define GlobalOperations_h 1

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <nlohmann/json.hpp>
#include <sys/time.h>
#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include <thread>

std::string GetLastToken ( std::string value, std::string seprator );
nlohmann::json ReadAndSetConfiguration (std::string fileName );
long GetProcessIdByName( std::string processName );
bool WriteConfiguration( std::string fileName, nlohmann::json contentObj );
std::string GetTimeStamp();



class GlobalOperations
{
	public:
		//std::mutex mtx;
		static std::condition_variable cv;
		//static bool ready;
		
		//struct mq_attr attr, old_attr;   // To store queue attributes
		//struct sigevent sigevent;        // For notification
		//mqd_t mqdes, mqdes2;             // Message queue descriptors
		
		//unsigned int prio;               // Priority 
};

#endif