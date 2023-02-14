#ifndef MODBUSTCPMASTER
#define MODBUSTCPMASTER 1

#include <nlohmann/json.hpp>
#include <thread>

#include "modbus.h"
#include "Common.h"
#include "GlobalOperations.h"
#include "DeviceLibraryWrapper.h"
#include "AssetManager.h"


class ModbusTCPMaster : public DeviceLibraryWrapper
{
	
public:

	ModbusTCPMaster( std::string processName );
	~ModbusTCPMaster();
	
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
    
    bool m_changeValueState;
	
};

#endif