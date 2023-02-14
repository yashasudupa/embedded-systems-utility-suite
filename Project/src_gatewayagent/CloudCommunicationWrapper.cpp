#include "CloudCommunicationWrapper.h"
#include "certs.h"

/**
 * @brief Create an CloudCommunicationWrapper : it will read the configuration file and 
 * try to connect with cloud
 */
CloudCommunicationWrapper::CloudCommunicationWrapper():
    m_connectionStatus(false),
	m_deviceHandle(NULL),
	m_twinVersion(-1),
	m_directMethodFlag( false ),
	m_restartGatewayFlag( true ),
    m_systemRebootTime(3600)
{
	std::stringstream logg;
	m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
	try
	{
		IoTHub_Init();
		m_configuration = ReadAndSetConfiguration( CONFIGURATIONFILE );
		if( m_configuration.contains( CLOUD_DETAILS ) )
		{
			m_connectionString = "HostName=";
			m_connectionString += m_configuration[CLOUD_DETAILS][HOST_NAME];
			m_connectionString += ";DeviceId=";
			m_gatewayId = m_configuration[CLOUD_DETAILS][DEVICEID];
			m_connectionString += m_gatewayId;
			m_connectionString += ";SharedAccessKey=";
			m_connectionString += m_configuration[CLOUD_DETAILS][SHARED_ACCESS_KEY];
			m_cloudAppName = m_configuration[CLOUD_DETAILS][CLOUD_APP];
			std::cout << "Connection string is created successfully " << std::endl;
			GetTwinVersion();
			std::cout << "GetTwinVersion() is working successfully " << std::endl;
			ConnectToCloud(); //chech in while 5 times.
			std::cout << "ConnectToCloud() is working successfully " << std::endl;
		}
        
		nlohmann::json gwRebootJson;
		gwRebootJson = ReadAndSetConfiguration( GATEWAY_REBOOT_PERSISTENCY_CONFIG );
		
		std::cout << "gwRebootJson is working successfully " << std::endl;
		
		if( gwRebootJson.is_null() )
		{
			gwRebootJson[REBOOT_TIMER] = m_systemRebootTime;
			WriteConfiguration( GATEWAY_REBOOT_PERSISTENCY_CONFIG, gwRebootJson );
			std::cout << "gwRebootJson is NULL " << std::endl;

		}
		else
        {
            m_systemRebootTime = gwRebootJson[REBOOT_TIMER];
			std::cout << "gwRebootJson is not NULL " << std::endl;

        }
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "CloudCommunicationWrapper::CloudCommunicationWrapper()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "CloudCommunicationWrapper::CloudCommunicationWrapper()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief destroy an CloudCommunicationWrapper  
 *
 */
CloudCommunicationWrapper::~CloudCommunicationWrapper()
{
    if( m_deviceHandle )
	{
        IoTHubDeviceClient_Destroy( m_deviceHandle );
	}
    if( m_messageHandle )
	{
        IoTHubMessage_Destroy( m_messageHandle );
	}
    if( m_devicellHandle )
	{
        IoTHubDeviceClient_LL_Destroy( m_devicellHandle );
	}
	
    IoTHub_Deinit();
}

/**
 * @brief ConnectToCloud() : 	Create Communication with cloud and create Communication handle.
 * 								It also set the callbacks for Direct method, Twincallback, C2D message,
 * 
 */
bool CloudCommunicationWrapper::ConnectToCloud()
{
	std::stringstream logg;
	try
	{
		m_deviceHandle = IoTHubDeviceClient_CreateFromConnectionString( (const char*)m_connectionString.c_str(), MQTT_Protocol );
		if ( m_deviceHandle == NULL )
		{
			logg.str("");
			logg << "CloudCommunicationWrapper::ConnectToCloud()  GatewayID : " << m_gatewayId << ",  Message : Create cloud connection handle failed(m_deviceHandle)";
			m_exceptionLoggerObj->LogError( logg.str() );
			std::cout << "CloudCommunicationWrapper::ConnectToCloud() failed " << std::endl;

		}
		else
		{
			IoTHubDeviceClient_SetOption( m_deviceHandle, OPTION_TRUSTED_CERT, certificates );
			IoTHubDeviceClient_SetConnectionStatusCallback( m_deviceHandle, CloudCommunicationWrapper::ConnectionStatusCallback, (void*)this );
			IoTHubDeviceClient_SetDeviceMethodCallback( m_deviceHandle, DirectMethodCallback,(void*)this );
			(void)IoTHubDeviceClient_SetDeviceTwinCallback( m_deviceHandle, DeviceTwinCallback, (void*)this );
			IoTHubDeviceClient_SetMessageCallback( m_deviceHandle, CloudToDeviceCallback, (void*)this );
            
			std::cout << "CloudCommunicationWrapper::ConnectToCloud() success " << std::endl;
			
			logg.str("");
			logg << "CloudCommunicationWrapper::ConnectToCloud()  GatewayID : " << m_gatewayId << ",  Message : Create Handle Successfully(m_deviceHandle)";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			return true;
		}
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "CloudCommunicationWrapper::ConnectToCloud()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "CloudCommunicationWrapper::ConnectToCloud()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	return false;
}

/**
 * @brief SendDeviceToCloudMessage() : This function will send Device to cloud message in async mode.
 * Asynchronous call to send the message specified by eventMessageHandle.
 * @param char* message : The message format which it sent to the IOT hub(cloud).
 * @param char* schema 	: set the type of message like telemetry, alert, notification etc.
 * @param std::string correlationId : send a reply to any of the message using message Id/correlation Id.
 * 									  It is optional field.
 * @return If message send Successfully it will return true otherwise return false.
 */
bool CloudCommunicationWrapper::SendDeviceToCloudMessage( char* message, char* schema, std::string correlationId )
{
	int retValue = false;
	std::stringstream logg;
	std::string msgSrc = "";
	try
	{
		if( m_deviceHandle && message != NULL ) // test
		{
			nlohmann::json configurationJson;
			configurationJson = nlohmann::json::parse( message );
			if( !configurationJson["message_source"].is_null() )
			{
				msgSrc = configurationJson["message_source"];
			}
			
			m_messageHandle = IoTHubMessage_CreateFromString( (char *) message );
			//(void)IoTHubMessage_SetCorrelationId( m_messageHandle, correlationId.c_str() );
			(void)IoTHubMessage_SetContentTypeSystemProperty( m_messageHandle, "application%2fjson" );
			(void)IoTHubMessage_SetContentEncodingSystemProperty( m_messageHandle, "utf-8" );

			// Set application properties
			time_t now = time(0);
			struct tm* timeInfo;
			timeInfo = gmtime( &now );
			char timeBuff[50];
			strftime(timeBuff, 50, "%Y-%m-%dT%H:%M:%SZ", timeInfo );
			
			MAP_HANDLE propMap = IoTHubMessage_Properties( m_messageHandle );
			(void)Map_AddOrUpdate( propMap, "$$MessageSchema", schema );
			(void)Map_AddOrUpdate( propMap, "$$ContentType", "JSON" );
			(void)Map_AddOrUpdate( propMap, "$$CreationTimeUtc", timeBuff );
			if( msgSrc != "" )
			{
				(void)Map_AddOrUpdate( propMap, "message_source", (char *)msgSrc.c_str() );
			}
			
			IOTHUB_CLIENT_RESULT result = IoTHubDeviceClient_SendEventAsync( m_deviceHandle, m_messageHandle, CloudCommunicationWrapper::SendConfirmCallback, (void*)this );
				
			if( result == IOTHUB_CLIENT_OK )
			{
				retValue = true;
				std::cout << "CloudCommunicationWrapper::SendDeviceToCloudMessage IOTHUB_CLIENT_OK" << std::endl;
			}
			else
			{
				const char* retstr =  IOTHUB_CLIENT_RESULTStrings( result );
				logg.str("");
				logg << "CloudCommunicationWrapper::SendDeviceToCloudMessage()  GatewayID : " << m_gatewayId << ",  Message : Send Device to cloud message failed. Error Info : " << retstr;
				m_exceptionLoggerObj->LogError( logg.str() );
				std::cout << "CloudCommunicationWrapper::SendDeviceToCloudMessage IOTHUB_CLIENT_NOTOK" << std::endl;
			}
		}
	}
	catch( ... )
	{
		logg.str("");
		logg << "CloudCommunicationWrapper::SendDeviceToCloudMessage()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	
    return retValue;  
}

/**
 * @brief SendReportedState() : This method sends a report of the device's properties and their current values.
 * 								It used for twin call back report state.
 * @param char *reportedProperties : The current device property values to be 'reported' to the IoTHub.
 *
 */
void CloudCommunicationWrapper::SendReportedState( char *reportedProperties )
{
	try
	{
		if( reportedProperties != NULL )
		{
			(void)IoTHubDeviceClient_SendReportedState(m_deviceHandle, (const unsigned char*)reportedProperties, strlen(reportedProperties), ReportedStateCallback, NULL);
			std::cout << "CloudCommunicationWrapper::SendReportedState" << std::endl;
		}
	}
	catch( ... )
	{
		std::stringstream logg;
		logg.str("");
		logg << "CloudCommunicationWrapper::SendReportedState()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	
}

/**
 * @brief ReportedStateCallback() : The callback specified by the device client to be called with the result of the transaction.
 * @param int status_code : send status code.
 * @param void* userContextCallback: User specified context that will be provided to the callback. This can be NULL.
 */
void CloudCommunicationWrapper::ReportedStateCallback( int status_code, void* userContextCallback )
{
    (void)userContextCallback;
    printf("Device Twin reported properties update completed with result: %d\r\n", status_code);
}

/**
 * @brief UploadBlobStorage() : This method upload the cached data to the cloud.
 * 
 * @param std::string message : The source of data.
 * @param std::string destFilePath : The name of the file to be created in Azure Blob Storage.
 * 									 Example <device_id>/<date>/<epochtime>.json
 * 									 KCMS_Dev_LD_53/2021/04/25/1619373592.json
 * 
 * @return : Return true if Successfully upload the file other wise return fail status.
 */
bool CloudCommunicationWrapper::UploadBlobStorage( std::string message, std::string destFilePath )
{
    bool ret = false;
    std::stringstream logg;
	try
	{
		if( m_deviceHandle && message != "" && destFilePath != "" ) //use string compaire
		{
			IOTHUB_CLIENT_RESULT blobRet = IoTHubClient_UploadToBlobAsync( m_deviceHandle, destFilePath.c_str(), (const unsigned char*)message.c_str(), message.length(),NULL,NULL );
			const char* retStr =  IOTHUB_CLIENT_RESULTStrings( blobRet );
			
			if( blobRet != IOTHUB_CLIENT_OK )
			{
				logg.str("");
				logg << "CloudCommunicationWrapper::UploadBlobStorage()  GatewayID : " << m_gatewayId << ",  Message : Upload Blob Storage file failed. Error Info : " << retStr << ", FileName : " << destFilePath;
				m_exceptionLoggerObj->LogError( logg.str() );
			}
			else
			{
				std::cout << "File upload successfully " << (long)blobRet << " : " << retStr << "\n\n";
				ret = true;
			}
		}
	}
	catch( ... )
	{
		logg.str("");
		logg << "CloudCommunicationWrapper::UploadBlobStorage()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
    return ret;
}

/**
 * @brief getConnectionStatus() : It will return the connection status.
 * @return : return true if connection established successfully otherwise return false.
 * 
*/
bool CloudCommunicationWrapper::GetConnectionStatus()
{
     return m_connectionStatus;
	 std::cout << "bool CloudCommunicationWrapper::GetConnectionStatus() " << std::endl;
}

/**
 * @brief SetConnectionStatus() : It will set the connection status in predecided json format.
 * 
*/
void CloudCommunicationWrapper::SetConnectionStatus( bool status )
{
	nlohmann::json jsonObj;
	jsonObj[COMMAND_INFO][COMMAND_TYPE] = "connection_status";
	jsonObj["connection_status"] = status;
	m_ConnectionStatusCB( jsonObj );
    m_connectionStatus = status;
	std::cout << "CloudCommunicationWrapper::SetConnectionStatus " << std::endl;
}

/**
 * @brief SetSystemRebootTime() : It will set the Sytem Reboot time if connection is not available for that time.
 * 
*/
void CloudCommunicationWrapper::SetSystemRebootTime(long systemreboot)
{
    nlohmann::json gwRebootJson;
       
    m_systemRebootTime = systemreboot;
		
    gwRebootJson[REBOOT_TIMER] = m_systemRebootTime;
    WriteConfiguration( GATEWAY_REBOOT_PERSISTENCY_CONFIG, gwRebootJson );
	std::cout << "CloudCommunicationWrapper::SetSystemRebootTime(long systemreboot) " << std::endl;
}

/**
 * @brief RegisterCloudRequestCB() 	: 	Register the callback. If C2D, Directmethod or Twin Callback will be
 *						 				received payload from cloud then this call back has been required for 
 *						 				further process.
*/
void CloudCommunicationWrapper::RegisterCloudRequestCB( std::function<void(nlohmann::json)> cb )
{
	m_cloudRequestCB = cb;
	std::cout << "CloudCommunicationWrapper::RegisterCloudRequestCB " << std::endl;
}

/**
 * @brief RegisterConnectionStatusCB() : Register connection status callback. If connection status has been changed  
 * 										 then this callback is required for inform connection status.
 * 
*/
void CloudCommunicationWrapper::RegisterConnectionStatusCB( std::function<void(nlohmann::json)> cb )
{
	std::cout << "CloudCommunicationWrapper::RegisterConnectionStatusCB " << std::endl;
	m_ConnectionStatusCB = cb;
}

/**
 * @brief SetJsonValue() : set value to the call back
 * 
 * @param nlohmann::json jsonObj : received C2D message, Direct method, twin call back message.
*/
void CloudCommunicationWrapper::SetJsonValue( nlohmann::json jsonObj )
{
	m_cloudRequestCB( jsonObj );
	std::cout << "CloudCommunicationWrapper::SetJsonValue( nlohmann::json jsonObj )" << std::endl;
}

/**
 * @brief SetTwinVersion() 		: 	Maintain Twin callback version. It will usefull for to check received
 * 							 		twin payload is new or old. Write the version in the file.
 * 
 * @param : long twinVersion 	: 	last received twin payload version number.
*/
void CloudCommunicationWrapper::SetTwinVersion( long twinVersion )
{
	std::ofstream writefile(TWINVERSIONFILE);
	if( writefile )
	{
 		writefile << twinVersion << std::endl;
		writefile.close();
	}
	
	m_twinVersion = twinVersion;	
	std::cout << "CloudCommunicationWrapper::SetTwinVersion" << std::endl;
}

/**
 * @brief GetTwinVersion() : Read the version file and set it to the member variable.
 * 
*/
void CloudCommunicationWrapper::GetTwinVersion()
{
	std::ifstream file( TWINVERSIONFILE );
	std::stringstream logg;
	if( file )
	{
		file >> m_twinVersion;
		logg.str("");
		logg << "CloudCommunicationWrapper::GetTwinVersion()  GatewayID : " << m_gatewayId << ",  Message : Get twin verion from file successfully";
		m_exceptionLoggerObj->LogInfo( logg.str() );
		file.close();
	}
	else
	{	
		logg.str("");
		logg << "CloudCommunicationWrapper::GetTwinVersion()  GatewayID : " << m_gatewayId << ",  Message : Failed to Get twin verion from file";
		m_exceptionLoggerObj->LogError( logg.str() );
	}
	std::cout << "CloudCommunicationWrapper::GetTwinVersion() " << std::endl;
}

/**
 * @brief GetGatewayId() : It will return the gatewayId which is defined by cloud application.
 * @return : return GatewayId.
 * 
*/
std::string CloudCommunicationWrapper::GetGatewayId()
{
	std::cout << "CloudCommunicationWrapper::GetGatewayId() " << std::endl;
	return m_gatewayId;
	
}

/**
 * @brief GetCloudAppName() : It will return the application name which is defined by cloud application.
 * @return : return cloud app name.
 * 
*/
std::string CloudCommunicationWrapper::GetCloudAppName()
{
	std::cout << "CloudCommunicationWrapper::GetCloudAppName() " << std::endl;
	return m_cloudAppName;
}

/**
 * @brief SendConfirmCallback() : The callback specified by the device for receiving confirmation of the delivery of the IoT Hub message.
 * 
 * @param IOTHUB_CLIENT_CONFIRMATION_RESULT result : send status(result) code.
 * @param void* userContextCallback: User specified context that will be provided to the callback. This can be NULL.
 */
void CloudCommunicationWrapper::SendConfirmCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
	CloudCommunicationWrapper *self = (CloudCommunicationWrapper*)(userContextCallback);
	std::stringstream logg;
	try
	{
		std::string resultMsg = MU_ENUM_TO_STRING( IOTHUB_CLIENT_CONFIRMATION_RESULT, result );
		if( resultMsg == "IOTHUB_CLIENT_CONFIRMATION_OK" )
		{
            logg.str("");
			logg << "CloudCommunicationWrapper::SendConfirmCallback()  GatewayID : " << self->m_gatewayId << ",  Message : Confirmation callback received for Device to Cloud message with result : " << resultMsg; 
			//self->m_exceptionLoggerObj->LogInfo( logg.str() );
            std::cout << logg.str() << std::endl;
		}
		else
		{
			logg.str("");
			logg << "CloudCommunicationWrapper::SendConfirmCallback()  GatewayID : " << self->m_gatewayId << ",  Message : ERROR callback received for Device to Cloud message with result : " << resultMsg; 
			self->m_exceptionLoggerObj->LogError( logg.str() );
		}
		std::cout << "CloudCommunicationWrapper::SendConfirmCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)" << std::endl;
	}
	catch( ... )
	{
		logg.str("");
		logg << "CloudCommunicationWrapper::SendConfirmCallback()  GatewayID : " << self->m_gatewayId << ",  Message : Unknown exception occured.";
		self->m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief SendNotificationToCloud()	: 	It will create the notification/error json and call the Device to cloud api.
								  
 * 
 * @param std::string message : it contains extra info.
 * @param std::string status : It contains the status as "success"/"failure".
 * @param size_t size : size of the payload message.
 * 
 */
void CloudCommunicationWrapper::SendNotificationToCloud( std::string message, std::string status )
{
    nlohmann::json responseErrorInfoJson;
	std::stringstream logg;
	if( status != "failure" )
	{
		responseErrorInfoJson[TYPE] = "error";
        responseErrorInfoJson[STATUS] = "failure";
	}
	else
	{
		responseErrorInfoJson[TYPE] = "notification";
        responseErrorInfoJson[STATUS] = "success";
	}
    
	std::cout << "CloudCommunicationWrapper::SendNotificationToCloud( std::string message, std::string status ) result, void* userContextCallback)" << std::endl;
	
	responseErrorInfoJson[TIMESTAMP] = GetTimeStamp();
	responseErrorInfoJson[MESSAGE] = message;
	
	std::string msg = responseErrorInfoJson.dump();
	if ( SendDeviceToCloudMessage( (char *)msg.c_str(), "notification" ) )
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
	std::cout << "GatewayAgentManager::SendNotificationToCloud()  GatewayID :" << std::endl;
}

/**
 * @brief DeviceTwinCallback() 			: 		The callback specified by the device client to be used for updating the desired state. The callback
 * 								    			will be called in response to a request send by the IoTHub services. The payload will be passed to 
 * 								  				the callback, along with two version numbers:
								  
 * 
 * @param DEVICE_TWIN_UPDATE_STATE updateState : send status(updateState) code.
 * @param const unsigned char* payLoad : it is nothing but cloud request message.
 * @param size_t size : size of the payload message.
 * 
 * @param void* userContextCallback: User specified context that will be provided to the callback. This can be NULL.
 */
void CloudCommunicationWrapper::DeviceTwinCallback( DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char* payLoad, size_t size, void* userContextCallback)
{
	CloudCommunicationWrapper *self = (CloudCommunicationWrapper*)userContextCallback;
	std::stringstream logg;
	try
	{	
		std::string payloadString = ( reinterpret_cast< char const* >(payLoad) ) ;
		payloadString = payloadString.substr(0,size);	
		
		nlohmann::json obj = nlohmann::json::parse(payloadString.c_str());
		
		nlohmann::json Tempobj;
		long receivedVersion;
		
		if (updateState == DEVICE_TWIN_UPDATE_COMPLETE)
		{
			Tempobj = obj[DESIRED][PACKAGE_MANAGEMENT];
			receivedVersion = obj[DESIRED][DOLLER_VERSION];
		}
		else
		{
			Tempobj = obj[PACKAGE_MANAGEMENT];
			receivedVersion = obj[DOLLER_VERSION];
		}
		Tempobj[PACKAGE_DETAILS][SUB_JOB_ID] = obj[SUB_JOB_ID];
		if( self->m_twinVersion != receivedVersion )
		{
			if( !obj.contains(SUB_JOB_ID) )
			{
				std::string errMsg = "sub_job_id key not found in received payload.";
				self->SendNotificationToCloud( errMsg, "failure" );
				logg << "CloudCommunicationWrapper::DeviceTwinCallback()  GatewayID : " << self->m_gatewayId << ",  Message : 'sub_job_id' key not found in received payload. Payload : " << payloadString;
				self->m_exceptionLoggerObj->LogError( logg.str() );
				return;
			}
			
			logg.str("");
			logg << "CloudCommunicationWrapper::DeviceTwinCallback()  GatewayID : " << self->m_gatewayId << ",  Message : Received new Payload. Payload : " << Tempobj;
			self->m_exceptionLoggerObj->LogInfo( logg.str() );
			// Providing desired value Json to AppManager.
			self->m_cloudRequestCB( Tempobj );
			self->SetTwinVersion( receivedVersion );
			std::cout << "CloudCommunicationWrapper::DeviceTwinCallback()  GatewayID" << std::endl;
		}
		else
		{
			logg.str("");
			logg << "CloudCommunicationWrapper::DeviceTwinCallback()  GatewayID : " << self->m_gatewayId << ",  Message : Received old Payload.";
			self->m_exceptionLoggerObj->LogInfo( logg.str() );
		}
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "CloudCommunicationWrapper::DeviceTwinCallback()  GatewayID : " << self->m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		self->m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "CloudCommunicationWrapper::DeviceTwinCallback()  GatewayID : " << self->m_gatewayId << ",  Message : Unknown exception occured.";
		self->m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief DirectMethodCallback() :	The callback which will be called by IoTHub. 
 * 									Direct methods represent a request-reply interaction with a 
 * 									device similar to an HTTP call in that they succeed or fail 
 * 									immediately (after a user-specified timeout)
 * 
 * @param const char* methodName	:	unique method name.
 * @param const unsigned char* payLoad 	:	it is nothing but cloud request message.
 * @param size_t size : size of the payload message.
 * @param unsigned char** response	:	it is filled with response message(json).
 * @param size_t* responseSize	:	Size of response Message.
 * @param void* userContextCallback	:	User specified context that will be provided to the callback.
										This can be NULL.
 */	
int CloudCommunicationWrapper::DirectMethodCallback( const char* methodName, const unsigned char* payLoad, size_t size, unsigned char** response, size_t* responseSize, void* userContextCallback)
{
    CloudCommunicationWrapper *self = (CloudCommunicationWrapper*)userContextCallback;
	std::stringstream logg;
	int result;
	try
	{
		nlohmann::json startUpMsg;
		startUpMsg[GATEWAY_ID] = self->m_gatewayId;
		startUpMsg[TIMESTAMP] = GetTimeStamp();
		
		std::string payloadString = ( reinterpret_cast< char const* >(payLoad) ) ;
		payloadString = payloadString.substr(0,size);
		nlohmann::json obj = nlohmann::json::parse(payloadString.c_str());
        obj[COMMAND] = methodName;
		std::string methodNameStr = methodName;
        
        if( !obj.contains(SUB_JOB_ID) )
        {
            result = -1;
            std::string errMsg = "sub_job_id key not found in received payload.";
            *responseSize = errMsg.length();
            *response = (unsigned char *)strdup( errMsg.c_str() );
            logg << "CloudCommunicationWrapper::DirectMethodCallback()  GatewayID : " << self->m_gatewayId << ",  Message : 'sub_job_id' key not found in received payload. Payload : " << payloadString << ", MethodName : " << methodNameStr;
            self->m_exceptionLoggerObj->LogError( logg.str() );
            return result;
        }
		
		transform( methodNameStr.begin(), methodNameStr.end(), methodNameStr.begin(), ::tolower ); 
		
		logg << "CloudCommunicationWrapper::DirectMethodCallback()  GatewayID : " << self->m_gatewayId << ",  Message : Received DirectMethodCallback message. Payload : " << payloadString << ", MethodName : " << methodNameStr;
		self->m_exceptionLoggerObj->LogInfo( logg.str() );
		startUpMsg[SUB_JOB_ID] = obj[SUB_JOB_ID];
		if( strcmp( TEST_CONNECTION_GATEWAY, methodNameStr.c_str() ) == 0 )
		{
            
			startUpMsg[STATUS] = "connected";
			result = 200;
		}
		else if ( strcmp( "START_APP", methodName ) == 0 || ( strcmp( "RESTART_APP", methodName ) == 0 ) || ( strcmp( "STOP_APP", methodName ) == 0 ) )
		{
			self->m_exceptionLoggerObj->LogInfo( logg.str() );
			self->SetJsonValue( obj );
			self->m_mtx.lock();
			
			std::thread waitingThread = std::thread([self](){
				while( !self->m_directMethodFlag )
				{
					usleep(100);
				}
			});
			
			if ( self->m_directMethodStatus == SUCCESS )
			{
				result = 200;
				startUpMsg[MESSAGE] = self->m_appStatusMessage;
				std::cout << " ********** "<< self->m_appStatusMessage << "\n\n";
			}
			else
			{
				result = -1;
				startUpMsg[MESSAGE] = self->m_appStatusMessage;
			}
			
			waitingThread.detach();
			
			self->m_directMethodFlag = false;
			self->m_mtx.unlock();
		}
		else
		{
			self->SetJsonValue( obj );
			startUpMsg[MESSAGE] = "Message received Successfully";
			result = 200;
		}
		
		char finalMsg[4000];
		std::string msg = startUpMsg.dump();
		sprintf( finalMsg,"%s",msg.c_str() );

		*responseSize = strlen( finalMsg );
		*response = (unsigned char *)strdup(finalMsg);
		(void)memcpy(*response, finalMsg, *responseSize);
		std::cout << "CloudCommunicationWrapper::DirectMethodCallback( const char* methodName, const unsigned char* payLoad, size_t size, unsigned char** response, size_t* responseSize, void* userContextCallback)" << std::endl;
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "CloudCommunicationWrapper::DirectMethodCallback  GatewayID : " << self->m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		self->m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "CloudCommunicationWrapper::DirectMethodCallback  GatewayID : " << self->m_gatewayId << ",  Message : Unknown exception occured.";
		self->m_exceptionLoggerObj->LogException( logg.str() );
	}	
    return result;
}

/**
 * @brief ConnectionStatusCallback() :	The callback which will be called by IoTHub. 
 * 										ConnectionStatusCallback method represent the callback specified by the
										device for receiving updates about the status of the connection to IoT Hub.
 * 
 * @param IOTHUB_CLIENT_CONNECTION_STATUS result	:	it represent the connection status(result) code.
 * @param IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason	:	it represent the connection status(result) message
 * @param void* userContext	:	User specified context that will be provided to the callback.
 *								This can be NULL.
 */	
void CloudCommunicationWrapper::ConnectionStatusCallback( IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* userContext )
{
    time_t seconds;
    static int rebootIndex = 0;
	CloudCommunicationWrapper *self = (CloudCommunicationWrapper*)(userContext);
	std::string resultMsg = MU_ENUM_TO_STRING( IOTHUB_CLIENT_CONNECTION_STATUS, result );
	std::stringstream logg;
	try
	{
        if (rebootIndex == 0)
        {
            self->m_systemRebootTimeSec = time (NULL);
        }
		if ( result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED )
		{
			logg.str("");
			logg << "CloudCommunicationWrapper::ConnectionStatusCallback()  GatewayID : " << self->m_gatewayId << ",  Message : The device client is connected to iothub.";
			self->m_exceptionLoggerObj->LogInfo( logg.str() );
			self->SetConnectionStatus(1);
            rebootIndex = 0;
			
			if( self->m_restartGatewayFlag )
			{
				self->m_restartGatewayFlag = false;
				self->SendGatewayRestartNotification();
			}
		}
		else
		{
			logg.str("");
			logg << "CloudCommunicationWrapper::ConnectionStatusCallback()  GatewayID : " << self->m_gatewayId << ",  Message : The device client has been disconnected. Error Message : " << resultMsg ;
			self->m_exceptionLoggerObj->LogError( logg.str() );
			self->SetConnectionStatus(0);
            
            seconds = time (NULL);
            if(rebootIndex && ( seconds - self->m_systemRebootTimeSec ) > self->m_systemRebootTime)
            {
                logg.str("");
                logg << "CloudCommunicationWrapper::ConnectionStatusCallback()  rebootIndex : " << rebootIndex << ",  Message : Rebooting Complete System\n";
                self->m_exceptionLoggerObj->LogInfo( logg.str() );
                
                rebootIndex = 0;
                std::string rebootSystemCmd = "reboot";
                system( rebootSystemCmd.c_str() );
            }
            if(!rebootIndex)
            {
                rebootIndex ++;
            }
		}
		std::cout << "CloudCommunicationWrapper::ConnectionStatusCallback()  rebootIndex : ,  Message : Rebooting Complete System\n" << std::endl;
	}
	catch( ... )
	{
		logg.str("");
		logg << "CloudCommunicationWrapper::ConnectionStatusCallback()  GatewayID : " << self->m_gatewayId << ",  Message : Unknown exception occured.";
		self->m_exceptionLoggerObj->LogException( logg.str() );
	}
}


void CloudCommunicationWrapper::SendGatewayRestartNotification()
{
	std::stringstream logg;
	try
	{
		bool restartFlag = false;
		std::string fileContent = ReadAndSetConfigurationInStr( RESTART_GATEWAY_FILE );

		if( strstr( fileContent.c_str(),"gateway_restart" ) )
		{
			restartFlag = true;
		}
		else
		{
			
		}

		if( unlink( RESTART_GATEWAY_FILE ) != 0 )
		{
			/*char msg[1000];
			sprintf(msg, "CloudCommunicationWrapper::SendGatewayRestartNotification\tDeviceID=%d\tMessage={File=%s cannot be deleted.}",itsDeviceID,filenm.c_str());
			GlobalExceptionLogger.logException( msg );*/
		}
		else
		{
			/*char msg[1000];
			sprintf(msg, "DelayedDataProcess::run()\tDeviceID=%d\tMessage={File=%s deleted successfully}",itsDeviceID,filenm.c_str());
			GlobalExceptionLogger.logException( msg );*/
		}
		
		nlohmann::json restartInfoJson;
		restartInfoJson[TIMESTAMP] = GetTimeStamp();
		restartInfoJson[TYPE] = "notification";
		
		
		if( restartFlag )
		{
			restartInfoJson[MESSAGE] = "Gateway has restarted";
			restartInfoJson[EVENT] = "gateway_restart";
		}
		else
		{
			restartInfoJson[MESSAGE] = "GatewayAgent application has restarted";
			restartInfoJson[EVENT] = "application_restart";
		}
		
		std::string msg = restartInfoJson.dump();
		if ( SendDeviceToCloudMessage( (char *)msg.c_str(), "notification" ) )
		{
			logg.str( std::string() );
			logg << "CloudCommunicationWrapper::SendGatewayRestartNotification  GatewayID : " << m_gatewayId << ",  Message : Send notification successfully. JSON : " << restartInfoJson;
			m_exceptionLoggerObj->LogInfo( logg.str() );
		}
		else
		{
			logg.str( std::string() );
			logg << "CloudCommunicationWrapper::SendGatewayRestartNotification  GatewayID : " << m_gatewayId << ",  Message : Send notification failed. JSON : " << restartInfoJson;
			m_exceptionLoggerObj->LogError( logg.str() );
		}
		std::cout << "CloudCommunicationWrapper::SendGatewayRestartNotification()" << std::endl;
	}
	catch( ... )
	{
		logg.str("");
		logg << "CloudCommunicationWrapper::SendGatewayRestartNotification()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief SetDirectMethodResponse() :	If Direct method response set from GatewayAgent then it need to
 * 										set this flag for sending response to the cloud.
 *
 */ 
void CloudCommunicationWrapper::SetDirectMethodResponse( ResponseStruct responseStructObj )
{
	m_directMethodFlag = true;
	m_directMethodStatus = responseStructObj.status;
	m_appStatusMessage = responseStructObj.responseMetaInformation;
	std::cout << "CloudCommunicationWrapper::SetDirectMethodResponse( ResponseStruct responseStructObj )" << std::endl;
}

/**
 * @brief CloudToDeviceCallback() :	The callback which will be called by IoTHub. 
 * 									Sets up the message callback to be invoked when IoT Hub issues
 * 									a message to the device. This is a blocking call.
 * 
 * @param IOTHUB_MESSAGE_HANDLE message	:	it represnt the message handle. it includes message,
 *											MessageId, and correlationId.
 * @param void* userContext	:	User specified context that will be provided to the callback.
 *								This can be NULL.
 */	
IOTHUBMESSAGE_DISPOSITION_RESULT CloudCommunicationWrapper::CloudToDeviceCallback( IOTHUB_MESSAGE_HANDLE message, void* userContext )
{
	
    const char* messageId;
    const char* correlationId;
	std::stringstream logg;
    CloudCommunicationWrapper *self = (CloudCommunicationWrapper*)( userContext );
    try
	{
		if ( ( messageId = IoTHubMessage_GetMessageId( message ) ) == NULL )
		{
			messageId = "<unavailable>";
		}
		if ( ( correlationId = IoTHubMessage_GetCorrelationId( message ) ) == NULL )
		{
			correlationId = "<unavailable>";
		}
		
		if ( IoTHubMessage_GetContentType( message ) == IOTHUBMESSAGE_BYTEARRAY )
		{
			const unsigned char* buffMessage;
			size_t buffLength;
			if ( IoTHubMessage_GetByteArray( message, &buffMessage, &buffLength ) != IOTHUB_MESSAGE_OK)
			{
				logg.str("");
				logg << "CloudCommunicationWrapper::CloudToDeviceCallback()  GatewayID : " << self->m_gatewayId << ",  Message : Failure retrieving byte array message.";
				self->m_exceptionLoggerObj->LogError( logg.str() );
			}
			else
			{
				std::string crid = messageId;
				std::string payloadString = ( reinterpret_cast< char const* >(buffMessage) ) ;
				payloadString = payloadString.substr( 0, buffLength );
				nlohmann::json jsonPayloadObj = nlohmann::json::parse( payloadString.c_str() );
                
                if( !jsonPayloadObj.contains(SUB_JOB_ID) )
                {
                    std::string errMsg = "sub_job_id key not found in received payload.";
                    self->SendNotificationToCloud( errMsg, "failure" );
                    logg.str("");
                    logg << "CloudCommunicationWrapper::CloudToDeviceCallback()  GatewayID : " << self->m_gatewayId << ",  Message : 'sub_job_id' key not found in received payload. Payload : " << payloadString;
                    self->m_exceptionLoggerObj->LogError( logg.str() );
                    return IOTHUBMESSAGE_ACCEPTED;
                }
                
				jsonPayloadObj[CORELATION_ID] = crid ;
				logg.str("");
				logg << "CloudCommunicationWrapper::CloudToDeviceCallback()  GatewayID : " << self->m_gatewayId << ",  Message : Received C2D message. Json is : " << jsonPayloadObj;
				self->m_exceptionLoggerObj->LogInfo( logg.str() );
				self->SetJsonValue( jsonPayloadObj );
			}
		}
		else
		{
			logg.str("");
			logg << "CloudCommunicationWrapper::CloudToDeviceCallback()  GatewayID : " << self->m_gatewayId << ",  Message : Failure retrieving byte array message.";
			self->m_exceptionLoggerObj->LogError( logg.str() );
		}
		
		const char* property_value = "property_value";
		const char* property_key = IoTHubMessage_GetProperty( message, property_value );
	}
	catch( ... )
	{
		logg.str("");
		logg << "CloudCommunicationWrapper::CloudToDeviceCallback()  GatewayID : " << self->m_gatewayId << ",  Message : Unknown exception occured.";
		self->m_exceptionLoggerObj->LogException( logg.str() );
	}
	
	std::cout << "IOTHUBMESSAGE_DISPOSITION_RESULT CloudCommunicationWrapper::CloudToDeviceCallback( IOTHUB_MESSAGE_HANDLE message, void* userContext ) " << std::endl;
	
    return IOTHUBMESSAGE_ACCEPTED;
}
