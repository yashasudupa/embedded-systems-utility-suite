#include "GatewayAgentManager.h"

int debug_int = 0;
/**
 * @brief Create an GatewayAgentManager	:	It will call the initilize method. 
 * 
 * @param std::string processName 		:	it represent current process name.
 */
GatewayAgentManager::GatewayAgentManager( std::string processName ):
	m_processName( processName ),
	m_updateGatewayFlag( true ),
    m_networkFailFlag( false )
{
	m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
	InitGatewayAgentManager();
    
   // system("ip addr flush dev eth0");
    //system("ipconfig eth0 192.168.0.100");
}

/**
 * @brief destroy an GatewayAgentManager	:	It will deinitilize the CloudCommunicationWrapper,PackageManager, 
 * 												LocalBrokerCommunicationManager and watchdog.
 */
GatewayAgentManager::~GatewayAgentManager()
{
	if( m_cloudConnectionObj )
	{
		delete m_cloudConnectionObj;
	}
	
	if( m_watchDogObj )
	{
		delete m_watchDogObj;
	}
	
	if( m_sendMsgToCloudObj )
	{
		delete m_sendMsgToCloudObj;
	}
	
	if( m_packageManagerObj )
	{
		delete m_packageManagerObj;
	}	
		
	if( m_localBrockerObj )
	{
		delete m_localBrockerObj;
	}
    
    if( m_logCleanUpObj )
	{
		delete m_logCleanUpObj;
	}
    
}

/**
 * @brief ParseTimeInUTC	:	Convert seconds to UTC format 
 * 								
 * 
 */ 

void GatewayAgentManager::ParseTimeInUTC(time_t t, DATE_MONTH_AND_TIME_DATABASE &db)
{
	struct tm * timeinfo = localtime (&t);
	
	// Convert seconds into human readable format
	// e.g: format : Wed Feb 13 15:46:11 2013
	std::string time_in_hrf = asctime(timeinfo);
	
	std::cout << "time_in_hrf : " << time_in_hrf << std::endl;
	char *str = (char *)time_in_hrf.c_str();

	char *token = std::strtok(str, " ");

	//Parse appropriate month
	token = std::strtok(NULL, " ");
	db.month = token;
	
	//Parse appropriate date
	token = std::strtok(NULL, " ");
	db.date = atoi(token);
	
	//Parse appropriate hour
	token = std::strtok(NULL, ":");
	db.hours = atoi(token);
	
	//Parse appropriate minute
	token = std::strtok(NULL, ":");
	db.minutes = atoi(token);
	
	std::cout << "db.minutes : " << db.minutes << std::endl;
	
	//Parse appropriate seconds
	token = std::strtok(NULL, " ");
	db.seconds = atoi(token);
	
	std::cout << "db.seconds : " << db.seconds << std::endl;

}

/**
 * @brief CheckVersionChange	:	This method will check the gateway version 
 * 
 */ 
bool GatewayAgentManager::CheckVersionChange()
{
	std::stringstream logg;
	try
	{
		nlohmann::json gatewayAgentJson;
		gatewayAgentJson = ReadAndSetConfiguration( GATEWAYAGENT_PERSISTENCY_CONFIG );
		
		if( gatewayAgentJson.is_null() )
		{
			nlohmann::json installedAppInfoJson;
			gatewayAgentJson["GatewayAgent"] = VERSION_NUMBER;
			WriteConfiguration( GATEWAYAGENT_PERSISTENCY_CONFIG, gatewayAgentJson );
			installedAppInfoJson[INSTALLED_PACKAGES]["GatewayAgent"] = VERSION_NUMBER;
			installedAppInfoJson[SYSTEM_APPS]["GatewayAgent"][STATUS] = "Running";
			m_sendMsgToCloudObj->SendReportedJsonToCloud( installedAppInfoJson, REPORT_GENERIC );
		}
		else
		{
			std::string versionNo = gatewayAgentJson["GatewayAgent"];
			if( versionNo != VERSION_NUMBER ) //strcmp use
			{
				std::ifstream ifile;
				std::string filepath = INSTALLPATH; 
				filepath += "temp_gateway_version.txt";
				ifile.open( filepath.c_str() );
				nlohmann::json jsonObj;
				jsonObj[APP_NAME] = "GatewayAgent";
				jsonObj[SUB_JOB_ID] = gatewayAgentJson[SUB_JOB_ID];
				jsonObj["package_details"]["version"] = VERSION_NUMBER;
				m_sendMsgToCloudObj->SendReportedJsonToCloud( jsonObj, REPORT_INSTALLED );
				gatewayAgentJson["GatewayAgent"] = VERSION_NUMBER;
				WriteConfiguration( GATEWAYAGENT_PERSISTENCY_CONFIG, gatewayAgentJson );
				
				nlohmann::json installedAppInfoJson1;
				installedAppInfoJson1[SUB_JOB_ID] = gatewayAgentJson[SUB_JOB_ID];
				installedAppInfoJson1[INSTALLED_PACKAGES]["GatewayAgent"] = VERSION_NUMBER;
				installedAppInfoJson1[SYSTEM_APPS]["GatewayAgent"] = "Running";
				m_sendMsgToCloudObj->SendReportedJsonToCloud( installedAppInfoJson1, REPORT_GENERIC );
				
				if( ifile ) 
				{
					std::string fileCommand = "rm -f " + filepath;
					system( fileCommand.c_str() );
				}
				else 
				{
					ResponseStruct responseStructObj;
					responseStructObj.status = SUCCESS;
					responseStructObj.responseMetaInformation = "GatewayAgent apllication updated manually.";
					SendNotificationToCloud( responseStructObj );
				}
			}
		}
	}
	catch( ... )
	{
		logg.str("");
		logg << "GatewayAgentManager::CheckVersionChange()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
		return false;
	}
	return true;
}
 
