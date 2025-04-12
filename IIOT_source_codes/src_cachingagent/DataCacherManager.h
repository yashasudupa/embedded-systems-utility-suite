#ifndef DataCacherManager_h
#define DataCacherManager_h 1

#include <iostream>
#include <nlohmann/json.hpp>
#include "Common.h"
#include "LocalBrokerCommunicationManager.h"
#include "DataStorageWrapper.h"
#include "FileUploadWrapper.h"
#include <ctime>
#include <map>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <regex>
#include <string>
#include <sstream>
#include <bits/stdc++.h>

class DataCacherManager
{
public:
	DataCacherManager( std::string gatewayId, std::string cloudAppName );
	~DataCacherManager();
	void ReceiveSubscribedData( std::string data );
	void ReceiveCachedData( nlohmann::json jsonObj );
	
private:
	void InitDataCacherManager();
	bool one_month_databackup(nlohmann::json dataJson);
	
private:
	std::string m_gatewayId;
	std::string m_deviceId;
	std::string m_cloudAppName;
	//std::map<int, int> month_and_date_database;
	
	std::mutex mtx;

	typedef std::string T_EPOCH;
	typedef tm T_TM;

	T_EPOCH calendar_epoch;
	T_TM calendar_tm;

	typedef struct CALENDAR_DATABASE
	{
		T_EPOCH calendar_epoch;
		T_TM calendar_tm;
	}CAL_DB;

	typedef	std::int32_t FILE_SIZE;
	FILE_SIZE files_list;

	std::vector<CAL_DB> cal_epoch_tm;
	
	//Testing
	int time_current;
	std::string asset;
	
	LocalBrokerCommunicationManager *m_localBrokerCommObj;
	DataStorageWrapper *m_dataStorageWrapperObj;
	FileUploadWrapper *m_fileUploadWrapperObj;
	ExceptionLogger *m_exceptionLoggerObj;
	void rm_oldfile_from_database(CAL_DB &dynamic_db, std::string jsonpath);
	short backup_procedure(std::vector<CAL_DB> &dynamic_db, std::string jsonpath);
	void DoDataBackup(void);
	void first_json_from_files(std::string jsonpath, std::vector<CAL_DB> &json_to_dynamic_list);
};

#endif