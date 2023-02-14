#include "ModbusTCPMaster.h"

#define MAKEWORD( l, h )	((short)(((char)(l)) | ((short)((char)(h))) << 8))

extern "C" 
{
	void DestroyObject( ModbusTCPMaster* object )
	{
		delete object;
	}
	
	DeviceLibraryWrapper * CreateObject( std::string processName )
	{
		return new ModbusTCPMaster( processName ) ;
	}
}

ModbusTCPMaster::ModbusTCPMaster( std::string processName ):
	m_processName( processName ) 
{
	m_configFilePath = "./config/" + m_processName + "/";
    
    m_commandJson[APP_NAME] = processName;
	m_commandJson[APP_ID] = GetProcessIdByName(processName);
	m_commandJson[COMMAND_TYPE] = RESPONSE;
}


ModbusTCPMaster::~ModbusTCPMaster()
{
    auto iter = m_connectionMap.begin();
    while (iter != m_connectionMap.end()) 
    {
        AssetManager *deviceInfoObj = iter->second; 
        if( deviceInfoObj )
        {
            delete deviceInfoObj;
        }
        ++iter;
    }
    m_connectionMap.clear();
	m_connectionThreadObj.join();
}

nlohmann::json ModbusTCPMaster::Discoverdevice()
{
	std::string configFilePath = m_configFilePath + REGISTER_DEVICES_JSON;
	m_discoverdeviceConfigJson = ReadAndSetConfiguration( configFilePath );
	return m_discoverdeviceConfigJson;
}

bool ModbusTCPMaster::Connectdevice ( std::string deviceId, nlohmann::json jsonObj )
{
    try
    {
        auto it = m_connectionMap.find(deviceId);
        if (it == m_connectionMap.end())
        {
            AssetManager *assetManagerObj = new AssetManager( deviceId, m_processName );
            if( assetManagerObj )
            {
                m_connectionMap[deviceId] = assetManagerObj;
                assetManagerObj->RegisterPropertiesCB(std::bind( &ModbusTCPMaster::PropertiesReceiver, this, std::placeholders::_1));
                assetManagerObj->LoadProperties();
            }
        }
    }
    catch( nlohmann::json::exception &e )
	{
		std::cout << e.id << " : " << e.what() << std::endl;
	}
	return true;
}

nlohmann::json ModbusTCPMaster::LoadDeviceProperties( std::string deviceId, std::string processName )
{
    try
	{
		std::string fileName =  m_configFilePath + deviceId + ".json";
		nlohmann::json fileContentJson = ReadAndSetConfiguration( fileName );
        
		if( !fileContentJson.is_null() )
		{
            auto it = m_connectionMap.find(deviceId);
            AssetManager *assetManagerObj;
            if (it == m_connectionMap.end())
            {
                assetManagerObj = new AssetManager( deviceId, processName );
                if( assetManagerObj )
                {
                    m_connectionMap[deviceId] = assetManagerObj;
                    assetManagerObj->RegisterPropertiesCB(std::bind( &ModbusTCPMaster::PropertiesReceiver, this, std::placeholders::_1));
                    assetManagerObj->LoadProperties();
                }
            }
            else
            {
                assetManagerObj = it->second;
                assetManagerObj->RegisterPropertiesCB(std::bind( &ModbusTCPMaster::PropertiesReceiver, this, std::placeholders::_1));
            }
            
            
            std::cout << "fileContentJson : " << fileContentJson << "\n\n";
            if( assetManagerObj )
            {
                assetManagerObj->SetPropertyJson( fileContentJson );
            }
            
		}
	}
	catch( nlohmann::json::exception &e )
	{
		std::cout << e.id << " : " << e.what() << std::endl;
	}

	return NULL;
    
}

// input = json
nlohmann::json ModbusTCPMaster::GetDeviceState( nlohmann::json devicePropertyJson )
{
	nlohmann::json obj;	
	return obj;
}

