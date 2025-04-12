#include "DeviceManager.h"
#include <string>
#include <string.h>

//libModbusTCPMaster.so
//ModbusTCPMaster

DeviceManager::DeviceManager( std::string libName, std::string processName ):
	m_processName( processName )
{
	std::stringstream logg;
	try
	{
		m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
		m_configFilePath = "./config/" + m_processName + "/";
		mkdir ( m_configFilePath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO );
		chmod( m_configFilePath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO );
		
		#ifdef SIEMENS_TCP
			if (libName == "SiemensTCPIP")
			{
				libName = "snap7";
			}
		#endif
		
		char ErrorString [256];
		std::string strPath = "/usr/lib/lib" + libName + ".so";
		m_moduleHandle = dlopen(strPath.c_str(), RTLD_LAZY);
		
		if ( m_moduleHandle == NULL )
		{
			strcpy(ErrorString ,dlerror());
			logg << "DeviceManager::DeviceManager  Message : DLL not found. .SO File Name :  " << strPath << ", Error Info : " << ErrorString;
			m_exceptionLoggerObj->LogError(logg.str());
			exit(0);
		}
		else
		{
			logg << "DeviceManager::DeviceManager  Message : Load .so file successfully .SO File Name :  " << strPath;
			m_exceptionLoggerObj->LogInfo( logg.str() );
			CreateDeviceObject();
		}
		
		if( m_deviceLibWrapperObj )
		{
			m_deviceLibWrapperObj->DataCB(std::bind( &DeviceManager::DeviceDataReceive, this, std::placeholders::_1));
		}
	}
	catch( ... )
	{
		logg.str( std::string() );
		logg << "DeviceManager::DeviceManager  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

DeviceManager::~DeviceManager()
{
	if ( m_deviceLibWrapperObj )
	{
		delete m_deviceLibWrapperObj;
	}
	
	if ( m_moduleHandle )
	{
		dlclose( m_moduleHandle );
	}
}

void DeviceManager::DeviceDataReceive( nlohmann::json jsonObject )
{
	m_dataCB( jsonObject );
}

void DeviceManager::RegisterDeviceId()
{
	
}

void DeviceManager::CreateDeviceObject()
{
	createdeviceobject FunctionAddress = ( createdeviceobject ) dlsym ( m_moduleHandle, "CreateObject" );
	if ( FunctionAddress == NULL )
	{
		return;
	}
	m_deviceLibWrapperObj = FunctionAddress ( m_processName );
}


bool DeviceManager::Connectdevice ( std::string deviceId, nlohmann::json connectionDetails )
{
	return m_deviceLibWrapperObj->Connectdevice( deviceId, connectionDetails );
}

//return structure
//validate json
//
void DeviceManager::Discoverdevice ( std::string receivedDeviceId, bool statusFlag )
{
	std::stringstream logg;
	try
	{
		if( m_deviceLibWrapperObj )
		{
			nlohmann::json jsonObj = m_deviceLibWrapperObj->Discoverdevice();
			if( !jsonObj.is_null() ) 
			{
				for ( auto& x : jsonObj["assets"].items() )
				{
					std::string deviceId = x.key();
					nlohmann::json registerAppJson;
					registerAppJson[COMMAND_INFO][DEVICE_ID] = deviceId;
					registerAppJson[COMMAND_INFO][COMMAND_TYPE] = "device_register";
					m_registerDeviceCB( registerAppJson );
					m_activeDeviceSet.insert( deviceId );
					nlohmann::json connectionDetails = x.value();
					
					if( statusFlag )
					{
                        std::cout << "ReceivedDeviceID : " << receivedDeviceId << "\n\n";
                        std::cout << "deviceId : " << deviceId << "\n\n";
                        
						if( deviceId == receivedDeviceId  )
						{
							LoadDeviceProperties( deviceId, m_processName );
							break;
						}
						else
						{
							continue;
						}
					}

					Connectdevice( deviceId, connectionDetails );
					LoadDeviceProperties( deviceId, m_processName );
				}
			}
		}
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "DeviceManager::Discoverdevice  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "DeviceManager::Discoverdevice  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}	
}


void DeviceManager::DeviceDataCB( std::function<void(nlohmann::json)> cb )
{
	m_dataCB = cb;
}

void DeviceManager::RegisterAppCB( std::function<void(nlohmann::json)> cb )
{
	m_registerDeviceCB = cb;
}

//return structure
//
void DeviceManager::LoadDeviceProperties( std::string deviceId, std::string appId )
{
	m_deviceLibWrapperObj->LoadDeviceProperties( deviceId, appId );
}

//return structure
//
nlohmann::json DeviceManager::GetDeviceState( nlohmann::json devicePropertyJson )
{
	return m_deviceLibWrapperObj->GetDeviceState( devicePropertyJson );
}

//return structure
//
bool DeviceManager::SetDeviceState( nlohmann::json devicePropertyJson )
{
	return m_deviceLibWrapperObj->SetDeviceState( devicePropertyJson );
}

//validate json
void DeviceManager::SetConfig( nlohmann::json config )
{
	std::stringstream logg;
	try
	{
		std::string configJson = config[COMMAND];
		std::string message ="";
		std::string configFilePath = "";
		
        if( configJson == SET_CONFIGURATION )
        {
            configFilePath = m_configFilePath + DEVICE_CONFIGURATION_JSON;
			nlohmann::json changeConfigJson;
            changeConfigJson = ReadAndSetConfiguration( configFilePath );
            
            for (auto& el : changeConfigJson["assets"].items())
            {
                std::string keyDeviceId = el.key();
                
                changeConfigJson["assets"][keyDeviceId]["g1_ingestion_frequency_in_ms"]     = config["g1_ingestion_frequency_in_ms"];
                changeConfigJson["assets"][keyDeviceId]["g1_measurement_frequency_in_ms"]   = config["g1_measurement_frequency_in_ms"];
                changeConfigJson["assets"][keyDeviceId]["g1_turbo_mode_frequency_in_ms"]    = config["g1_turbo_mode_frequency_in_ms"];
                changeConfigJson["assets"][keyDeviceId]["g2_ingestion_frequency_in_ms"]     = config["g2_ingestion_frequency_in_ms"];
                changeConfigJson["assets"][keyDeviceId]["g2_measurement_frequency_in_ms"]   = config["g2_measurement_frequency_in_ms"];
                changeConfigJson["assets"][keyDeviceId]["g2_turbo_mode_frequency_in_ms"]    = config["g2_turbo_mode_frequency_in_ms"];
                changeConfigJson["assets"][keyDeviceId]["g3_ingestion_frequency_in_ms"]     = config["g3_ingestion_frequency_in_ms"];
                changeConfigJson["assets"][keyDeviceId]["g3_measurement_frequency_in_ms"]   = config["g3_measurement_frequency_in_ms"];
                changeConfigJson["assets"][keyDeviceId]["g3_turbo_mode_frequency_in_ms"]    = config["g3_turbo_mode_frequency_in_ms"];
                changeConfigJson["assets"][keyDeviceId]["ingestion_settings_type"]          = config["ingestion_settings_type"];
                changeConfigJson["assets"][keyDeviceId]["turbo_mode_timeout_in_ms"]         = config["turbo_mode_timeout_in_ms"];
                
                changeConfigJson["assets"][keyDeviceId].update( config["assets"][keyDeviceId] );
                
                logg.str("");
                logg << "DeviceManager::SetConfig  H1 $$$$$$ config: " << config << "\nkeyDeviceId : " << keyDeviceId;
                m_exceptionLoggerObj->LogInfo( logg.str() );
                
                
                logg.str("");
                logg << "DeviceManager::SetConfig  H1 $$$$$$ changeConfigJson: " << changeConfigJson << "\nkeyDeviceId : " << keyDeviceId;
                m_exceptionLoggerObj->LogInfo( logg.str() );
                
                WriteConfiguration( configFilePath, changeConfigJson );
            }
			m_deviceLibWrapperObj->SetConfig( config );
        }
		else if( configJson == REGISTER_DEVICES )
		{
			configFilePath = m_configFilePath + REGISTER_DEVICES_JSON;
			nlohmann::json readRegisterDevicesJson;
			readRegisterDevicesJson = ReadAndSetConfiguration( configFilePath );
			
			readRegisterDevicesJson["assets"].update( config["assets"] );
			WriteConfiguration( configFilePath, readRegisterDevicesJson );
			nlohmann::json registerAppJson;
            std::string subJobId = config[SUB_JOB_ID];

			for( auto& x : config["assets"].items() )
			{
                nlohmann::json registerJson;
                registerJson[PROCESS_NAME] = m_processName;
                registerJson[COMMAND] = REGISTER_DEVICES;
                registerJson[DEVICE_ID] = x.key();
                
                Connectdevice( x.key(),x.value() );
                m_deviceLibWrapperObj->SetConfig( registerJson );
                
				registerAppJson[COMMAND_INFO][DEVICE_ID] = x.key();
				registerAppJson[COMMAND_INFO][COMMAND_TYPE] = "device_register";
                UpdateConfigInTwin( x.key(), subJobId );
				message = "Register asset successfully";
				SendC2DResponse( config, message, x.key() );
                m_registerDeviceCB( registerAppJson );
			}
		}
		else if( configJson == CHANGE_DEVICE_MODE || configJson == "register_rule_device_properties" || configJson == REGISTER_SLAVES || 
                    configJson == SET_SLAVE_CONFIGURATION || configJson == RESET_SENSOR_BOARD || configJson == RESET_MESH_NETWORK || 
                    configJson == RESET_SENSOR || configJson == RESET_BLUENRG || configJson == SET_BOARD_CONFIGURATION || 
                    configJson == SET_POSITION_CONFIGURATION || configJson == SET_MOP_CONFIGURATION || configJson == READ_SENSOR_PROPERTY ||
                    configJson == SET_FOTA_MODE || configJson == GET_DEVICE_CONFIGURATION || configJson == READ_SENSOR_POSITION_CONFIGURATION)
		{
			m_deviceLibWrapperObj->SetConfig( config );
		}
		else if( configJson == DEREGISTER_DEVICES )
		{
			for( auto& x : config["assets"].items() )
			{
				nlohmann::json keyJson;

				std::string deviceId = x.value();
				std::string propertiesFilepathcommand = "rm -r " + m_configFilePath + deviceId + ".json";
				system( propertiesFilepathcommand.c_str() );
                
                std::string slavesFilepathcommand = "rm -r " + m_configFilePath + deviceId + "_slaves.json";
				system( slavesFilepathcommand.c_str() );
				
				std::string registerDevicePath = m_configFilePath + DEVICE_CONFIGURATION_JSON;
				nlohmann::json deletePollingObj = ReadAndSetConfiguration( registerDevicePath );
				
				if( !deletePollingObj.is_null() )
				{
					deletePollingObj["assets"].erase( deviceId );
					WriteConfiguration( registerDevicePath, deletePollingObj );
				}
				
				registerDevicePath = m_configFilePath + REGISTER_DEVICES_JSON;
				nlohmann::json deleteRegisterObj = ReadAndSetConfiguration( registerDevicePath );
				if( !deleteRegisterObj.is_null() )
				{
					deleteRegisterObj["assets"].erase( deviceId );
					WriteConfiguration( registerDevicePath, deleteRegisterObj );
				}
			
				nlohmann::json registerAppJson;
				registerAppJson[COMMAND_INFO][DEVICE_ID] = deviceId;
				registerAppJson[COMMAND_INFO][COMMAND_TYPE] = "device_deregister";
				registerAppJson[SUB_JOB_ID] = config[SUB_JOB_ID];
				
				nlohmann::json deleteRegisterObj1;
				deleteRegisterObj1[SUB_JOB_ID] = config[SUB_JOB_ID];
				deleteRegisterObj1[DEVICE_ID] = deviceId;
				deleteRegisterObj1[COMMAND] = DEREGISTER_DEVICES;
				m_deviceLibWrapperObj->SetConfig( deleteRegisterObj1 );
                m_registerDeviceCB( registerAppJson );
				
			}
		}
		else if( configJson == SET_PROPERTIES )
		{
			std::string deviceId = config[DEVICE_ID];
			std::string filePath = m_configFilePath + deviceId + ".json";
			nlohmann::json readPropertiesJson = ReadAndSetConfiguration( filePath );
			
			if( !config[MEASURED_PROPERTIES].is_null() )
			{
				readPropertiesJson[MEASURED_PROPERTIES].update( config[MEASURED_PROPERTIES] );
				message = "Register measured properties successfully";
				SendC2DResponse( config, message, deviceId );
			}
			
			if( !config[ALERTS_PROPERTIES].is_null() )
			{
				readPropertiesJson[ALERTS_PROPERTIES].update( config[ALERTS_PROPERTIES] );
				message = "Register alerts successfully";
				SendC2DResponse( config, message, deviceId );
			}
			
			if( !config[WRITABLE_PROPERTIES].is_null() )
			{
				readPropertiesJson[WRITABLE_PROPERTIES].update( config[WRITABLE_PROPERTIES] );
			}
			
			if( !config[READABLE_PROPERTIES].is_null() )
			{
				readPropertiesJson[READABLE_PROPERTIES].update( config[READABLE_PROPERTIES] );
			}
			
			if( !config[DERIVED_PROPERTIES].is_null() )
			{
				readPropertiesJson[DERIVED_PROPERTIES].update( config[DERIVED_PROPERTIES] );
				message = "Register derived properties successfully";
				SendC2DResponse( config, message, deviceId );
			}
			
			WriteConfiguration( filePath, readPropertiesJson );
			Discoverdevice( deviceId, true );
		}
		else
		{
			logg.str("");
			logg << "DeviceManager::SetConfig  Message : Received invalid command. Command : " << configJson;
			m_exceptionLoggerObj->LogError( logg.str() );
		}
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "DeviceManager::SetConfig  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "DeviceManager::SetConfig  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

bool DeviceManager::SendC2DResponse( nlohmann::json jsonObj, std::string message, std::string deviceId )
{
	nlohmann::json c2dJson;
	c2dJson[COMMAND_INFO][COMMAND_TYPE] = "response";
	c2dJson[COMMAND_INFO][APP_NAME] = m_processName;
	c2dJson[COMMAND_INFO][APP_ID] = GetProcessIdByName( m_processName );
	c2dJson[SUB_JOB_ID] = jsonObj[SUB_JOB_ID];
	c2dJson[STATUS] = "success";
	c2dJson[TIMESTAMP] = GetTimeStamp();
	c2dJson[DEVICE_ID] = deviceId;
	c2dJson[MESSAGE] = message;
	c2dJson[TYPE] = "c2dmessage";
	
	if( m_dataCB )
	{
		m_dataCB( c2dJson );
		return true;
	}
	return false;
}


void DeviceManager::UpdateConfigInTwin( std::string deviceID, std::string subJobId )
{    
    nlohmann::json twinReportedJson;
    twinReportedJson[COMMAND_INFO][COMMAND_TYPE] = "response";
	twinReportedJson[COMMAND_INFO][APP_NAME] = m_processName;
	twinReportedJson[COMMAND_INFO][APP_ID] = GetProcessIdByName( m_processName );
    twinReportedJson[TYPE] = "reported_twin";
    twinReportedJson[SUB_JOB_ID] = subJobId;
    twinReportedJson["apps"][m_processName][STATUS] = "Running";
    
    twinReportedJson["apps"][m_processName]["asset_configuration"][deviceID]["g1_ingestion_frequency_in_ms"] = 10000;
    twinReportedJson["apps"][m_processName]["asset_configuration"][deviceID]["g1_measurement_frequency_in_ms"] = 5000;
    twinReportedJson["apps"][m_processName]["asset_configuration"][deviceID]["g1_turbo_mode_frequency_in_ms"] = 5000;
    twinReportedJson["apps"][m_processName]["asset_configuration"][deviceID]["g2_ingestion_frequency_in_ms"] = 20000;
    twinReportedJson["apps"][m_processName]["asset_configuration"][deviceID]["g2_measurement_frequency_in_ms"] = 10000;
    twinReportedJson["apps"][m_processName]["asset_configuration"][deviceID]["g2_turbo_mode_frequency_in_ms"] = 10000;
    twinReportedJson["apps"][m_processName]["asset_configuration"][deviceID]["g3_ingestion_frequency_in_ms"] = 30000;
    twinReportedJson["apps"][m_processName]["asset_configuration"][deviceID]["g3_measurement_frequency_in_ms"] = 15000;
    twinReportedJson["apps"][m_processName]["asset_configuration"][deviceID]["g3_turbo_mode_frequency_in_ms"] = 15000;
    twinReportedJson["apps"][m_processName]["asset_configuration"][deviceID]["ingestion_settings_type"] = "all_props_at_fixed_interval";
    twinReportedJson["apps"][m_processName]["asset_configuration"][deviceID]["telemetry_mode"] = "normal";
    twinReportedJson["apps"][m_processName]["asset_configuration"][deviceID]["turbo_mode_timeout_in_milli_sec"] = 120000;
    twinReportedJson["apps"][m_processName]["asset_configuration"][deviceID]["turbo_mode_timeout_in_ms"] = 60000;
    
    m_dataCB( twinReportedJson );
    
}