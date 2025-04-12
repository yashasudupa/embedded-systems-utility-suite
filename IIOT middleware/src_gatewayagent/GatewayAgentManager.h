#ifndef GatewayAgentManager_h
#define GatewayAgentManager_h 1

#include <functional>
#include <string>
#include <iostream>
#include <math.h>
#include <nlohmann/json.hpp>

#include "CloudCommunicationWrapper.h"
#include "PackageManager.h"
#include "LocalBrokerCommunicationManager.h"
#include "SendMessageToCloudWrapper.h"
#include "ExceptionLogger.h"
#include "WatchDog.h"
#include "LogFileCleanUp.h"
#include <ctime>

class GatewayAgentManager
{
public:
	GatewayAgentManager( std::string processName );
	~GatewayAgentManager();
	void DataRecevier( nlohmann::json jsonObject );
	void WatchdogCommandReceiver( nlohmann::json jsonObject );
	void ConnectionStatusReceiver( nlohmann::json jsonObject  );
	void ExecuteCloudCommand( nlohmann::json jsonObject );
	void ReceiveDeviceData( std::string  ); //add name
	void ReceiveDeviceDataForBackup( std::string  ); //add name
	void BluenrgMonitor(time_t* time_monitor);

	typedef struct DATE_MONTH_AND_TIME_DATABASE
	{
		std::string month;
		std::uint16_t date;
		std::uint16_t hours;
		std::uint16_t minutes;
		std::uint16_t seconds;
	}DATE_MONTH_AND_TIME_DATABASE;
	
	// Extract time database of start
	
	DATE_MONTH_AND_TIME_DATABASE db1;
	DATE_MONTH_AND_TIME_DATABASE db2;
	
	void ParseTimeInUTC(time_t t, DATE_MONTH_AND_TIME_DATABASE &db);
	
private:
	bool UploadLogFiles();
	void InitGatewayAgentManager();
	ResponseStruct ValidateJson( nlohmann::json jsonObj, int caseId );
	void SendNotificationToCloud( ResponseStruct responseStructObj, std::string subJobId = "" );
	bool CheckVersionChange();
private:

	CloudCommunicationWrapper *m_cloudConnectionObj;
	PackageManager *m_packageManagerObj;
	LocalBrokerCommunicationManager *m_localBrockerObj;
	SendMessageToCloudWrapper *m_sendMsgToCloudObj;
	WatchDog *m_watchDogObj;
	LogFileCleanUp *m_logCleanUpObj;
	
	std::string m_gatewayId;
	std::string m_cloudAppName;
	std::string m_processName;
	
	bool m_updateGatewayFlag;
    bool m_networkFailFlag;
	
	nlohmann::json m_configJson;
	nlohmann::json m_devicesRegisterJson;
	nlohmann::json m_networkFailJson;
	ExceptionLogger *m_exceptionLoggerObj; 
	
	std::string m_configFileName;
	std::string m_deviceFileName;
};

#endif