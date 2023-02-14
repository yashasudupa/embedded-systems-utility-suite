#ifndef READ_PROPERTIES
#define READ_PROPERTIES 1
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <string.h>
#include <map>
#include <list>
#include <thread>
#include <unistd.h>
#include <bits/stdc++.h>
#include <boost/format.hpp>
#include "muParser.h"
#include "ManageGroupData.h"
#include <mutex>
#include "modbus.h"
#include "GlobalOperations.h"
#include "Common.h"
#include "ExceptionLogger.h"
#define	MAXADDR			99999


class ReadProperties
{
public:

	ReadProperties( std::string deviceId, std::string slaveId, std::string processName, nlohmann::json connectionJson );
	~ReadProperties();
	void RegisterPropertiesCB( std::function<void(nlohmann::json)> cb );
	void CheckConnectionStatusCB( std::function<void(std::string)> connectionCB );
	void StartDeviceStateGetterThread();
	void SetConfig( nlohmann::json config );
	void SetThreadStatus( bool threadStartStaus );
	void SetDeviceProperties( nlohmann::json deviceProperties );
	void CreateCommandStruct();
	void CreatePropertiesStruct();
    
    void SetDeviceConfiguration( nlohmann::json config );
    bool ConnectToDevice();
    void CloseConnection();
    void UpdateConfigFile();
    void SetPollingFreq( long pollingFreq );
    void PropertiesReceiver( nlohmann::json jsonObj );
	
private:
    
	void AddDesiredPropertiesInTelemetry();
	short GetStates();
	void AddPropertiesAndAlertInMap( nlohmann::json alertPropertiesJson, bool alertFlag  );
	
	void CreateTelemetryAndAlertJson( bool isAlertFlag, VALUE_TYPE *oldValueType,
					VALUE_TYPE *newValueType, char datatype, std::string propertyName );
    
    
    void CreateTelemetryJson( VALUE_TYPE *newValueType, char datatype, 
                              std::string propertyName, std::string grpName="g1" );
	
	short ParseValues( int functionCode, long strtAddr, short wordQuant, short blockNumber,
					uint16_t wordValues[], uint8_t coilValues[] );
					
	short ReadRegisters( int functionCode, long startAddress, short wordQuant,
						int dataType, uint16_t wordValues[], uint8_t coilValues[] );
                        
    void SendNotificationToCloud( int caseId, nlohmann::json config = nullptr );
	
private:
	nlohmann::json m_commandJson;
	nlohmann::json m_deviceProperties;
	nlohmann::json m_telemetryJson;
	nlohmann::json m_alertJsonArray;
	nlohmann::json m_ruleEnginePropertyJson;
	nlohmann::json m_derivedPropertiesJson;
	nlohmann::json m_telemetryModeJson;
	nlohmann::json m_lastAlertPersistancyJson;
    nlohmann::json m_connectionJson;
    nlohmann::json m_cmmonJson;
    nlohmann::json m_localTelemetryJson;
	
	std::string m_deviceId;
	std::string m_deviceMode;
    std::string m_telemetryMode;
    std::string m_telemetryModeSubJobId;
	std::string m_alertPercistancyFileName;
    std::string m_ruleEnginePropertiesFileName;
	std::string m_processName;
	std::string m_configFilePath;
	std::string m_slave_Id;
    
    bool m_firstTimeAppStartFlag;
    long m_lastAlertVal;
	
    modbus_t* m_ctx;
	
	std::map<std::string, PROPERTIES_STRUCT*> m_propertiesMap; 
    std::map<std::string, ManageGroupData*> m_commandGroupStructMap;
	std::list<CMD_STRUCT*> m_commandStructList;
	std::set<std::string> m_ruleEnginePropertyMap;

	
	short m_noOfRegInOneBlock;
	short m_blockNumber;
	short m_totalNumberOfProperties;
    short m_numberOfRequests;
    
	std::mutex m_mtx;
	
	bool m_threadStartStaus;
	bool m_changeValueState;
	bool m_ThreadRunning;
    bool m_connectionStatus;
    bool m_connectionNotificationFlag;
    
	long m_pollingFreq;
	long m_turboTimeout;
    
    GROUP_STRUCT *m_grpStructObj1;
    GROUP_STRUCT *m_grpStructObj2;
    GROUP_STRUCT *m_grpStructObj3;
    
	ExceptionLogger *m_exceptionLoggerObj;
	std::thread m_threadObj;
	std::function<void(nlohmann::json)> m_propertiesCB;
};

#endif