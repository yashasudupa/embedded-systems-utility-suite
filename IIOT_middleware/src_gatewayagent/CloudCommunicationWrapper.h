#ifndef CloudCommunicationWrapper_h
#define CloudCommunicationWrapper_h 1

#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <functional>
#include <math.h>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <sys/reboot.h>
#include <bits/stdc++.h>
#include <sys/time.h>
#include <fstream>

#include "iothub.h"
#include "iothub_client.h"
#include "iothub_device_client.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "iothubtransportmqtt.h"
#include "iothubtransporthttp.h"
#include "parson.h"
#include "Common.h"
#include "GlobalOperations.h"
#include "ExceptionLogger.h"


class CloudCommunicationWrapper
{
    
public:
    CloudCommunicationWrapper();
    ~CloudCommunicationWrapper();
	
	bool ConnectToCloud();
	bool ConnectToCloudForBlobUpload();
	bool SendDeviceToCloudMessage( char* message, char* schema, std::string correlationId = "CORE_ID" );
    bool UploadBlobStorage( std::string message,std::string destFilePath );
    bool GetConnectionStatus();
	
	void SetConnectionStatus( bool status );
	void SendReportedState( char *reportedProperties );
	void RegisterCloudRequestCB( std::function<void(nlohmann::json)> cb );
	void RegisterConnectionStatusCB( std::function<void(nlohmann::json)> cb );
	void SetDirectMethodResponse( ResponseStruct responseStructObj );
	
	std::string GetGatewayId();
	std::string GetCloudAppName();
    
    void SetSystemRebootTime(long);
	
	//Static members
	static void ConnectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result,
				IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* userContext);
				
	static void SendConfirmCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, 
			void* userContextCallback);
			
	static int DirectMethodCallback(const char* methodName, const unsigned char* payLoad, 
			size_t size,unsigned char** response, size_t* responseSize,
			void* userContextCallback);
			
	static void DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE update_state, 
			const unsigned char* payLoad, size_t size, void* userContextCallback);
			
	static IOTHUBMESSAGE_DISPOSITION_RESULT CloudToDeviceCallback(IOTHUB_MESSAGE_HANDLE message,
				void* user_context1);
			
	static void ReportedStateCallback(int status_code, void* userContextCallback);
	
private:
	void SetJsonValue( nlohmann::json jsonObj );
	void SetTwinVersion( long  twinVersion);
	void GetTwinVersion();
    void SendNotificationToCloud( std::string message, std::string status );
	void SendGatewayRestartNotification();
			
private:

    std::string m_connectionString;
    std::string m_gatewayId;
    std::string m_cloudAppName;
    std::string m_appStatusMessage;
	
    bool m_connectionStatus;
    bool m_msgSendStatus;
	bool m_directMethodFlag;
	bool m_directMethodStatus;
	bool m_restartGatewayFlag;
	
	long m_twinVersion;
    
    long m_systemRebootTime;
    long m_systemRebootTimeSec;
	
	std::mutex m_mtx;

    IOTHUB_DEVICE_CLIENT_HANDLE m_deviceHandle;
    IOTHUB_MESSAGE_HANDLE m_messageHandle;
    IOTHUB_DEVICE_CLIENT_LL_HANDLE m_devicellHandle;
	
	std::function<void(nlohmann::json)> m_cloudRequestCB; //Callback member
	std::function<void(nlohmann::json)> m_ConnectionStatusCB; //Callback member
	nlohmann::json m_configuration;
	
	ExceptionLogger *m_exceptionLoggerObj;
};

#endif
