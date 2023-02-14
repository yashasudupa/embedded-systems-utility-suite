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
	void RegisterCB(std::function<void(char *, std::uint32_t payloadLen, char *topic)> cb);
	void ConnectionEstablish();
	bool PublishData(std::string payload, std::string topic);
	bool SubscribeTopic(std::string topic);
	
private:
	void DeviceDataReceiver( char * data, std::uint32_t payloadLen, char *topic );
private:
	nlohmann::json m_mqttConfiguration;
	MQTTCommunicationWrapper *m_mqttCommunicationObj;
	std::function<void(char *, std::uint32_t payloadLen, char *)> m_localdataCB;
	ExceptionLogger *m_exceptionLoggerObj;
	
};

#endif