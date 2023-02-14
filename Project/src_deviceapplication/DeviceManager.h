#ifndef DeviceManager_h
#define DeviceManager_h 1

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <dlfcn.h>
#include <set>
#include <sys/stat.h>
#include "DeviceLibraryWrapper.h"
#include "GlobalOperations.h"
#include "Common.h"

typedef void ( * destroydeviceobject ) ( DeviceLibraryWrapper* );
typedef DeviceLibraryWrapper* ( * createdeviceobject ) ( std::string );

class DeviceManager 
{
public:
	DeviceManager( std::string libName, std::string processName );
	~DeviceManager();
	void DeviceDataCB( std::function<void(nlohmann::json)> cb );
	void RegisterAppCB( std::function<void(nlohmann::json)> cb );
	void SetConfig( nlohmann::json config );
	void Discoverdevice( std::string deviceId, bool statusFlag );
	  
private:
	void DeviceDataReceive(nlohmann::json jsonObject);
	bool Connectdevice ( std::string deviceId, nlohmann::json connectionDetails );
	void CreateDeviceObject();
	void LoadDeviceProperties( std::string deviceId, std::string processName );
	nlohmann::json GetDeviceState( nlohmann::json devicePropertyJson );
	bool SetDeviceState( nlohmann::json devicePropertyJson );
	bool SendC2DResponse( nlohmann::json jsonObj, std::string message,std::string deviceId );
	void RegisterDeviceId();
    void UpdateConfigInTwin( std::string deviceID, std::string subJobId );
private: 

	DeviceLibraryWrapper *m_deviceLibWrapperObj;
	void *m_moduleHandle;
	std::function<void(nlohmann::json)> m_dataCB;
	std::function<void(nlohmann::json)> m_registerDeviceCB;
	
	std::string m_processName;
	std::string m_configFilePath;
	std::set<std::string> m_activeDeviceSet;
	nlohmann::json m_registerAppJson;
	
	ExceptionLogger *m_exceptionLoggerObj;
};

#endif
