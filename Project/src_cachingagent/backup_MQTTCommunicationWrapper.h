#ifndef MQTTCommunicationWrapper_h
#define MQTTCommunicationWrapper_h 1

#include <mosquitto.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <nlohmann/json.hpp>
#include "Common.h"

class MQTTCommunicationWrapper
{
public:
	MQTTCommunicationWrapper(nlohmann::json mqttConfig);
	~MQTTCommunicationWrapper();
	
	bool ConnectMqttBroker();
	bool Publish( std::string payload, std::string topic );
	bool DiconnectMqttBroker();

	void Subscribe ( std::string topic );
	void Datacb ();
	void SendDataToLocalBrokerCB(std::function<void(std::string)> cb);
	void MQTTConnectionStatusCB( std::function<void()> cb);
	
	
private:
	static void OnConnect(struct mosquitto *mosq, void *obj, int reason_code);
	static void OnMessage(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);
	static void OnDisconnect(struct mosquitto *mosq, void *obj, int rc);
	
	void LoopStart();
	void setData(const struct mosquitto_message *msg);
	
private:
	nlohmann::json m_mqttConfigJson; 
	struct mosquitto *m_mosq;
	std::function<void(std::string)> m_dataCB;
	std::function<void()> m_connectionStatusCB;
	bool m_connectionStatus;
	
};

#endif