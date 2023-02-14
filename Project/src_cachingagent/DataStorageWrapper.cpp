#include "DataStorageWrapper.h"

DataStorageWrapper::DataStorageWrapper( std::string gatewayId, std::string cloudAppName ):
	m_gatewayId( gatewayId ),
    m_cloudAppName( cloudAppName )
{
	m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
}

DataStorageWrapper::~DataStorageWrapper()
{
	
}

bool DataStorageWrapper::restart_check( nlohmann::json dataJson, std::string dataPath )
{	
	
	//fmgnmt.txt file is opened
	std::vector<CAL_DB> dynamic_db_list;
	//Open the file
	std::string filename = dataPath + "fmgnmt.txt";
	std::ifstream in_file(filename, std::ios::binary);
	std::string line;
	short no_of_lines = 0;
	
	std::regex r_e("_[0-9.]+");
	std::smatch m;

	time_t curr_time;
	
	while(std::getline(in_file, line, '\n'))
	{
		std::cout << "line : " << line << std::endl;
		regex_search(line, m, r_e);

		std::string time_epoch = m.str();

		std::cout << "m.str() : " << time_epoch << std::endl;
		
		//Check if it is a first file
		if(time_epoch.empty())
		{
			std::cout << "time_epoch.empty()" << std::endl;
			continue;
		}

		//Extract asset information
		if(asset.empty() && line != "fmgnmt.txt")
		{
			//Legacy_Asset_Rel11_1661498878.json
			std::regex r_a("^(.*?)_[0-9]+");
			regex_search(line, m, r_a);
			asset = m.str();
			std::cout << "asset m.str() : " << asset << std::endl;
		}

		//Extract epoch information
		std::string trimmed_time_epoch =  time_epoch.substr(1, time_epoch.find(".") - 1);
		std::cout << "trimmed_time_epoch : " << trimmed_time_epoch.c_str() << std::endl;
		curr_time = std::stoi(trimmed_time_epoch, nullptr, 10);

		tm *tm_gmt = gmtime(&curr_time);

		CAL_DB dynamic_db;
		dynamic_db.calendar_epoch = trimmed_time_epoch;
		dynamic_db.calendar_tm = *tm_gmt;
		
		dynamic_db_list.push_back(dynamic_db);
		
		no_of_lines++;
	}
	
	//
	if(this->t_of_day_str.empty() && no_of_lines == 0)
	{
		
		std::time_t curr_time = std::time(nullptr);
		start_time = *localtime(&curr_time);
		return true;
	}
	
	if(this->t_of_day_str.empty() && no_of_lines != 0)
	{
		auto it = dynamic_db_list.end() - 1;
		std::string deviceId = dataJson[DEVICE_ID];
		CACHED_ALERT_TELEMETRY_STRUCT* backedupTelemetryObj;
		
		backedupTelemetryObj = new CACHED_ALERT_TELEMETRY_STRUCT;
			
		//TODO:
		//unique_pointer
		
		backedupTelemetryObj->jsonObj[DEVICE_ID] = deviceId;
		//backedupTelemetryObj->jsonObj["app"] = m_cloudAppName;
		#ifdef SIEMENS_GATEWAY
			backedupTelemetryObj->jsonObj[TYPE] = dataJson[TYPE];
		#else
			backedupTelemetryObj->jsonObj[TYPE] = dataPath;
		#endif
		
		m_backedupTelemetryMap[deviceId] = backedupTelemetryObj;
		backedupTelemetryObj->fileName = DB_TELEMERTY_PATH + deviceId + "_" + it->calendar_epoch + ".json";
		//backedupTelemetryObj->fileName = DB_TELEMERTY_PATH + deviceId + "_" + GetTimeStampInEpoch() + ".json";	
	}
}