/**
 * @brief InitGatewayAgentManager	:	This method will initilize CloudCommunicationWrapper,PackageManager, LocalBrokerCommunicationManager and watchdog.. 
 * 
 */ 
void GatewayAgentManager::InitGatewayAgentManager()
{
	std::stringstream logg;
	try
	{
		//Initilize CloudCommunicationWrapper Object.
		m_cloudConnectionObj = new CloudCommunicationWrapper();

		if( m_cloudConnectionObj )
		{
			m_gatewayId = m_cloudConnectionObj->GetGatewayId();
			m_cloudAppName = m_cloudConnectionObj->GetCloudAppName();
			
			logg.str("");
			logg << "GatewayAgentManager::InitGatewayAgentManager()  GatewayID : " << m_gatewayId << ",  Message : CloudCommunicationWrapper object created Successfully.";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			
			m_sendMsgToCloudObj = m_sendMsgToCloudObj->GetInstance( m_cloudConnectionObj );
			m_cloudConnectionObj->RegisterCloudRequestCB( std::bind( &GatewayAgentManager::DataRecevier, this, std::placeholders::_1) );
			m_cloudConnectionObj->RegisterConnectionStatusCB( std::bind( &GatewayAgentManager::ConnectionStatusReceiver, this, std::placeholders::_1 ) );
			
		}
		else
		{
			logg.str("");
			logg << "GatewayAgentManager::InitGatewayAgentManager()  GatewayID : " << m_gatewayId << ",  Message : Cloud communication object creation Failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
		
        m_watchDogObj = m_watchDogObj->GetInstance();
		if( m_watchDogObj )
		{
			logg.str("");
			logg << "GatewayAgentManager::InitGatewayAgentManager()  GatewayID : " << m_gatewayId << ",  Message : WatchDog object created Successfully.";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			m_watchDogObj->RegisterWatchDogCB( std::bind( &GatewayAgentManager::WatchdogCommandReceiver, this, std::placeholders::_1 ) );
			m_watchDogObj->StartWatchDogThread();
		}
		else
		{
			logg.str("");
			logg << "GatewayAgentManager::InitGatewayAgentManager()  GatewayID : " << m_gatewayId << ",  Message : WatchDog object creation Failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
		
		m_localBrockerObj = new LocalBrokerCommunicationManager();
		if( m_localBrockerObj )
		{
			logg.str("");
			logg << "GatewayAgentManager::InitGatewayAgentManager()  GatewayID : " << m_gatewayId << ",  Message : Local Broker Communication object created successfully.";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			
            //kemsys/gateway/<gw_id>/communicatorapp/rule_device/response
			std::string topic = PUBLISH_PREFIX + m_gatewayId + COMMUNICATOR_APP_RESPONSE_PREFIX;
			m_localBrockerObj->RegisterCB( std::bind( &GatewayAgentManager::ReceiveDeviceData, this, std::placeholders::_1 ) );
			m_localBrockerObj->SubscribeTopic( topic );
		}
		else
		{
			logg.str("");
			logg << "GatewayAgentManager::InitGatewayAgentManager()  GatewayID : " << m_gatewayId << ",  Message : Local Broker Communication object creation Failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
		
		m_packageManagerObj = new PackageManager( m_gatewayId, m_cloudAppName, m_processName );
		if( m_packageManagerObj )
		{
			logg.str("");
			logg << "GatewayAgentManager::InitGatewayAgentManager()  GatewayID : " << m_gatewayId << ",  Message : Package Manager object created successfully.";
			m_exceptionLoggerObj->LogInfo( logg.str() );
		}
		else
		{
			logg.str("");
			logg << "GatewayAgentManager::InitGatewayAgentManager()  GatewayID : " << m_gatewayId << ",  Message : Package Manager object creation Failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
        
        m_logCleanUpObj = new LogFileCleanUp( m_gatewayId );
	}
	catch( ... )
	{
		logg.str("");
		logg << "GatewayAgentManager::InitGatewayAgentManager()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief DataRecevier		:	This method will bind with data recevier call back which will receive the  
 * 								cloud requested data. 
 * 
 * @param nlohmann::json jsonObject 	:	Content of the request which is send by cloud.
 */
void GatewayAgentManager::DataRecevier( nlohmann::json jsonObject )
{
	ExecuteCloudCommand ( jsonObject );
}

/**
 * @brief WatchdogCommandReceiver		:	This method will bind with watchdog call back which will receive the  
 * 											watchdog requested data. 
 * 
 * @param nlohmann::json jsonObject 	:	Content of the request which is send by watchdog.
 */
void GatewayAgentManager::WatchdogCommandReceiver( nlohmann::json jsonObject )
{
	ResponseStruct responseStructObj;
	responseStructObj = m_packageManagerObj->StartApplication( jsonObject );
	
	if( responseStructObj.status == SUCCESS )
	{
        std::stringstream logg;
		logg.str("");
        logg << "GatewayAgentManager::WatchdogCommandReceiver()  GatewayID : " << m_gatewayId << ",  Message : " << responseStructObj.responseMetaInformation ;
        m_exceptionLoggerObj->LogInfo( logg.str() );
	}
	else
	{
		
	}

}

/**
 * @brief destroy an ConnectionStatusReceiver	:	This method will bind with connection status call back which will 
 * 													receive the cloud requested data. If Connetion status is disconnected 
 * 													then it will maintain network restart notification json. and it will 
 * 													send to the cloud once connection established status received.
 * 
 * @param nlohmann::json jsonObject 			:	content of the request which is send by cloud.
 */
void GatewayAgentManager::ConnectionStatusReceiver( nlohmann::json jsonObject )
{
	//std::string publishTopic = PUBLISH_PREFIX + m_gatewayId + GATEWAY_AGENT_CONNECTION_PREFIX;
	//m_localBrockerObj->PublishData( jsonObject.dump(), publishTopic );
	
	if( jsonObject[CONNECTION_STATUS] && m_updateGatewayFlag )
	{
		int retryCount = 3;
		bool publishStatus = false;
		while( retryCount-- && !publishStatus )
		{
			publishStatus = CheckVersionChange();
			if( publishStatus )
			{
				m_updateGatewayFlag = false;
			}
            usleep(100);
		}
	}
	if( jsonObject[CONNECTION_STATUS] && !m_networkFailJson.is_null() ) //If conncetion status is true network restart notification send to cloud.
	{
		std::stringstream logg;
		logg << "GatewayAgentManager::ConnectionStatusReceiver()  GatewayID : " << m_gatewayId << ",  Message : Network has been restarted.";
		m_exceptionLoggerObj->LogInfo( logg.str() );
		
        if( m_networkFailFlag )
        {
            std::string msg = m_networkFailJson.dump();
            m_networkFailJson.erase( m_networkFailJson.begin() );
            m_networkFailJson.clear();
            m_networkFailJson = nullptr;
            m_cloudConnectionObj->SendDeviceToCloudMessage( (char *)msg.c_str(), "notification" );
            m_networkFailFlag = false;
        }
    }
	else
	{
        m_networkFailFlag = true;
		m_networkFailJson[TYPE] = "notification";
		m_networkFailJson[MESSAGE] = "Network has Restarted";
		m_networkFailJson[EVENT] = "network_restart";
		m_networkFailJson[TIMESTAMP] = GetTimeStamp();
	}
}

/**
 * @brief ReceiveDeviceData		:	This method will bind with receive device data call back which will 
 * 									receive the legacy device data like telemetry, alert, notification and.
 *									cached telemetry and cached alert etc.
 * 									It will call appropriate methode and send data to cloud once data received.
 * 
 * @param std::string data		:	content of the request which is send by device.
 */
void GatewayAgentManager::ReceiveDeviceData( std::string data )
{
	try
	{
		nlohmann::json dataJsonObj = nlohmann::json::parse( data );
		std::string type = dataJsonObj[TYPE];
		//use macro
		if( type == RULE_ENGINE )
		{
			//
		}
		else if( type == REPORTED_TWIN )
		{
            if( dataJsonObj.contains(COMMAND_INFO) )
            {
                dataJsonObj.erase( dataJsonObj.find(COMMAND_INFO) );
            }
			dataJsonObj.erase( dataJsonObj.find(TYPE) );
			data = dataJsonObj.dump();
			m_cloudConnectionObj->SendReportedState( (char*)data.c_str() );
		}
		else if( type == CACHED_TELEMETRY )
		{
			std::string destFilePath = dataJsonObj[COMMAND_INFO][FILE_NAME];
			dataJsonObj.erase(dataJsonObj.find(COMMAND_INFO));
			m_cloudConnectionObj->UploadBlobStorage( dataJsonObj.dump(), destFilePath );
		}
		else
		{
			if( dataJsonObj.contains( COMMAND_INFO ) ) 
			{
				dataJsonObj.erase( dataJsonObj.find(COMMAND_INFO) );
			}
			data = dataJsonObj.dump();
			m_cloudConnectionObj->SendDeviceToCloudMessage( (char*)data.c_str(), TELEMETRY_STRING );
		}
	}
	catch( ... )
	{
		std::stringstream logg;
		logg << "GatewayAgentManager::ReceiveDeviceData()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief BluenrgMonitor		:	Method to monitor the bluenrg
 * 
 */
void GatewayAgentManager::BluenrgMonitor(time_t* time_monitor)
{

	// Check if the gateway start time and running time is more than 600 secs
	std::stringstream logg;
	if ((time(NULL) - *time_monitor) >= BUFF_TIME)
	{
		time_t current = time(NULL);
		
		ParseTimeInUTC(*time_monitor, db1);
		
		ParseTimeInUTC(current, db2);
		
		*time_monitor = time(NULL);
		
		// Logic to check if it is midnight
		if (db2.date != db1.date)
		{
			// Run the shell script to do the hard reset
			logg.str("");
			logg << "Bluenrg reset is done" << "\n";
			m_exceptionLoggerObj->LogDebug( logg.str() );
			
			m_deviceFileName = "./usr/bin/resetkey-script.sh";
			system("resetkey-script.sh");

			/*
			nlohmann::json jsonObject;
			std::string jsonString = "";					
			std::string publishTopic = PUBLISH_PREFIX + m_gatewayAgentManagerObj->m_gatewayId + COMMUNICATOR_APP_REQUEST_PREFIX;

			m_gatewayAgentManagerObj->m_deviceFileName = "./config/persistency_config/communicationApp_persistency_config.json";
			m_gatewayAgentManagerObj->m_devicesRegisterJson = ReadAndSetConfiguration( m_gatewayAgentManagerObj->m_deviceFileName );

			LocalBrokerCommunicationManager *m_brokerObj = (LocalBrokerCommunicationManager *)m_gatewayAgentManagerObj->cmdStrctObj1->objectArgv;
			jsonObject[COMMAND] = RESET_BLUENRG;
			jsonObject[DEVICE_ID] = m_gatewayAgentManagerObj->m_devicesRegisterJson[DEVICE_ID];
			jsonString = jsonObject.dump();
			m_brokerObj->PublishData( jsonString, publishTopic );
			
			 */
			memset(&db1, 0, sizeof(db1));
			memset(&db2, 0, sizeof(db2));
		}
		// Again check for next 24 hrs from this instance of time
		*time_monitor = time(NULL);
	}
}


/**
 * @brief ExecuteCloudCommand		:	This method will follow the received request and redirect to particuler
 * 										method
 * 
 * @param nlohmann::json jsonObject	:	content of the request which is send by cloud.
 */
void GatewayAgentManager::ExecuteCloudCommand( nlohmann::json jsonObject )
{
	ResponseStruct responseStructObj;
	std::stringstream logg;
	try
	{
		if( jsonObject[COMMAND].is_null() )
		{
			logg.str("");
			logg << "GatewayAgentManager::ExecuteCloudCommand()  GatewayID : " << m_gatewayId << ",  Message : Command Not found in respected json. Json : " << jsonObject;
			m_exceptionLoggerObj->LogError( logg.str() );
			return;
		}
		
		int responseStatus;
		std::string jsonString = "";
		std::string commandStr = jsonObject[COMMAND];
        std::string subJobId = "";
		
		transform(commandStr.begin(), commandStr.end(), commandStr.begin(), ::tolower); 
		std::string publishTopic = PUBLISH_PREFIX + m_gatewayId + COMMUNICATOR_APP_REQUEST_PREFIX;
		
		logg.str("");
		logg << "GatewayAgentManager::ExecuteCloudCommand()  commandstr : " << commandStr;
		m_exceptionLoggerObj->LogDebug( logg.str() );
		
		if( commandStr == START_APP )
		{
			responseStructObj = ValidateJson( jsonObject, START_APPLICATION );
			if( responseStructObj.status == SUCCESS )
			{
				responseStructObj = m_packageManagerObj->StartApplication( jsonObject );
			}
			m_cloudConnectionObj->SetDirectMethodResponse( responseStructObj );
		}
		else if( commandStr == STOP_APP )
		{
			responseStructObj = ValidateJson( jsonObject, STOP_APPLICATION );
			if( responseStructObj.status == SUCCESS )
			{
				jsonObject[UPDATE_FLAG] = false;
				responseStructObj = m_packageManagerObj->StopApplication( jsonObject );
			}
			m_cloudConnectionObj->SetDirectMethodResponse( responseStructObj );
		}
		else if( commandStr == RESTART_APP )
		{
			responseStructObj = ValidateJson( jsonObject, RESTART_APPLICATION );
			if( responseStructObj.status == SUCCESS )
			{
				responseStructObj = m_packageManagerObj->RestartApplication( jsonObject );
			}
			m_cloudConnectionObj->SetDirectMethodResponse( responseStructObj );
		}
		else if( commandStr == RESTART_GATEWAY )
		{
			m_packageManagerObj->RebootGateway();
		}
        else if( commandStr == INSTALL_NGROK )
		{
            std::string installNgrok = "/opt/IoT_Gateway/ngrok/NgrokDiagnostic.sh install";
            system( installNgrok.c_str() );
		}
        else if( commandStr == UNINSTALL_NGROK )
		{
			std::string uninstallNgrok = "/opt/IoT_Gateway/ngrok/NgrokDiagnostic.sh uninstall";
            system( uninstallNgrok.c_str() );
		}
        else if( commandStr == SYSTEM_REBOOT_TIME )
		{
			responseStructObj = ValidateJson( jsonObject, SYSTEM_REBOOT );
			if( responseStructObj.status == SUCCESS )
			{
                long systemreboot = 0;
                
                systemreboot = jsonObject[REBOOT_TIMER];
                
                logg.str("");
                logg << "GatewayAgentManager::ExecuteCloudCommand()  Json : " << jsonObject << "\nsystemreboot :  " << systemreboot;
                m_exceptionLoggerObj->LogInfo( logg.str() );
                
				m_cloudConnectionObj->SetSystemRebootTime( systemreboot );
			}
			m_cloudConnectionObj->SetDirectMethodResponse( responseStructObj );
		}
		else if( commandStr == INSTALL_PACKAGE )
		{
			responseStructObj = ValidateJson( jsonObject[PACKAGE_DETAILS], INSTALL );
			if( responseStructObj.status == SUCCESS )
			{
				responseStructObj = m_packageManagerObj->InstallPackage( jsonObject [PACKAGE_DETAILS] );
				subJobId = jsonObject [PACKAGE_DETAILS][SUB_JOB_ID];
				SendNotificationToCloud( responseStructObj, subJobId );
			}
			else
			{
                if( jsonObject[PACKAGE_DETAILS].contains( SUB_JOB_ID ) )
                {
                    subJobId = jsonObject[PACKAGE_DETAILS][SUB_JOB_ID];
                    SendNotificationToCloud( responseStructObj, subJobId );
                }
                else
                {
                    SendNotificationToCloud( responseStructObj );
                }
			}
		}
		else if( commandStr == UPGRADE_PACKAGE )
		{
			responseStructObj = ValidateJson( jsonObject[PACKAGE_DETAILS], UPGRADE );
			if( responseStructObj.status == SUCCESS )
			{
				responseStructObj = m_packageManagerObj->UpgradePackage( jsonObject[PACKAGE_DETAILS] );
				subJobId = jsonObject [PACKAGE_DETAILS][SUB_JOB_ID];
				SendNotificationToCloud( responseStructObj, subJobId );
			}
			else
			{
				if( jsonObject[PACKAGE_DETAILS].contains( SUB_JOB_ID ) )
                {
                    subJobId = jsonObject[PACKAGE_DETAILS][SUB_JOB_ID];
                    SendNotificationToCloud( responseStructObj, subJobId );
                }
                else
                {
                    SendNotificationToCloud( responseStructObj );
                }
			}
		}
		else if( commandStr == DELETE_PACKAGE )
		{
			responseStructObj = ValidateJson( jsonObject[PACKAGE_DETAILS], DELETE );
			if( responseStructObj.status == SUCCESS )
			{
				responseStructObj = m_packageManagerObj->UninstallPackage( jsonObject [PACKAGE_DETAILS] );
				subJobId = jsonObject [PACKAGE_DETAILS][SUB_JOB_ID];
				SendNotificationToCloud( responseStructObj, subJobId );
			}
			else
			{
				if( jsonObject[PACKAGE_DETAILS].contains( SUB_JOB_ID ) )
                {
                    subJobId = jsonObject[PACKAGE_DETAILS][SUB_JOB_ID];
                    SendNotificationToCloud( responseStructObj, subJobId );
                }
                else
                {
                    SendNotificationToCloud( responseStructObj );
                }
			}
		}
		else if( commandStr == REGISTER_DEVICES )
		{
			responseStructObj = ValidateJson( jsonObject, REGISTER_DEVICES_CASE );
			if( responseStructObj.status == SUCCESS )
			{
				jsonObject[COMMAND_INFO][COMMAND_TYPE] = REQUEST;
				jsonString = jsonObject.dump();
				m_localBrockerObj->PublishData( jsonString, publishTopic );
			}
			else
			{
				if( jsonObject.contains( SUB_JOB_ID ) )
                {
                    subJobId = jsonObject[SUB_JOB_ID];
                    SendNotificationToCloud( responseStructObj, subJobId );
                }
                else
                {
                    SendNotificationToCloud( responseStructObj );
                }
			}
		}
		
		else if (commandStr == DEREGISTER_DEVICES )
		{
			responseStructObj = ValidateJson( jsonObject, DEREGISTER_DEVICES_CASE );
			if( responseStructObj.status == SUCCESS )
			{
				jsonObject[COMMAND_INFO][COMMAND_TYPE] = REQUEST;
				jsonString = jsonObject.dump();
				m_localBrockerObj->PublishData( jsonString, publishTopic );
			}
			else
			{
				if( jsonObject.contains( SUB_JOB_ID ) )
                {
                    subJobId = jsonObject[SUB_JOB_ID];
                    SendNotificationToCloud( responseStructObj, subJobId );
                }
                else
                {
                    SendNotificationToCloud( responseStructObj );
                }
			}
		}
		
		else if( commandStr == SET_POLLING_CONFIG || commandStr == SET_CONFIGURATION )
		{
			
			if(!jsonObject.contains( DEVICES ))
				{
					if(jsonObject.contains( APP_NAME ) )
						{
							std::string app = jsonObject[APP_NAME];
							m_deviceFileName = "./config/" + app + "/register_devices.json";
							m_devicesRegisterJson = ReadAndSetConfiguration( m_deviceFileName );
							jsonObject[DEVICES] = m_devicesRegisterJson[DEVICES];
							logg << "m_devicesRegisterJson[DEVICES]: " << jsonObject << "\n";
							m_exceptionLoggerObj->LogDebug( logg.str() );
						}
				}
		
			responseStructObj = ValidateJson( jsonObject, SET_POLLING_CONFIG_CASE );
			
			std::cout << "responseStructObj.status : " << responseStructObj.status  << std::endl;
			
			if( responseStructObj.status == SUCCESS )
			{
				jsonObject[COMMAND_INFO][COMMAND_TYPE] = REQUEST;
				jsonString = jsonObject.dump();
				m_localBrockerObj->PublishData( jsonString, publishTopic );
			}
			else
			{
				if( jsonObject.contains( SUB_JOB_ID ) )
                {
                    subJobId = jsonObject[SUB_JOB_ID];
                    SendNotificationToCloud( responseStructObj, subJobId );
                }
                else
                {
                    SendNotificationToCloud( responseStructObj );
                }
			}
			std::cout << "set_configuration is done " << std::endl;
		}
		
		else if(commandStr == GET_DEVICE_CONFIGURATION || commandStr == READ_SENSOR_POSITION_CONFIGURATION)
		{
			
			m_deviceFileName = "./config/persistency_config/communicationApp_persistency_config.json";
			m_devicesRegisterJson = ReadAndSetConfiguration( m_deviceFileName );
			
			if( !jsonObject.contains( APP_NAME ) )
			{   
				#ifdef BLUENRG_RESET                 
					jsonObject[APP_NAME] = "N_BlueNRG"; 
					logg << "jsonObject[APP_NAME] : " << jsonObject[APP_NAME] << "\n";
					m_exceptionLoggerObj->LogDebug( logg.str() );
				#endif
			}
			
			m_deviceFileName = "./config/N_BlueNRG/register_devices.json";
			m_devicesRegisterJson = ReadAndSetConfiguration( m_deviceFileName );
			
			if(!jsonObject.contains( DEVICES ))
				{
					jsonObject[DEVICES] = m_devicesRegisterJson[DEVICES];
				}
			
			responseStructObj = ValidateJson( jsonObject, GET_DEVICE_OR_GET_POS_CONFIG_CASE);
			
			if( responseStructObj.status == SUCCESS )
			{
				jsonObject[COMMAND_INFO][COMMAND_TYPE] = REQUEST;
				logg.str("");
				logg << "jsonObject : " << jsonObject;
				m_exceptionLoggerObj->LogDebug( logg.str() );
				jsonString = jsonObject.dump();
				m_localBrockerObj->PublishData( jsonString, publishTopic );
			}
			else
			{
				if( jsonObject.contains( SUB_JOB_ID ) )
                {
                    subJobId = jsonObject[SUB_JOB_ID];
                    SendNotificationToCloud( responseStructObj, subJobId );
                }
                else
                {
                    SendNotificationToCloud( responseStructObj );
                }
			}
		}
		
		else if( commandStr == SET_VALUE_CANAGE )
		{
			responseStructObj = ValidateJson( jsonObject, SET_VALUE_CANAGE_CASE );
			if( responseStructObj.status == SUCCESS )
			{
				jsonObject[COMMAND_INFO][COMMAND_TYPE] = REQUEST;
				jsonString = jsonObject.dump();
				m_localBrockerObj->PublishData( jsonString, publishTopic );
			}
			else
			{
				if( jsonObject.contains( SUB_JOB_ID ) )
                {
                    subJobId = jsonObject[SUB_JOB_ID];
                    SendNotificationToCloud( responseStructObj, subJobId );
                }
                else
                {
                    SendNotificationToCloud( responseStructObj );
                }
			}
		}
		
		else if( commandStr == SET_PROPERTIES )
		{
			jsonObject[COMMAND_INFO][COMMAND_TYPE] = REQUEST;
			jsonString = jsonObject.dump();
			m_localBrockerObj->PublishData( jsonString, publishTopic );
		}
		
		else if( commandStr == CHANGE_DEVICE_MODE )
		{
			jsonObject[COMMAND_INFO][COMMAND_TYPE] = REQUEST;
			jsonString = jsonObject.dump();
			m_localBrockerObj->PublishData( jsonString, publishTopic );
		}
		
		else if( commandStr == SET_DEVICE_RULES )
		{
			jsonObject[COMMAND_INFO][COMMAND_TYPE] = REQUEST;
			jsonString = jsonObject.dump();
			m_localBrockerObj->PublishData( jsonString, publishTopic );
		}
		
		else if( commandStr == REPLACE_DEVICE_RULES )
		{
			jsonObject[COMMAND_INFO][COMMAND_TYPE] = REQUEST;
			jsonString = jsonObject.dump();
			m_localBrockerObj->PublishData( jsonString, publishTopic );
		}
		
		else if( commandStr == DELETE_DEVICE_RULES )
		{
			jsonObject[COMMAND_INFO][COMMAND_TYPE] = REQUEST;
			jsonString = jsonObject.dump();
			m_localBrockerObj->PublishData( jsonString, publishTopic );
		}
        else if( commandStr == REGISTER_SLAVES || commandStr == SET_SLAVE_CONFIGURATION || commandStr == RESET_SENSOR_BOARD 
                || commandStr == RESET_MESH_NETWORK || commandStr == RESET_SENSOR || commandStr == SET_BOARD_CONFIGURATION
                 || commandStr == RESET_BLUENRG || commandStr == SET_POSITION_CONFIGURATION || commandStr == SET_MOP_CONFIGURATION
                  || commandStr == READ_SENSOR_PROPERTY || commandStr == SET_SENSOR_FOTA_MODE )                
		{
			
			jsonObject[COMMAND_INFO][COMMAND_TYPE] = REQUEST;
			jsonString = jsonObject.dump();
			m_localBrockerObj->PublishData( jsonString, publishTopic );
		}
		else if ( commandStr == UPLOAD_LOG_FILES )
		{
			UploadLogFiles();
		}
		else
		{
			logg.str("");
			logg << "GatewayAgentManager::ExecuteCloudCommand()  GatewayID : " << m_gatewayId << ",  Message : Invalid Command received from cloud. Json : " << jsonObject;
			m_exceptionLoggerObj->LogError( logg.str() );
			//***send error msg to cloud
		}
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "GatewayAgentManager::ExecuteCloudCommand()  GatewayID : " << m_gatewayId << ",  Message : Exception found in received json. JSON : " << jsonObject;
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "GatewayAgentManager::ExecuteCloudCommand()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured. JSON : " << jsonObject;
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief ValidateJson() 			:	This method will validate the received json.
 * 
 * @param nlohmann::json jsonObj	:	It contains cloud requested command in json format
 * @param int caseId				:	It represented which json should be validate.
 *
 * @return : It will return SUCCESS if json is validated otherwise return predefined errors.
 */
ResponseStruct GatewayAgentManager::ValidateJson( nlohmann::json jsonObj, int caseId )
{
	ResponseStruct responseStructObj;
	responseStructObj.status = SUCCESS;
	responseStructObj.responseMetaInformation = "Json validated successfully";
	std::stringstream logg;
	try
	{
		if( jsonObj.is_null() )
		{
			logg.str("");
			logg << "GatewayAgentManager::ValidateJson()  GatewayID : " << m_gatewayId << ",  Message : Empty Json received.";
			m_exceptionLoggerObj->LogError( logg.str() );
			
			responseStructObj.status = FAILURE;
			responseStructObj.responseMetaInformation = "Received empty json from cloud";
			return responseStructObj;
		}
        
        if( !jsonObj.contains( SUB_JOB_ID ) )
        {
            responseStructObj.status = FAILURE;
            responseStructObj.responseMetaInformation = "'sub_job_id' key not found in received payload";
            
            logg.str("");
            logg << "GatewayAgentManager::ValidateJson()  GatewayID : " << m_gatewayId << ",  Message : 'sub_job_id' key not found in jsonObject. JSON : " << jsonObj;
            m_exceptionLoggerObj->LogError( logg.str() );
            return responseStructObj;
        }
        
		
		switch( caseId )
		{
			case INSTALL:
			case UPGRADE:
			case DELETE:
				{
					if( !jsonObj.contains( APP_NAME ) )
					{
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'app_name' key not found in received payload";
						
						logg.str("");
						logg << "GatewayAgentManager::ValidateJson()  GatewayID : " << m_gatewayId << ",  Message : 'app_name' key not found in jsonObject. JSON : " << jsonObj;
						m_exceptionLoggerObj->LogError( logg.str() );
						//return responseStructObj;
					}
					
					if( !jsonObj.contains( VERSION ) )
					{
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'version' key not found in received payload";
						//return responseStructObj;
					}
					
					if( !jsonObj.contains( URL ) )
					{
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'url' key not found in received payload";
						//return responseStructObj;
					}
					
					if( !jsonObj.contains( TOKEN ) )
					{
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'token' key not found in received payload";
						//return responseStructObj;
					}
				}
				break;
			case START_APPLICATION:
			case STOP_APPLICATION:
			case RESTART_APPLICATION:
				{
					if( !jsonObj.contains( APP_NAME ) )
					{
						logg.str( std::string() );
						logg << "GatewayAgentManager::ValidateJson()  GatewayID : " << m_gatewayId << ",  Message : 'app_name' key not found in respected json. JSON : " << jsonObj; 
						m_exceptionLoggerObj->LogError( logg.str() );
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'app_name' key not found in received payload";
						//return responseStructObj;
					}
					
					if( !jsonObj.contains( COMMAND ) )
					{
						logg.str( std::string() );
						logg << "GatewayAgentManager::ValidateJson()  GatewayID : " << m_gatewayId << ",  Message : 'command' key not found in respected json. JSON : " << jsonObj; 
						m_exceptionLoggerObj->LogError( logg.str() );
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'command' key not found in received payload";
						//return responseStructObj;
					}
				}
				break;
			case REGISTER_DEVICES_CASE:
			case DEREGISTER_DEVICES_CASE:
			case SET_POLLING_CONFIG_CASE:
			case GET_DEVICE_OR_GET_POS_CONFIG_CASE: //same for both GET_DEVICE_CONFIGURATION and READ_SENSOR_POSITION_CONFIGURATION
			case SET_VALUE_CANAGE_CASE:
				{
					if( !jsonObj.contains( APP_NAME ) )
					{
						logg.str( std::string() );
						logg << "GatewayAgentManager::ValidateJson()  GatewayID : " << m_gatewayId << ",  Message : 'app_name' key not found in respected json. JSON : " << jsonObj; 
						m_exceptionLoggerObj->LogError( logg.str() );
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'app_name' key not found in received payload";
						
					}
					
					if( !jsonObj.contains( COMMAND ) )
					{
						logg.str( std::string() );
						logg << "GatewayAgentManager::ValidateJson()  GatewayID : " << m_gatewayId << ",  Message : 'command' key not found in respected json. JSON : " << jsonObj; 
						m_exceptionLoggerObj->LogError( logg.str() );
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'command' key not found in received payload";
					}
					
					if( !jsonObj.contains( DEVICES ) )
					{
						logg.str( std::string() );
						logg << "GatewayAgentManager::ValidateJson()  GatewayID : " << m_gatewayId << ",  Message : 'devices' key not found in respected json. JSON : " << jsonObj; 
						m_exceptionLoggerObj->LogError( logg.str() );
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'devices' key not found in received payload";
					}
				}
				break;
            case SYSTEM_REBOOT:
				{
					if( !jsonObj.contains( REBOOT_TIMER ) )
					{
						logg.str( std::string() );
						logg << "GatewayAgentManager::ValidateJson()  GatewayID : " << m_gatewayId << ",  Message : 'reboot_timer' key not found in respected json. JSON : " << jsonObj; 
						m_exceptionLoggerObj->LogError( logg.str() );
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'reboot_timer' key not found in received payload";
						
					}
				}
				break;
			case SET_PROPERTIES_CASE:
				{
					
				}
				break;
			case CHANGE_DEVICE_MODE_CASE:
				{
					
				}
				break;
			case SET_DEVICE_RULES_CASE:
				{
					
				}
				break;
			case REPLACE_DEVICE_RULES_CASE:
				{
					
				}
				break;
			case DELETE_DEVICE_RULES_CASE:
				{
					
				}
				break;
            case REGISTER_SLAVES_CASE:
				{
					
				}
				break;
            case SET_SLAVE_CONFIGURATION_CASE:
				{
					
				}
				break;
            case RESET_SENSOR_BOARD_CASE:
				{
					
				}
				break;
            case RESET_MESH_NETWORK_CASE:
				{
					
				}
				break;
            case RESET_SENSOR_CASE:
				{
					
				}
				break;
            case SET_BOARD_CONFIGURATION_CASE:
				{
					
				}
				break;
                
            case SET_SENSOR_FOTA_CASE:
            {
                
            }
                break;
		}
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "GatewayAgentManager::ValidateJson()  GatewayID : " << m_gatewayId << ",  Message : Exception found in received json. JSON : " << jsonObj;
		m_exceptionLoggerObj->LogException( logg.str() );
		responseStructObj.status = FAILURE;
		responseStructObj.responseMetaInformation = "Exception occered in received Json.";
	}
	catch( ... )
	{
		logg.str("");
		logg << "GatewayAgentManager::ValidateJson()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured. JSON : " << jsonObj;
		m_exceptionLoggerObj->LogException( logg.str() );
		responseStructObj.status = FAILURE;
		responseStructObj.responseMetaInformation = "Unknown exception occered.";
	}
	
	return responseStructObj;
}

/**
 * @brief UploadLogFiles		:	This method will Upload gateway Exception Log Files
 * 
 * @return : It will return true if upload log files successfully otherwise return false.
 */
bool GatewayAgentManager::UploadLogFiles()
{
	std::stringstream logg;
	try
	{
        std::string zipCommand = "";
        
        #ifdef SIEMENS_GATEWAY
            zipCommand = "busybox zip -r ./logs/GatewayLogs.zip ./logs"; 
        #else
            zipCommand = "zip -r ./logs/GatewayLogs.zip ./logs";
        #endif
        
		std::string readFilepath = "/opt/IoT_Gateway/GatewayAgent/logs/GatewayLogs.zip" ;
		
		if ( system( zipCommand.c_str() ) == 0 )
		{
			logg.str( std::string() );
			logg << "GatewayAgentManager::UploadLogFiles()  GatewayID : " << m_gatewayId << ",  Message : Log file zipped successfully. ";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			std::string fileContent = ReadAndSetConfigurationInStr( readFilepath );
			std::string destFilePath = "logs" + GetCurrentDate() + "GatewayLogs.zip";

			if ( m_cloudConnectionObj->UploadBlobStorage( fileContent, destFilePath ) )
			{
				logg.str( std::string() );
				logg << "GatewayAgentManager::UploadLogFiles()  GatewayID : " << m_gatewayId << ",  Message : Log file Uploaded successfully ";
				m_exceptionLoggerObj->LogInfo( logg.str() );
			}
			else
			{
				logg.str( std::string() );
				logg << "GatewayAgentManager::UploadLogFiles()  GatewayID : " << m_gatewayId << ",  Message : Log file upload failed ";
				m_exceptionLoggerObj->LogError( logg.str() );
			}
			
			std::string removeZipFile = "rm -r " + readFilepath;
			if ( system( removeZipFile.c_str() ) == 0 )
			{
				logg.str( std::string() );
				logg << "GatewayAgentManager::UploadLogFiles()  GatewayID : " << m_gatewayId << ",  Message : Log zip file Removed Successfully ";
				m_exceptionLoggerObj->LogInfo( logg.str() );
			}
		}
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str( std::string() );
		logg << "GatewayAgentManager::UploadLogFiles()  GatewayID : " << m_gatewayId << ",  Message : Exception found in received json.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str( std::string() );
		logg << "GatewayAgentManager::UploadLogFiles()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}


void GatewayAgentManager::SendNotificationToCloud( ResponseStruct responseStructObj, std::string subJobId )
{
	nlohmann::json responseErrorInfoJson;
	std::stringstream logg;
	
	if( subJobId != "" )
	{
		responseErrorInfoJson[SUB_JOB_ID] = subJobId;
	}
	
	if( responseStructObj.status == FAILURE )
	{
		responseErrorInfoJson[STATUS] = "failure";
        responseErrorInfoJson[TYPE] = "error";
	}
	else
	{
		responseErrorInfoJson[STATUS] = "success";
        responseErrorInfoJson[TYPE] = "notification";
	}
	
	responseErrorInfoJson[TIMESTAMP] = GetTimeStamp();
	responseErrorInfoJson[MESSAGE] = responseStructObj.responseMetaInformation;
	
	std::string msg = responseErrorInfoJson.dump();
	if ( m_cloudConnectionObj->SendDeviceToCloudMessage( (char *)msg.c_str(), "notification" ) )
	{
		logg.str( std::string() );
		logg << "GatewayAgentManager::SendNotificationToCloud()  GatewayID : " << m_gatewayId << ",  Message : Send notification successfully. JSON : " << responseErrorInfoJson;
		m_exceptionLoggerObj->LogInfo( logg.str() );
	}
	else
	{
		logg.str( std::string() );
		logg << "GatewayAgentManager::SendNotificationToCloud()  GatewayID : " << m_gatewayId << ",  Message : Send notification failed. JSON : " << responseErrorInfoJson;
		m_exceptionLoggerObj->LogError( logg.str() );
	}
}
