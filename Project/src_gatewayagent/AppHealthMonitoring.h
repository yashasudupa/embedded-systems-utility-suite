#ifndef AppHealthMonitoring_h
#define AppHealthMonitoring_h 1

#include <iostream>
#include <stdlib.h>
#include <string>
#include <thread>
#include <string.h>
#include <functional>
#include <nlohmann/json.hpp>
#include <sys/statvfs.h>

#include "Common.h"
#include "GlobalOperations.h"
#include "SendMessageToCloudWrapper.h"
#include "ExceptionLogger.h"

typedef struct AppHealth
{
	std::string app_cpu_usage;
	std::string app_ram_usage;
} APPHEALTH;

class AppHealthMonitoring
{
public:
	AppHealthMonitoring( std::string processName );
	~AppHealthMonitoring();
	void StartHealthInfoGetterThread();
	void RegisterApp( std::string processname );
	void DeRegisterApp( std::string processname );
	
private:
	std::string GetRamUsage( std::string processname );
	std::string GetCpuUsage( std::string processname );
	int GetDiskFreeSpace();

private:
	std::map<std::string,APPHEALTH*> m_appHealthMap;
	std::string m_prviousDate;
	std::thread m_threadObj;
	bool m_appHealthMoniThreadFlag;
	
	SendMessageToCloudWrapper *m_sendMsgToCloudObj;
	ExceptionLogger *m_exceptionLoggerObj;
};

#endif