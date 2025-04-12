#ifndef MQTTRequestResponseManager_h
#define MQTTRequestResponseManager_h 1
#include <iostream>
#include <map>
#include <set>
#include <nlohmann/json.hpp>
#include "GlobalOperations.h"
#include "Common.h"

typedef struct AppDetailStruct
{
	std::string appName;
	std::set<std::string> deviceIdSet;
}APP_DETAILS; 


class MQTTRequestResponseManager
{
public:
	MQTTRequestResponseManager();
	~MQTTRequestResponseManager();
	ResponseStruct ExecuteCommand( nlohmann::json dataJson );
	void SetGatewayId( std::string gatewayId );
	void RegisterCB( std::function<void(nlohmann::json,std::string)> cb );
	
private:
	ResponseStruct ValidateData( nlohmann::json &dataJson, int caseId );
	ResponseStruct ResponseHandler( nlohmann::json responseJson );
	ResponseStruct RequestHandler( nlohmann::json requestJson );
	bool MaintainPersistency();

private:
	std::map<long,APP_DETAILS*> m_appDetailMap;
	std::function<void(nlohmann::json,std::string)> m_dataCB;
	std::string m_gatewayId;
	nlohmann::json m_persistentJson;
	nlohmann::json m_deviceIdInfoJson;
	bool m_cloudConnectionStatus;
	ExceptionLogger *m_exceptionLoggerObj;
};

#endif