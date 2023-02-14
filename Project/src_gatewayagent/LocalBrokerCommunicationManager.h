#ifndef LocalBrokerCommunicationManager_h
#define LocalBrokerCommunicationManager_h 1

#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include "MQTTCommunicationWrapper.h"
#include "ExceptionLogger.h"
#include "GlobalOperations.h"

class LocalBrokerCommunicationManager
{
public:
	LocalBrokerCommunicationManager();
	~LocalBrokerCommunicationManager();
	void RegisterCB( std::function<void(std::string)> cb );
	void ConnectionEstablish();
	bool PublishData(std::string payload, std::string topic);
	bool SubscribeTopic(std::string topic);
	
private:
	void DeviceDataReceiver(std::string data);
	void DeviceDataReceiverForBackup(std::string data);
	
private:
	nlohmann::json m_mqttConfiguration;
	MQTTCommunicationWrapper *m_mqttCommunicationObj;
	std::function<void(std::string)> m_localdataCB;
	std::function<void(std::string)> m_localdataCB_Backup;
	ExceptionLogger *m_exceptionLoggerObj;
	
};

#endif