void DataStorageWrapper::ExecuteBackup( nlohmann::json dataJson )
{
	std::stringstream logg;
	logg.str("");
	logg << "DataStorageWrapper::ExecuteBackup  tempJson : " << dataJson << std::endl;
	m_exceptionLoggerObj->LogDebug( logg.str());
	//validate JSON return structure try catch
	
    nlohmann::json tempJson = dataJson[COMMAND_INFO];
    
    if( !tempJson.contains( COMMAND_SCHEMA ) )
    {
        logg.str("");
        logg << "DataStorageWrapper::FormatAndStoreTelemetry  tempJson : " << tempJson << ",  Message : 'commandschema' not found in respected tempJson";
        m_exceptionLoggerObj->LogError( logg.str() );
        return ;
    }
    
	std::string commandSchema = dataJson[COMMAND_INFO][COMMAND_SCHEMA];
	
    logg.str("");
    logg << "DataStorageWrapper::ExecuteCommand()  dataJson : " << dataJson << std::endl;
    m_exceptionLoggerObj->LogDebug( logg.str() );
    
	if( commandSchema == "telemetry" )
	{
		//restart_check(dataJson, DB_TELEMERTY_PATH);
		FormatAndStoreTelemetryBackup( dataJson );
	}
	if( commandSchema == "alert" )
	{
		FormatAndStoreAlertBackup( dataJson );
	}
	logg.str("");
	logg << "DataStorageWrapper::ExecuteBackup ***** Telemetry is stored in Dataackup ******** : " << tempJson << std::endl;
	m_exceptionLoggerObj->LogDebug( logg.str());
	
	std::cout << "***** Telemetry is stored in Dataackup ********" << std::endl;
}

