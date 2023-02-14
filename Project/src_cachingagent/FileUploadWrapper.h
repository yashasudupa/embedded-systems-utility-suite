#ifndef FileUploadWrapper_h
#define FileUploadWrapper_h 1

#include <iostream>
#include <string>
#include <list>
#include <nlohmann/json.hpp>
#include <fstream>
#include <assert.h>
#include <dirent.h>
#include <thread>
#include <regex>

#include "Common.h"
#include "GlobalOperations.h"

class FileUploadWrapper
{
public:
	FileUploadWrapper( std::string appName, std::string gatewayId );
	~FileUploadWrapper();
	bool GetAndUploadFiles( std::string srcDirectory );
	short GetAndUploadFiles( std::string srcDirectory, nlohmann::json datajson);
	void RegisterCachedDataCB( std::function<void(nlohmann::json)> cb );
	short UploadFiles(std::string dataJson, std::string dataPath);
	void UploadFiles();
	
private:
	void FormatAndUploadJson( std::string fileNameWithPath, std::string fileName );
	std::string ValidateFileContent (std::string fileContent );
	void process_cached_files(std::string path, std::string srcDirectory, std::string filename);
	void no_of_cached_files(std::string jsonpath);

private:
	std::function<void(nlohmann::json)> m_cachedDataCB;
	std::string m_cloudAppName;
	std::string m_gatewayId;
	std::string m_deviceId;
	std::list<std::string> m_listTelemetryFiles;
	std::list<std::string> m_listAlertFiles;
	ExceptionLogger *m_exceptionLoggerObj;
	
	typedef std::string T_EPOCH;
	typedef tm T_TM;

	T_EPOCH calendar_epoch;
	T_TM calendar_tm;

	typedef	std::int32_t FILE_SIZE;
	FILE_SIZE no_of_files;
	
	typedef struct CALENDAR_DATABASE
	{
		T_EPOCH calendar_epoch;
		T_TM calendar_tm;
	}CAL_DB;
	
	std::string asset;
	
	std::vector<CAL_DB> cal_epoch_tm;
	void files_to_dynamic_array(std::string jsonpath, std::vector<CAL_DB> &dynamic_db_list);
};

#endif