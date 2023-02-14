#ifndef MQTT_BROADSENS_CLIENT
#define MQTT_BROADSENS_CLIENT 1

#include <nlohmann/json.hpp>

//#include "modbus.h"
#include "Common.h"
#include "GlobalOperations.h"
#include "DeviceLibraryWrapper.h"
#include "CommunicatorManager.h"
#include "AssetManager.h"
#include "MQTTRequestResponseManager.h"

class MQTTBroadsensClient : public DeviceLibraryWrapper
{
	
public:

	MQTTBroadsensClient( std::string processName );
	~MQTTBroadsensClient();
	
	bool Connectdevice ( std::string deviceId, nlohmann::json jsonObj );
	bool SetDeviceState( nlohmann::json devicePropertyJson );
	nlohmann::json Discoverdevice();
	nlohmann::json LoadDeviceProperties( std::string deviceId, std::string processName );
	nlohmann::json GetDeviceState( nlohmann::json devicePropertyJson );
	
	void SetConfig( nlohmann::json config );
	void PropertiesReceiver( nlohmann::json jsonObject );
	
private:

	short WriteSetPoint( nlohmann::json devicePropertyJson );
	bool SetPollingInfo( nlohmann::json jsonObj );
	bool SendC2DResponse( nlohmann::json jsonObj, std::string message, std::string deviceId );
    
private:

	std::map<std::string, AssetManager*> m_connectionMap; 

	nlohmann::json m_discoverdeviceConfigJson;
	nlohmann::json m_deviceConfigJson;
	nlohmann::json m_commandJson;
	std::thread m_connectionThreadObj;
	
	std::string m_processName;
	std::string m_configFilePath;
    
	void SendMQTTControlCommands(void);
	//void controlcommandsprocess (nlohmann::json payload, MQTTRequestResponseManager& m_mqttRequestResponseObj, std::string control_topic);
	void controlcommandsprocess (nlohmann::json payload, MQTTRequestResponseManager* m_mqttRequestResponseObj, std::string control_topic);
	void RegisterCB( std::function<void(nlohmann::json, std::string)> cb );
	void DataPublisher( nlohmann::json dataJson, std::string publishTopic );
	
	std::function<void(nlohmann::json,std::string)> m_dataCB_bs;
	
	bool m_ThreadRunning;
	LocalBrokerCommunicationManager *m_localBrokerObj;
	MQTTRequestResponseManager* m_mqttRequestResponseObj;
	GlobalOperations *GlobalOperationsObj;
	std::thread m_threadObj;
	
    bool m_changeValueState;
	
};

#endif