// input = json
bool ModbusTCPMaster::SetDeviceState( nlohmann::json devicePropertyJson )
{
	int returnStatus = -1;
	try
	{
		for (auto& x : devicePropertyJson["setproperties"].items())
		{
			nlohmann::json jsonObj = x.value();
			returnStatus = WriteSetPoint( jsonObj );
		}
	}
	catch( nlohmann::json::exception &e )
	{
		std::cout << e.id << " : " << e.what() << std::endl;
	}
	
	
	if( returnStatus > 0)
	{
		return true;
	}
	
	return false;
}


short ModbusTCPMaster::WriteSetPoint( nlohmann::json devicePropertyJson )
{
	return -1;
}


void ModbusTCPMaster::SetConfig( nlohmann::json config )
{
	std::string message ="";
	try
	{
		std::string configJson = config[COMMAND];
        if( configJson == SET_CONFIGURATION )
		{
			m_deviceConfigJson = config;
			for( auto& x : config["assets"].items() )
			{
				nlohmann::json keyJson;
				keyJson[DEVICE_ID] = x.key();
                nlohmann::json valueJson;
                valueJson = x.value();
				std::string deviceId = keyJson[DEVICE_ID];
                
                auto itr = m_connectionMap.find(deviceId);
                
                if( itr != m_connectionMap.end() )
                {
                    AssetManager *assetObj = itr->second;
                    if(assetObj)
                    {
                        assetObj->SetConfig( config );
                    }
                }

			}
		}
        else if ( configJson == REGISTER_SLAVES )
        {
            std::string deviceId = config[DEVICE_ID];
            auto it = m_connectionMap.find(deviceId);
            AssetManager *assetManagerObj;
            if (it != m_connectionMap.end())
            {
                assetManagerObj = it->second;
                if( assetManagerObj )
                {
                    assetManagerObj->DeRegisterSlaves( config );
                    assetManagerObj->RegisterDevice( config );
                }
            }
        }
		else if( configJson == DEREGISTER_DEVICES )
		{
			std::string deviceId = config[DEVICE_ID];
			message = "Deregister asset successfully";
			auto it = m_connectionMap.find( deviceId );
			if( it != m_connectionMap.end() )
			{
				AssetManager *assetObj = it->second;
				if( assetObj )
				{
                    assetObj->SetConfig( config );
					delete assetObj;
					m_connectionMap.erase(it);
				}
			}
		}
		else if( configJson == "register_rule_device_properties" || configJson == CHANGE_DEVICE_MODE )
		{
			std::string deviceId = config[DEVICE_ID];
            
            std::cout << "ModbusTCPMaster::SetConfig() : deviceId - " << deviceId << std::endl;
            
			auto it = m_connectionMap.find( deviceId );

			if( it != m_connectionMap.end() )
			{
				AssetManager *assetObj = it->second;
				if( assetObj )
				{
					assetObj->SetConfig( config );
				}
			}
		}
	}
	catch( nlohmann::json::exception &e )
	{
		std::cout << " SetConfig EXCEPTION : "<< e.id << " : " << e.what() << std::endl;
	}
}


void ModbusTCPMaster::PropertiesReceiver( nlohmann::json jsonObject )
{
	m_dataCB( jsonObject );
}


bool ModbusTCPMaster::SendC2DResponse( nlohmann::json jsonObj, std::string message, std::string deviceId )
{
	nlohmann::json c2dJson;
	c2dJson[COMMAND_INFO][COMMAND_TYPE] = "response";
	c2dJson[COMMAND_INFO][APP_NAME] = m_processName;
	c2dJson[COMMAND_INFO][APP_ID] = GetProcessIdByName( m_processName );
	c2dJson[SUB_JOB_ID] = jsonObj[SUB_JOB_ID];
	c2dJson[STATUS] = "success"; 
	c2dJson[TIMESTAMP] = GetTimeStamp();
	c2dJson[DEVICE_ID] = deviceId.c_str();
	c2dJson[MESSAGE] = message.c_str();
	c2dJson[TYPE] = "notification";
	
	if ( m_dataCB )
	{
		m_dataCB( c2dJson );
		return true;
	}
	return false;
}

