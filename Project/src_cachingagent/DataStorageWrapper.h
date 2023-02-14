#ifndef DataStorageWrapper_h
#define DataStorageWrapper_h 1

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <regex>

#include "Common.h"
#include "GlobalOperations.h"


typedef struct alertTelemetryStorageStruct
{
	long fileCount;
	nlohmann::json jsonObj;
	std::string fileName;
	
} CACHED_ALERT_TELEMETRY_STRUCT;

/*typedef struct alertStorageStruct
{
	std::fstream cachedAlertFile;
	long fileAlertCount;
	nlohmann::json jsonAlertObj;
} CACHED_ALERT_STRUCT;*/

class DataStorageWrapper
{
public:

	DataStorageWrapper( std::string gatewayId, std::string cloudAppName );
	~DataStorageWrapper();
	void ExecuteCommand( nlohmann::json dataJson );
	bool FormatAndStoreAlert( nlohmann::json dataJson );
	bool FormatAndStoreTelemetry( nlohmann::json dataJson );
	bool FormatAndStoreAlertBackup( nlohmann::json dataJson );
	bool FormatAndStoreTelemetryBackup( nlohmann::json dataJson );
	bool StopFileWriting();
	void ExecuteBackup( nlohmann::json dataJson );
	bool restart_check( nlohmann::json dataJson, std::string dataPath );
	
private:
	
private:
	/*std::fstream m_cachedTelemetryFile;
	std::fstream m_cachedAlertFile;
	long m_fileTelemetryCount;
	long m_fileAlertCount;*/
	
    std::string m_cloudAppName;
	std::map<std::string, CACHED_ALERT_TELEMETRY_STRUCT*> m_cachedTelemetryMap;
	std::map<std::string, CACHED_ALERT_TELEMETRY_STRUCT*> m_cachedAlertMap;
	std::map<std::string, CACHED_ALERT_TELEMETRY_STRUCT*> m_backedupTelemetryMap;
	std::map<std::string, CACHED_ALERT_TELEMETRY_STRUCT*> m_backedupAlertMap;
	ExceptionLogger *m_exceptionLoggerObj;
	std::string m_gatewayId;
	std::string t_of_day_str;
	struct tm start_time;
	
	typedef std::string T_EPOCH;
	typedef tm T_TM;

	T_EPOCH calendar_epoch;
	T_TM calendar_tm;

	typedef struct CALENDAR_DATABASE
	{
		T_EPOCH calendar_epoch;
		T_TM calendar_tm;
	}CAL_DB;
	
	std::string asset;

};

#endif