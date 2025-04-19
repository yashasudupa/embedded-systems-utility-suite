#include "DataCacherManager.h"

/**
 * @brief Create an DataCacherManager	:	It will call the InitDataCacherManager method and init required pointes.
 * 	
 * @param gatewayId 	:	register gateway id name..
 * @param cloudAppName 	:	register cloud app name..
 *										
 */
DataCacherManager::DataCacherManager( std::string gatewayId, std::string cloudAppName ):
	m_gatewayId( gatewayId ),
	m_cloudAppName( cloudAppName ),
	files_list (0)
{
	m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
	InitDataCacherManager();
}

/**
 * @brief destroy an LocalBrokerCommunicationManager	:	It will deinitilize DataCacherManager.
 * 
 */
DataCacherManager::~DataCacherManager()
{
	if( m_localBrokerCommObj )
	{
		delete m_localBrokerCommObj;
	}
	
	if( m_dataStorageWrapperObj )
	{
		delete m_dataStorageWrapperObj;
	}
	
	if( m_fileUploadWrapperObj )
	{
		delete m_fileUploadWrapperObj;
	}
	
}

/**
 * @brief InitGatewayAgentManager	:	This method will initilize FileUploadWrapper, DataStorageWrapper, LocalBrokerCommunicationManager. 
 * 
 */ 