bool DataStorageWrapper::FormatAndStoreTelemetryBackup( nlohmann::json dataJson )
{
	std::stringstream logg;
	try
	{
		if( !dataJson.contains( DEVICE_ID ) )
		{
			logg.str("");
			logg << "DataStorageWrapper::FormatAndStoreTelemetryBackup  GatewayID : " << m_gatewayId << ",  Message : 'device_id' not found in respected json";
			m_exceptionLoggerObj->LogError( logg.str() );
			return false;
		}
		
		/*
		#ifndef SIEMENS_GATEWAY
			if( !dataJson.contains( DEVICE_ID ) )
			{
				logg.str("");
				logg << "DataStorageWrapper::FormatAndStoreTelemetryBackup  GatewayID : " << m_gatewayId << ",  Message : 'data' not found in respected json";
				m_exceptionLoggerObj->LogError( logg.str() );
				return false;
			}
		#endif
		
		 */
		 
		std::string deviceId = dataJson[DEVICE_ID];
		auto it = m_backedupTelemetryMap.find( deviceId );
		CACHED_ALERT_TELEMETRY_STRUCT* backedupTelemetryObj;
		
		if( it == m_backedupTelemetryMap.end() )
		{
			backedupTelemetryObj = new CACHED_ALERT_TELEMETRY_STRUCT;
			m_backedupTelemetryMap[deviceId] = backedupTelemetryObj;	
			backedupTelemetryObj->fileCount = 0;
            
		//	backedupTelemetryObj->jsonObj[DEVICE_ID] = deviceId;
			//backedupTelemetryObj->jsonObj["app"] = m_cloudAppName;
		/*	#ifdef SIEMENS_GATEWAY
				backedupTelemetryObj->jsonObj[TYPE] = dataJson[TYPE];
			#else
				backedupTelemetryObj->jsonObj[TYPE] = DB_TELEMETRY;
			#endif
			*/
			std::time_t curr_time = std::time(nullptr);
			
			struct tm present_time = *localtime(&curr_time);
			
			time_t t_of_day = mktime(&present_time);
			std::cout << "t_of_day : " << t_of_day << std::endl;
			
			t_of_day_str = std::to_string(t_of_day);
			//backedupTelemetryObj->fileName = DB_TELEMERTY_PATH + deviceId + "_" + t_of_day_str + ".json";
			//backedupTelemetryObj->fileName = DB_TELEMERTY_PATH + deviceId + "_" + GetCurrentDate() + ".json";
			backedupTelemetryObj->fileName = DB_TELEMERTY_PATH + deviceId + "_" +  t_of_day_str + ".json";
			std::cout<<"****************************************filename::"<<backedupTelemetryObj->fileName<<std::endl;
			//backedupTelemetryObj->fileName = DB_TELEMERTY_PATH + deviceId + "_" + GetTimeStampInEpoch() + ".json";
		}
		else
		{
			backedupTelemetryObj = it->second;
		}
		
		std::time_t curr_time = std::time(nullptr);
		struct tm present_time = *localtime(&curr_time);
		
		nlohmann::json dbFormatJson;
		#ifdef SIEMENS_GATEWAY
			backedupTelemetryObj->jsonObj["backup_data"].push_back(dataJson);
		#else
			dbFormatJson = dataJson["data"];
			dbFormatJson += dataJson["latest"];
			
			for ( auto& x : dbFormatJson )
			{
				backedupTelemetryObj->jsonObj[DATA].push_back( x );
			}
		#endif

		std::cout << "-------backedupTelemetryObj->fileName-----------" << backedupTelemetryObj->fileName << std::endl; 
		//backedupTelemetryObj->fileCount++; 
		WriteConfiguration( backedupTelemetryObj->fileName, backedupTelemetryObj->jsonObj );
		
		//#define MAX_HOUR 22
		if(abs(start_time.tm_min - present_time.tm_min) >= 1)
		{
			start_time = present_time;
			backedupTelemetryObj->fileCount = 0;
			backedupTelemetryObj->jsonObj["backup_data"].clear();
			
			time_t t_of_day = mktime(&present_time);
			std::cout << "t_of_day : " << t_of_day << std::endl;
			
			t_of_day_str= std::to_string(t_of_day);
	//		backedupTelemetryObj->fileName = DB_TELEMERTY_PATH + deviceId + "_" +  + ".json";
			backedupTelemetryObj->fileName = DB_TELEMERTY_PATH + deviceId + "_" +  t_of_day_str + + ".json";
			 
			std::cout << "-------backedupTelemetryObj->fileName-----------" << backedupTelemetryObj->fileName << std::endl; 
			 
			 //backedupTelemetryObj->fileName = DB_TELEMERTY_PATH + deviceId + "_" + GetTimeStampInEpoch() + ".json";
		} 
		
		
		return true;
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "DataStorageWrapper::FormatAndStoreTelemetryBackup  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "DataStorageWrapper::FormatAndStoreTelemetryBackup  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	return false;
}

bool DataStorageWrapper::FormatAndStoreAlertBackup( nlohmann::json dataJson )
{
	std::stringstream logg;
	try
	{
		if( !dataJson.contains( DEVICE_ID ) )
		{
			logg.str("");
			logg << "DataStorageWrapper::FormatAndStoreAlertBackup  GatewayID : " << m_gatewayId << ",  Message : 'device_id' not found in respected json. JSON : " << dataJson;
			m_exceptionLoggerObj->LogError( logg.str() );
			return false;
		}
		
		if( !dataJson.contains( DATA ) )
		{
			logg.str("");
			logg << "DataStorageWrapper::FormatAndStoreAlertBackup  GatewayID : " << m_gatewayId << ",  Message : 'data' not found in respected json. JSON : " << dataJson;
			m_exceptionLoggerObj->LogError( logg.str() );
			return false;
		}
		
		
		std::string deviceId = dataJson[DEVICE_ID];
		auto it = m_backedupAlertMap.find( deviceId );
		CACHED_ALERT_TELEMETRY_STRUCT* backeupdAlertObj;
		
		if( it == m_backedupAlertMap.end() )
		{
			backeupdAlertObj = new CACHED_ALERT_TELEMETRY_STRUCT;
			m_backedupAlertMap[deviceId] = backeupdAlertObj;	
			
			backeupdAlertObj->fileCount = 0;
			backeupdAlertObj->jsonObj[DEVICE_ID] = deviceId;
			backeupdAlertObj->jsonObj[TYPE] = DB_ALERT;
			backeupdAlertObj->fileName = DB_ALERT_PATH + deviceId + "_" + GetTimeStampInEpoch() + ".json";
		}
		else
		{
			backeupdAlertObj = it->second;
		}
		
		nlohmann::json cachedFormatJson;
		cachedFormatJson = dataJson["data"];
		for ( auto& x : cachedFormatJson )
		{
			backeupdAlertObj->jsonObj[DATA].push_back( x );
		}
		backeupdAlertObj->fileCount++;
		WriteConfiguration( backeupdAlertObj->fileName, backeupdAlertObj->jsonObj );
		
		if( backeupdAlertObj->fileCount >= FILE_COUNT )
		{
			backeupdAlertObj->fileCount = 0;
			backeupdAlertObj->jsonObj[DATA].clear();
			backeupdAlertObj->fileName = DB_ALERT_PATH + deviceId + "_" + GetTimeStampInEpoch() + ".json";
		} 
		
		return true;
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "DataStorageWrapper::FormatAndStoreAlertBackup  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "DataStorageWrapper::FormatAndStoreAlertBackup  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	return false;
}

void DataStorageWrapper::ExecuteCommand( nlohmann::json dataJson )
{
    std::stringstream logg;
	//validate JSON return structure try catch
	//dataJson.erase( dataJson.find( COMMAND_INFO ) );
	
    nlohmann::json tempJson = dataJson[COMMAND_INFO];
    
    if( !tempJson.contains( COMMAND_SCHEMA ) )
    {
        logg.str("");
        logg << "DataStorageWrapper::FormatAndStoreTelemetry  tempJson : " << tempJson << ",  Message : 'commandschema' not found in respected tempJson";
        m_exceptionLoggerObj->LogError( logg.str() );
        return ;
    }
    
    std::string commandSchema = tempJson[COMMAND_SCHEMA];
    
    logg.str("");
    logg << "DataStorageWrapper::ExecuteCommand()  dataJson : " << dataJson << "commandSchema : " << commandSchema << std::endl;
    m_exceptionLoggerObj->LogDebug( logg.str() );
    
	if( commandSchema == "telemetry" )
	{
		FormatAndStoreTelemetry( dataJson );
	}
	if( commandSchema == "alert" )
	{
		FormatAndStoreAlert( dataJson );
	}
}

bool DataStorageWrapper::FormatAndStoreTelemetry( nlohmann::json dataJson )
{
	std::stringstream logg;
	try
	{
		if( !dataJson.contains( DEVICE_ID ) )
		{
			logg.str("");
			logg << "DataStorageWrapper::FormatAndStoreTelemetry  GatewayID : " << m_gatewayId << ",  Message : 'device_id' not found in respected json";
			m_exceptionLoggerObj->LogError( logg.str() );
			return false;
		}
		
		/*
		if( !dataJson.contains( DATA ) )
		{
			logg.str("");
			logg << "DataStorageWrapper::FormatAndStoreTelemetry  GatewayID : " << m_gatewayId << ",  Message : 'data' not found in respected json";
			m_exceptionLoggerObj->LogError( logg.str() );
			return false;
		}
		*/
		
		logg.str("");
		logg << "DataStorageWrapper::FormatAndStoreTelemetry  dataJson : " << dataJson;
		m_exceptionLoggerObj->LogDebug( logg.str() );
		
		std::string deviceId = dataJson[DEVICE_ID];
		auto it = m_cachedTelemetryMap.find( deviceId );
		CACHED_ALERT_TELEMETRY_STRUCT* cachedTelemetryObj;
		
		if( it == m_cachedTelemetryMap.end() )
		{
			cachedTelemetryObj = new CACHED_ALERT_TELEMETRY_STRUCT;
			m_cachedTelemetryMap[deviceId] = cachedTelemetryObj;	
			cachedTelemetryObj->fileCount = 0;
            
			cachedTelemetryObj->jsonObj[DEVICE_ID] = deviceId;
			cachedTelemetryObj->jsonObj["app"] = m_cloudAppName;
			cachedTelemetryObj->jsonObj[TYPE] = CACHED_TELEMETRY;
			cachedTelemetryObj->fileName = CACHED_TELEMERTY_PATH + deviceId + "_" + GetTimeStampInEpoch() + ".json";
		}
		else
		{
			cachedTelemetryObj = it->second;
		}
		
		nlohmann::json cachedFormatJson;
		cachedFormatJson = dataJson["m"];
		//cachedFormatJson += dataJson["latest"];
		
		std::string data;
		#ifdef SIEMENS_GATEWAY
			data = "m";
		#else
			 data = DATA;
		#endif
		
		for ( auto& x : cachedFormatJson )
		{
			cachedTelemetryObj->jsonObj[DEVICE_ID][data].push_back( x );
		}

		cachedTelemetryObj->fileCount++; 
		WriteConfiguration( cachedTelemetryObj->fileName, cachedTelemetryObj->jsonObj );
		
		if( cachedTelemetryObj->fileCount >= FILE_COUNT )
		{
			cachedTelemetryObj->fileCount = 0;
			cachedTelemetryObj->jsonObj[DEVICE_ID][data].clear();
			cachedTelemetryObj->fileName = CACHED_TELEMERTY_PATH + deviceId + "_" + GetTimeStampInEpoch() + ".json";
		} 
		
		return true;
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "DataStorageWrapper::FormatAndStoreTelemetry  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "DataStorageWrapper::FormatAndStoreTelemetry  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	return false;
}

bool DataStorageWrapper::FormatAndStoreAlert( nlohmann::json dataJson )
{
	std::stringstream logg;
	try
	{
		if( !dataJson.contains( DEVICE_ID ) )
		{
			logg.str("");
			logg << "DataStorageWrapper::FormatAndStoreAlert  GatewayID : " << m_gatewayId << ",  Message : 'device_id' not found in respected json. JSON : " << dataJson;
			m_exceptionLoggerObj->LogError( logg.str() );
			return false;
		}
		
		if( !dataJson.contains( DATA ) )
		{
			logg.str("");
			logg << "DataStorageWrapper::FormatAndStoreAlert  GatewayID : " << m_gatewayId << ",  Message : 'data' not found in respected json. JSON : " << dataJson;
			m_exceptionLoggerObj->LogError( logg.str() );
			return false;
		}
		
		
		std::string deviceId = dataJson[DEVICE_ID];
		auto it = m_cachedAlertMap.find( deviceId );
		CACHED_ALERT_TELEMETRY_STRUCT* cachedAlertObj;
		
		if( it == m_cachedAlertMap.end() )
		{
			cachedAlertObj = new CACHED_ALERT_TELEMETRY_STRUCT;
			m_cachedAlertMap[deviceId] = cachedAlertObj;	
			
			cachedAlertObj->fileCount = 0;
			cachedAlertObj->jsonObj[DEVICE_ID] = deviceId;
			cachedAlertObj->jsonObj[TYPE] = CACHED_ALERT;
			cachedAlertObj->fileName = CACHED_ALERT_PATH + deviceId + "_" + GetTimeStampInEpoch() + ".json";
		}
		else
		{
			cachedAlertObj = it->second;
		}
		
		nlohmann::json cachedFormatJson;
		cachedFormatJson = dataJson["data"];
		for ( auto& x : cachedFormatJson )
		{
			cachedAlertObj->jsonObj[DATA].push_back( x );
		}
		cachedAlertObj->fileCount++;
		WriteConfiguration( cachedAlertObj->fileName, cachedAlertObj->jsonObj );
		
		if( cachedAlertObj->fileCount >= FILE_COUNT )
		{
			cachedAlertObj->fileCount = 0;
			cachedAlertObj->jsonObj[DATA].clear();
			cachedAlertObj->fileName = CACHED_ALERT_PATH + deviceId + "_" + GetTimeStampInEpoch() + ".json";
		} 
		
		return true;
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "DataStorageWrapper::FormatAndStoreAlert  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "DataStorageWrapper::FormatAndStoreAlert  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	return false;
}