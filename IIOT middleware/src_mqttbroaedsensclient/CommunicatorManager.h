#ifndef CommunicatorManager_h
#define CommunicatorManager_h 1

#include "Common.h"
#include "LocalBrokerCommunicationManager.h"
#include "MQTTRequestResponseManager.h"

class CommunicatorManager
{
public:
	CommunicatorManager( std::string gatewayId );
	~CommunicatorManager();
	void ReceiveSubscribedData( char *payload, std::uint32_t payloadLen, char *topic);
	void DataPublisher( nlohmann::json dataJson, std::string publishTopic );
	
private:
	void SendNotificationToCloud();
	void InitCommunicatorManager();
	
private:
	LocalBrokerCommunicationManager *m_localBrokerObj;
	MQTTRequestResponseManager *m_mqttRequestResponseObj;
	std::string m_gatewayId;
	ExceptionLogger *m_exceptionLoggerObj;
};

#endif