void DataCacherManager::InitDataCacherManager()
{
	std::stringstream logg;
	try
	{
		std::string subCachedData = PUBLISH_PREFIX + m_gatewayId + CACHER_APP_CACHED_DATA_PREFIX;
		std::string subCachedDataRequest = PUBLISH_PREFIX + m_gatewayId + CACHER_APP_CACHED_DATA_REQUEST_PREFIX;
		m_localBrokerCommObj = new LocalBrokerCommunicationManager();
		
		if( m_localBrokerCommObj )
		{
			logg.str("");
			logg << "DataCacherManager::InitDataCacherManager()  GatewayID : " << m_gatewayId << ",  Message : LocalBrokerCommunicationManager object created successfully.";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			m_localBrokerCommObj->RegisterCB( std::bind( &DataCacherManager::ReceiveSubscribedData, this, std::placeholders::_1) );
			m_localBrokerCommObj->SubscribeTopic( subCachedData );
			m_localBrokerCommObj->SubscribeTopic( subCachedDataRequest );
		}
		else
		{
			logg.str("");
			logg << "DataCacherManager::InitDataCacherManager()  GatewayID : " << m_gatewayId << ",  Message : LocalBrokerCommunicationManager object creation Failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
		
		m_dataStorageWrapperObj = new DataStorageWrapper( m_gatewayId, m_cloudAppName  );
		
		if( m_dataStorageWrapperObj == NULL )
		{
			logg.str("");
			logg << "GatewayAgentManager::InitDataCacherManager()  GatewayID : " << m_gatewayId << ",  Message : DataStorageWrapper object creation Failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
		
		m_fileUploadWrapperObj = new FileUploadWrapper( m_cloudAppName, m_gatewayId );
		
		if( m_fileUploadWrapperObj )
		{
			logg.str("");
			logg << "DataCacherManager::InitDataCacherManager()  GatewayID : " << m_gatewayId << ",  Message : FileUploadWrapper object created successfully.";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			m_fileUploadWrapperObj->RegisterCachedDataCB( std::bind( &DataCacherManager::ReceiveCachedData, this, std::placeholders::_1) );
		}
		else
		{
			logg.str("");
			logg << "DataCacherManager::InitDataCacherManager()  GatewayID : " << m_gatewayId << ",  Message : FileUploadWrapper object creation Failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "DataCacherManager::InitDataCacherManager()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "DataCacherManager::InitDataCacherManager()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

void DataCacherManager::rm_oldfile_from_database(CAL_DB &dynamic_db, std::string filepath)
{
	std::stringstream logg;
	try
	{
		//std::lock_guard<std::mutex> lock(mtx);
		std::string filename = filepath + "fmgnmt.txt";
		std::ifstream input_file(filename);
		std::string line;

		if(!asset.empty())
		{
			std::cout << "asset is : " << asset << std::endl;    
			auto epoch_in_string = dynamic_db.calendar_epoch;

			std::cout << "epoch_in_string : " << epoch_in_string << std::endl;
			std::cout << "rm_oldfile_from_database::filename" << filename << std::endl;    
			std::string rm_cmd = "rm " + filepath + asset + ".json";
			std::cout << "rm_cmd :" << rm_cmd << std::endl;
			system(rm_cmd.c_str());
		}
		//mtx.unlock();
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "DataCacherManager::rm_oldfile_from_database() " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "DataCacherManager::rm_oldfile_from_database() " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

void DataCacherManager::first_json_from_files(std::string jsonpath, std::vector<CAL_DB> &dynamic_db_list)
{
	std::stringstream logg;
	try
	{
		//std::lock_guard<std::mutex> lock(mtx);
		std::string filename = jsonpath + "fmgnmt.txt";
		std::string cmd = "ls " + jsonpath + " > " + filename;
		system(cmd.c_str());

		std::ifstream input_file(filename);
		char *ch;
		std::string line;

		std::regex r_e("_[0-9.]+");
		std::smatch m;

		time_t curr_time;

		while(std::getline(input_file, line, '\n'))
		{
			std::cout << "line : " << line << std::endl;
			regex_search(line, m, r_e);

			std::string time_epoch = m.str();

			std::cout << "m.str() : " << time_epoch << std::endl;
			if(time_epoch.empty())
			{
				std::cout << "time_epoch.empty()" << std::endl;
				continue;
			}

			if(asset.empty() && line != "fmgnmt.txt")
			{
				//Legacy_Asset_Rel11_1661498878.json
				std::regex r_a("^(.*?)_[0-9]+");
				regex_search(line, m, r_a);
				asset = m.str();
				std::cout << "asset m.str() : " << asset << std::endl;
			}

			std::string trimmed_time_epoch =  time_epoch.substr(1, time_epoch.find(".") - 1);
			std::cout << "trimmed_time_epoch : " << trimmed_time_epoch.c_str() << std::endl;
			curr_time = std::stoi(trimmed_time_epoch, nullptr, 10);

			tm *tm_gmt = gmtime(&curr_time);

			CAL_DB dynamic_db;
			dynamic_db.calendar_epoch = trimmed_time_epoch;
			dynamic_db.calendar_tm = *tm_gmt;

			dynamic_db_list.push_back(dynamic_db);
			files_list++;
		}
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "DataCacherManager::first_json_from_files() " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "DataCacherManager::first_json_from_files() " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

short DataCacherManager::backup_procedure(std::vector<CAL_DB> &dynamic_db_list, std::string filepath)
{
	std::stringstream logg;
	try
	{
        //Open the file
		std::string filename = filepath + "fmgnmt.txt";

		//std::cout << "filename : " << filename << std::endl;

		std::ifstream in_file(filename, std::ios::binary);
		std::fstream file(filename, std::ios::binary);

		//in_file.open(filename, std::ios_base::app);
		in_file.seekg(0, std::ios::end);
		int file_size = in_file.tellg();

		if (file_size == -1)
		{
				std::cout << "File not found.";
				return -1; 
		}

		//std::cout << "file_size : " << file_size << std::endl;   

		// Check if the application was closed and continuing with the backup
		
		std::cout << "dynamic_db_list.size() : " << dynamic_db_list.size() << std::endl;
		first_json_from_files(filepath, dynamic_db_list);   						
		CAL_DB first_item = dynamic_db_list[0];
		std::cout << "first_item : " << first_item.calendar_epoch << std::endl;
		for(auto it = dynamic_db_list.begin() + 1; it!=dynamic_db_list.end(); ++it)
		{
			
			if (files_list>730)
			{
				rm_oldfile_from_database(first_item, filepath);
			}
			//std::lock_guard<std::mutex> lock(mtx);
			dynamic_db_list.clear();
			asset.clear();
			files_list = 0;
		}
		
		std::cout << "DataCacherManager::backup_procedure returning safely" << std::endl;
		return 0;
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "DataCacherManager::backup_procedure() " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "DataCacherManager::backup_procedure() " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief DoDataBackup		:	This method will do backup of 1 month data that is posted by GW Agent and if  
 * 								the reception of data exceeds more than a month then the data which is older  
 * 								than a month will be removed
 */
bool DataCacherManager::one_month_databackup(nlohmann::json dataJson)
{
	std::stringstream logg;
	try
	{
		//keep storing irrespective of the date
		std::thread StoreThread = std::thread([this, dataJson]()
		{
			//std::lock_guard<std::mutex> lock(mtx);
			
			std::cout << "-------m_dataStorageWrapperObj->ExecuteBackup-----------" << std::endl;
			
			m_dataStorageWrapperObj->ExecuteBackup( dataJson );
		});
		StoreThread.detach();
		
		std::string commandSchema = dataJson[COMMAND_INFO][COMMAND_SCHEMA];
		
		logg.str("");
		logg << "DataCacherManager::DoDataBackup dataJson : " << dataJson << commandSchema << std::endl;
		m_exceptionLoggerObj->LogDebug( logg.str() );
		
		std::thread backupThread = std::thread([this, commandSchema]()
		{
			short wError;
			std::cout << "DataCacherManager::DoDataBackup backupThread is opened " << std::endl;
			if( commandSchema == "telemetry" )
			{
				wError = backup_procedure(cal_epoch_tm, DB_TELEMERTY_PATH);
			}
			if( commandSchema == "alert" )
			{
				wError = backup_procedure(cal_epoch_tm, DB_ALERT_PATH);
			}
			if( wError != -1 )
			{
				std::cout << "Backup is performed successfully - wError : " << wError << std::endl;
			}
			else
			{
				std::cout << "Error in performing backup - wError : " << wError << std::endl;
			}
		});
		backupThread.detach();
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "DataCacherManager::DoDataBackup() " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "DataCacherManager::DoDataBackup() " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}
/**
 * @brief ReceiveSubscribedData		:	This method will bind with RegisterCB call back which will receive the cached data from 
 * 										communicator app. This methode will pass the received data to the Datastorage class
 * 
 * @param std::string data	 		:	Content of the request which is send by communicator app
 */
void DataCacherManager::ReceiveSubscribedData( std::string data )
{
	std::stringstream logg;
	try
	{
		nlohmann::json dataJson = nlohmann::json::parse( data );
		std::string commandSchema = dataJson[COMMAND_INFO][COMMAND_TYPE];
        
        logg.str("");
		logg << "DataCacherManager::ReceiveSubscribedData()  GatewayID : " << m_gatewayId << "  Cached DataJson is  : "  << dataJson;
		m_exceptionLoggerObj->LogDebug( logg.str() );
        
//      std::string commandSchema = dataJson[COMMAND_INFO][COMMAND_SCHEMA];
		
		one_month_databackup(dataJson);
		
        logg.str("");
		logg << "DataCacherManager::ReceiveSubscribedData()  GatewayID : " << m_gatewayId << "  commandSchema is  : "  << commandSchema;
		m_exceptionLoggerObj->LogDebug( logg.str() );
		if( commandSchema == CONNECTION_STATUS )
		{
			if( dataJson[CONNECTION_STATUS] )
			{
				m_fileUploadWrapperObj->UploadFiles();
				logg.str("");
				logg << "********** \n"; 
				logg << "DataCacherManager::ReceiveSubscribedData()  CONNECTION_STATUS : TRUE " << dataJson << "  commandSchema is a : "  << commandSchema;
				logg << "**********";
				m_exceptionLoggerObj->LogDebug( logg.str() ); // H1
			}
		}
		else
		{
            logg.str("");
			logg << "********** \n"; 
            logg << "DataCacherManager::ReceiveSubscribedData()  CONNECTION_STATUS : FALSE " << dataJson << "  commandSchema is a : "  << commandSchema;
			logg << "**********";
            m_exceptionLoggerObj->LogDebug( logg.str() ); // H1

            m_dataStorageWrapperObj->ExecuteCommand( dataJson );
		}
		
		logg.str("");
		logg << "DataCacherManager::ReceiveSubscribedData()  one month data backup invokes ";
		m_exceptionLoggerObj->LogDebug( logg.str() ); // H1
		
		if( commandSchema == DEVICE_DATA_BACKUP )
		{
			logg.str("");
            logg << "DataCacherManager::ReceiveSubscribedData() : DEVICE_DATA_BACKUP";
            m_exceptionLoggerObj->LogDebug( logg.str() ); // H1
		
			std::string commandInfo = dataJson[COMMAND_INFO][COMMAND_SCHEMA];
			
			short wError;			
			if (dataJson.contains(FILEUPLOAD_REQUEST_RECEIVED))
			{
				std::cout << "DataCacherManager::ReceiveSubscribedData() fileupload request is received " << std::endl;
				if( commandSchema == "telemetry" )
				{
					wError = m_fileUploadWrapperObj->UploadFiles(dataJson, DB_TELEMERTY_PATH);
				}
				if( commandSchema == "alert" )
				{
					wError = m_fileUploadWrapperObj->UploadFiles(dataJson, DB_ALERT_PATH);
				}
				if( wError != -1 )
				{
					std::cout << "DataCacherManager::ReceiveSubscribedData() fileupload request is processed " << std::endl;
				}
				else
				{
					std::cout << "DataCacherManager::ReceiveSubscribedData() fileupload request is not processed " << std::endl;
				}
				return;
			}
		}
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "DataCacherManager::ReceiveSubscribedData()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "DataCacherManager::ReceiveSubscribedData()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief ReceiveCachedData			:	This method will bind with RegisterCachedDataCB call back which will receive the cached data from 
 * 										FileUpload class. This methode will publish the received data to the communicator app
 * 
 * @param nlohmann::json jsonObj 	:	Content of the request which is send by FileUpload class.
 */
void DataCacherManager::ReceiveCachedData( nlohmann::json jsonObj )
{
	std::string cachedDataUploadTopic = PUBLISH_PREFIX 	+ m_gatewayId + CACHER_APP_CACHED_DATA_UPLOAD_PREFIX;
	std::string jsonstr = jsonObj.dump();
	m_localBrokerCommObj->PublishData( jsonstr, cachedDataUploadTopic );
}

