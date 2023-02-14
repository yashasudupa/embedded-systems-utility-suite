#ifndef MQTTRequestResponseManager_h
#define MQTTRequestResponseManager_h 1
#include <iostream>
#include <map>
#include <set>
#include <nlohmann/json.hpp>
#include "GlobalOperations.h"
#include "Common.h"
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <sstream>


typedef struct AppDetailStruct
{
	std::string appName;
	std::set<std::string> deviceIdSet;
}APP_DETAILS; 

typedef struct BROADSENS_MQTT_MESSAGE
{
	char *mqtt_data;
	std::string mqtt_topic;
}BROADSENS_MQTT_MESSAGE;

class MQTTRequestResponseManager
{
public:
	MQTTRequestResponseManager();
	~MQTTRequestResponseManager();
	ResponseStruct ExecuteCommand(char *payload, std::uint32_t payloadLen, char *topic);
	void SetGatewayId( std::string gatewayId );
	void RegisterCB( std::function<void(nlohmann::json,std::string)> cb );
	static MQTTRequestResponseManager *GetInstance();
	std::string buffer;              // A good-sized buffer
	std::condition_variable cv;
	std::mutex mtx;
	
private:
	ResponseStruct ValidateData( nlohmann::json &dataJson, int caseId );
	ResponseStruct ResponseHandler( nlohmann::json responseJson );
	ResponseStruct RequestHandler( nlohmann::json requestJson );
	bool MaintainPersistency();
	nlohmann::json BroadsensMessagesProcess( BROADSENS_MQTT_MESSAGE  &brsns_msg, std::uint32_t &payloadLen);
	std::string SensorType( int sns_type);

private:
	static MQTTRequestResponseManager *m_MQTTRqstRspInstance;
	std::map<long,APP_DETAILS*> m_appDetailMap;
	std::function<void(nlohmann::json,std::string)> m_dataCB;
	std::string m_gatewayId;
	nlohmann::json m_persistentJson;
	nlohmann::json m_deviceIdInfoJson;
	bool m_cloudConnectionStatus;
	ExceptionLogger *m_exceptionLoggerObj;
	
	std::vector<BROADSENS_MQTT_MESSAGE> broadsens_msg_queue;
	
	typedef struct fft_message
	{
		int16_t fftsensor;
		int16_t fftmeasure;
		int16_t filtertype;
		int16_t filterorder;
		int16_t cutoff;
		int16_t cutofftwo;
	}FFT_MESSAGE;


	FFT_MESSAGE fft_msg;
	
	std::map<std::string ,nlohmann::json> sensor_idlist;	
	bool daqstatus = true;
};

#endif