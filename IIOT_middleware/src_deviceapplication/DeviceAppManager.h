#ifndef DeviceAppManager_h
#define DeviceAppManager_h 1

#include "GlobalOperations.h"
#include "Common.h"
#include "DeviceManager.h"
#include "LocalBrokerCommunicationManager.h"

class DeviceAppManager
{
public:
	DeviceAppManager( std::string processName, std::string gatewayId, std::string libName );
	~DeviceAppManager();
	void DeviceDataReceive( nlohmann::json deviceDataJson );
	void RegisterDeviceId( nlohmann::json registerDeviceJson );
	void ReceiveSubscribedData( std::string data );

private:
	void SendNotificationToCloud();
	void RegisterApp( std::string deviceId );
	void InitDeviceAppManager();

private:
	DeviceManager *m_deviceManagerObj;
	LocalBrokerCommunicationManager *m_localBrokerObj;
	
	std::string m_gatewayId;
	std::string m_processName;
	std::string m_publishTopic;
	std::string m_libName;
	
	long m_appId;
	ExceptionLogger *m_exceptionLoggerObj;
};
#endif