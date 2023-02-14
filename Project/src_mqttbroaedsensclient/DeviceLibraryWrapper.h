#ifndef DeviceLibraryWrapper_h
#define DeviceLibraryWrapper_h 1

#include <iostream>
#include <string>
#include <functional>
#include <nlohmann/json.hpp>

class DeviceLibraryWrapper 
{
public:

	~DeviceLibraryWrapper(){};
	virtual nlohmann::json Discoverdevice() = 0;
	virtual nlohmann::json LoadDeviceProperties( std::string deviceId, std::string processName ) = 0;
	virtual nlohmann::json GetDeviceState( nlohmann::json devicePropertyJson ) = 0;
	
	virtual bool Connectdevice ( std::string deviceId, nlohmann::json connectionDetails ) = 0;
	virtual bool SetDeviceState( nlohmann::json devicePropertyJson ) = 0;
	
    virtual void SetConfig( nlohmann::json config ) = 0;
	virtual void DataCB( std::function<void(nlohmann::json)> cb )
	{
		std::cout << "DeviceLibraryWrapper : " << std::endl;
		m_dataCB = cb;
	}

protected:
	std::function<void(nlohmann::json)> m_dataCB;
};

using Base_creator_t = DeviceLibraryWrapper *(*)();
using Base_destroy_t = void (*)(DeviceLibraryWrapper *);

#endif
