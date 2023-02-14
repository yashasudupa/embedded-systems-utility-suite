#include "AssetManager.h"

AssetManager::AssetManager( std::string assetId, std::string processName ):
            m_assetId(assetId),
            m_processName(processName),
            m_startupFlag(true),
            m_deviceMode("normal"),
            m_turboTimeout(60000)
{
    m_configFilePath = "./config/" + m_processName + "/";
    int pid = GetProcessIdByName( processName );
    m_commandJson[APP_NAME] = processName;
	m_commandJson[APP_ID] = pid;
	m_commandJson[COMMAND_TYPE] = RESPONSE;

}

AssetManager::~AssetManager()
{
    for( auto itr = m_slaveConfigMap.begin(); itr != m_slaveConfigMap.end(); ++itr )
    {
        DEVICEINFOSTRUCT *devsInfo = itr->second;
        
        if( itr->second->readPropertiesObj )
        {
            delete itr->second->readPropertiesObj;
        }
    }
}

void AssetManager::LoadProperties()
{
    std::string slaveJsonPath = m_configFilePath + m_assetId + "_slaves.json";
    m_slaveJson = ReadAndSetConfiguration( slaveJsonPath );

    if( m_slaveJson.contains( "slaves" ) )
    {
        RegisterDevice( m_slaveJson );
    }
    
    /*std::string propertyJsonPath = m_configFilePath + m_assetId + ".json";
    m_propertyJson = ReadAndSetConfiguration( propertyJsonPath );
    std::cout << "************************************ReadProperties created for : " <<  m_propertyJson <<"\n\n";
    if( !m_propertyJson.is_null() )
    {
        std::cout << "************************************ReadProperties created for : " <<  m_propertyJson <<"\n\n";
        SetPropertyJson( m_propertyJson );
    }*/
}

void AssetManager::SetPropertyJson( nlohmann::json propertyJson )
{
    m_propertyJson = propertyJson;
     std::cout << " ********************** propertyJson : "<< propertyJson << std::endl;
     
     if(m_slaveConfigMap.empty())
     {
          std::cout << " ********************** EMPTY : \n\n";
     }
    for( auto it = m_slaveConfigMap.rbegin(); it != m_slaveConfigMap.rend(); it++ )
    {
        std::cout << " ***************** SlaveID : "<< it->first << std::endl;
        
        DEVICEINFOSTRUCT *devInfoStruct = it->second;
        ReadProperties *readPropertiesObj = (ReadProperties*)devInfoStruct->readPropertiesObj;
        readPropertiesObj->SetThreadStatus( false );
        readPropertiesObj->SetDeviceProperties( propertyJson );
        readPropertiesObj->CreatePropertiesStruct();
        readPropertiesObj->CreateCommandStruct();
        readPropertiesObj->SetThreadStatus( true );
    }
}

void AssetManager::RegisterDevice( nlohmann::json slaveJson )
{
    try
    {
        nlohmann::json twinReportedJson;
        for( auto& x : slaveJson["slaves"].items() )
        {
            std::string slaveId = x.key();
            auto it = m_slaveConfigMap.find(slaveId);
            DEVICEINFOSTRUCT *devInfoStruct;
            ReadProperties *readPropertiesObj;
            
            if( it == m_slaveConfigMap.end() )
            {
                nlohmann::json tmpJson;
                tmpJson = x.value();
                readPropertiesObj = new ReadProperties( m_assetId, slaveId, m_processName, tmpJson );
                readPropertiesObj->RegisterPropertiesCB(std::bind( &AssetManager::PropertiesReceiver, this, std::placeholders::_1));
                readPropertiesObj->UpdateConfigFile();
                readPropertiesObj->SetThreadStatus( false );
                readPropertiesObj->StartDeviceStateGetterThread();
                
                std::cout << "************************************ReadProperties created for : " <<  slaveId <<"\n\n";
                
                nlohmann::json config;
                config[SLAVE_ID] = slaveId;
                if( slaveJson.contains(SUB_JOB_ID) )
                {
                    config[SUB_JOB_ID] = slaveJson[SUB_JOB_ID];
                }
                
                SendNotificationToCloud(config, 1);
                devInfoStruct = new DEVICEINFOSTRUCT;
                devInfoStruct->readPropertiesObj = readPropertiesObj;
                devInfoStruct->connectionJson = tmpJson;
                devInfoStruct->assetId = m_assetId;
                m_slaveConfigMap[slaveId] = devInfoStruct;
                twinReportedJson["apps"][m_processName]["asset_configuration"][m_assetId]["slaves"][slaveId] = "Provisioned";
                
                std::cout << "\n\n\ntmpJson ::: " << tmpJson << std::endl;
            }
        }
        
        m_slaveJson = slaveJson;
        std::string configFilePath = m_configFilePath + m_assetId + "_slaves.json";
        WriteConfiguration( configFilePath, slaveJson);
        
        if( !twinReportedJson.is_null() )
        {
            twinReportedJson[COMMAND_INFO] = m_commandJson;
            twinReportedJson[TYPE] = "reported_twin";
            if( slaveJson.contains(SUB_JOB_ID) )
            {
                twinReportedJson[SUB_JOB_ID] = slaveJson[SUB_JOB_ID];
            }
            std::cout << "************************************ReadProperties created for : " <<  twinReportedJson <<"\n\n";
            m_propertiesCB( twinReportedJson );
            std::cout << "************************************ReadProperties created for : " <<  twinReportedJson <<"\n\n";
            twinReportedJson.clear();
        }
    }
    catch( nlohmann::json::exception &e )
	{
		std::cout << " SetConfig EXCEPTION : "<< e.id << " : " << e.what() << std::endl;
	}
}

void AssetManager::DeRegisterSlaves( nlohmann::json config )
{
    try
    {
        std::string deviceId = config[DEVICE_ID];

        nlohmann::json twinReportedJson;
        std::string slaveJsonPath = m_configFilePath + m_assetId + "_slaves.json";
        nlohmann::json readslaveJson = ReadAndSetConfiguration( slaveJsonPath );
        nlohmann::json tempSlaveJson = readslaveJson;
        for( auto& x : readslaveJson["slaves"].items() )
        {
            nlohmann::json tmpJson;
            tmpJson["slaveId"] = x.key();
            if( config.contains(SUB_JOB_ID) )
            {
                tmpJson[SUB_JOB_ID] = config[SUB_JOB_ID];
            }
            std::string slaveId = tmpJson["slaveId"];
            std::cout << " ############################################ slaveId : " << slaveId << "\n\n";
            
            if( config["slaves"][slaveId].is_null() )
            {
                twinReportedJson["apps"][m_processName]["asset_configuration"][m_assetId]["slaves"][slaveId] = "Deprovisioned";
                std::cout << " ############################################ true :m_slaveConfigMap empty " << twinReportedJson <<" \n\n";
    
                
                auto itr1 = m_slaveConfigMap.find( slaveId );
                if( itr1 != m_slaveConfigMap.end() )
                {
                    DEVICEINFOSTRUCT *devInfoStruct = itr1->second;
                    ReadProperties *readPropertiesObj = (ReadProperties*)devInfoStruct->readPropertiesObj;
                    
                    if( readPropertiesObj )
                    {
                        tempSlaveJson["slaves"].erase(slaveId);
                        readPropertiesObj->SetThreadStatus( false );
                        
                        nlohmann::json connectionFailJson;
                        connectionFailJson[TIMESTAMP] = GetTimeStamp();
                        connectionFailJson[COMMAND_INFO] = m_commandJson;
                        connectionFailJson[TYPE] = "notification";
                        connectionFailJson[MESSAGE] = slaveId + " slave deprovisioned successfully.";
                        connectionFailJson[EVENT] = "sensor deprovision";
                        if( config.contains(SUB_JOB_ID) )
                        {
                            connectionFailJson[SUB_JOB_ID] = config[SUB_JOB_ID];
                        }
                        connectionFailJson["slave_id"] = slaveId;
                        connectionFailJson[DEVICE_ID] = m_assetId;
                        m_propertiesCB( connectionFailJson );
                        m_slaveConfigMap.erase( itr1 );
                        
                        //delete readPropertiesObj;
                    }
                }
            }
        }
        
        std::cout << " **************************** twinReportedJson : " << twinReportedJson << "\n\n";
        if( !twinReportedJson.is_null() )
        {
            twinReportedJson[COMMAND_INFO] = m_commandJson;
            twinReportedJson[TYPE] = "reported_twin";
            if( config.contains(SUB_JOB_ID) )
            {
                twinReportedJson[SUB_JOB_ID] = config[SUB_JOB_ID];
            }
            std::cout << " **************************** twinReportedJson2 : " << twinReportedJson << "\n\n";
            m_propertiesCB( twinReportedJson );
        }
        
        WriteConfiguration( slaveJsonPath, tempSlaveJson );
    }
    catch( ... )
    {
        
    }
}

void AssetManager::SetConfig( nlohmann::json config )
{
	try
	{
		std::string configJson = config[COMMAND];
		transform(configJson.begin(), configJson.end(), configJson.begin(), ::tolower);
		if( configJson == SET_CONFIGURATION )
		{
            if( config["assets"][m_assetId].contains( "turbo_mode_timeout_in_ms" ) )
            {
                m_turboTimeout = config["assets"][m_assetId]["turbo_mode_timeout_in_ms"];
            }
            for( auto it = m_slaveConfigMap.begin(); it != m_slaveConfigMap.end(); ++it )
            {
                DEVICEINFOSTRUCT *devInfoStruct = it->second;
                ReadProperties *readPropertiesObj = (ReadProperties*)devInfoStruct->readPropertiesObj;
                readPropertiesObj->UpdateConfigFile();
            }
		}
        else if( configJson == DEREGISTER_DEVICES )
		{
            nlohmann::json twinReportedJson;
            nlohmann::json tempSlaveJson;
            tempSlaveJson = m_slaveJson;
            for( auto& x : m_slaveJson["slaves"].items() )
            {
                std::string slaveId = x.key();
                
                if( !config["slaves"].contains(slaveId) )
                {
                    nlohmann::json tmpJson;
                    tmpJson["slave_id"] = config["slave_id"];
                    if( config.contains(SUB_JOB_ID) )
                    {
                        tmpJson[SUB_JOB_ID] = config[SUB_JOB_ID];
                    }
                    SendNotificationToCloud(tmpJson, 2);
                    tempSlaveJson["slaves"].erase(slaveId);
                    twinReportedJson["apps"][m_processName]["asset_configuration"][m_assetId]["slaves"][slaveId] = "Deprovisioned";
                }
            }
            
            m_slaveJson = tempSlaveJson;
            std::string configFilePath = m_configFilePath + m_assetId + "_slaves.json";
            WriteConfiguration( configFilePath, m_slaveJson);
            
            if( !twinReportedJson.is_null() )
            {
                twinReportedJson[COMMAND_INFO] = m_commandJson;
                twinReportedJson[TYPE] = "reported_twin";
                if( config.contains(SUB_JOB_ID) )
                {
                    twinReportedJson[SUB_JOB_ID] = config[SUB_JOB_ID];
                }
                m_propertiesCB( twinReportedJson );
                twinReportedJson.clear();
            }
        }
		else if( configJson == CHANGE_DEVICE_MODE )
		{
			nlohmann::json errorTelemetryModeJson;
			std::string deviceMode = config[TELEMETRY_MODE];
			bool setflag = false;
			if( m_deviceMode == deviceMode )
			{
                SendNotificationToCloud( config, 5 );
			}
			else if( deviceMode == "turbo" )
			{
                setflag = true;
                m_deviceMode = "turbo";
                SendNotificationToCloud( config ,3 );
			}
			else
			{
                setflag = true;
				m_deviceMode = "normal";
                SendNotificationToCloud( config, 4 );
			}	
            
            if( setflag )
            {
                for( auto it = m_slaveConfigMap.begin(); it != m_slaveConfigMap.end(); ++it )
                {
                    DEVICEINFOSTRUCT *devInfoStruct = it->second;
                    ReadProperties *readPropertiesObj = (ReadProperties*)devInfoStruct->readPropertiesObj;
                    readPropertiesObj->SetConfig( config );
                }
                
                nlohmann::json twinReportedJson;
                twinReportedJson[COMMAND_INFO] = m_commandJson;
                twinReportedJson[TYPE] = "reported_twin";
                twinReportedJson["apps"][m_processName][STATUS] = "Running";
                twinReportedJson["apps"][m_processName]["asset_configuration"][m_assetId][TELEMETRY_MODE] = m_deviceMode;
                m_propertiesCB( twinReportedJson ); 
                
                std::cout << "Turbo Twin : " << twinReportedJson << "\n\n";
                
               if( deviceMode == "turbo" ) 
                {
                    m_flag = false;
                    m_threadObj = std::thread([this,config](){
                        long turboTimeout = m_turboTimeout + 20;
                        std::cout << turboTimeout << "******************##################\n\n";
                        
                        sleep( turboTimeout/1000 );
                        m_deviceMode = "normal";
                        
                        nlohmann::json twinReportedJson1;
                        twinReportedJson1[COMMAND_INFO] = m_commandJson;
                        twinReportedJson1[TYPE] = "reported_twin";
                        twinReportedJson1["apps"][m_processName][STATUS] = "Running";
                        twinReportedJson1["apps"][m_processName]["asset_configuration"][m_assetId][TELEMETRY_MODE] = m_deviceMode;
                        m_propertiesCB( twinReportedJson1 ); 
                        
                        nlohmann::json connectionFailJson1;
                        connectionFailJson1[COMMAND_INFO] = m_commandJson;
                        connectionFailJson1[SUB_JOB_ID] = config[SUB_JOB_ID];
                        connectionFailJson1[TYPE] = "notification";
                        connectionFailJson1[MESSAGE] = "Successfully configured to normal mode";
                        connectionFailJson1[CONFIGURATION][TELEMETRY_MODE] = "normal";
                        connectionFailJson1[TIMESTAMP] = GetTimeStamp();
                        connectionFailJson1[DEVICE_ID] = m_assetId;
                        connectionFailJson1[COMMAND_INFO] = m_commandJson;
                        m_propertiesCB( connectionFailJson1 ); 
                        
                        
                    });
                    
                    if( m_flag == true )
                    {
                        m_flag = false;
                        
                        std::cout << "Normal Twin : " << twinReportedJson << "\n\n";
                    }
                }   
            }
		}
		else if( configJson == "register_rule_device_properties" )
		{
            int sizeMap = m_slaveConfigMap.size();
            for( auto it = m_slaveConfigMap.begin(); it != m_slaveConfigMap.end(); ++it )
            {
                DEVICEINFOSTRUCT *devInfoStruct = it->second;
                ReadProperties *readPropertiesObj = (ReadProperties*)devInfoStruct->readPropertiesObj;
                readPropertiesObj->SetConfig( config );
            }
            std::cout << "AssetManager::SetConfig(): register_rule_device_properties : config : " << config << std::endl;
		}
		/*else if( configJson == SET_VALUE_CANAGE )
		{
			m_changeValueState = config["scv"];
		}*/
	}
	catch( nlohmann::json::exception &e )
	{
		std::cout <<" AssetManager :: SetConfig() [exception]: " << e.id << " : " << e.what() << std::endl;
	}
}

void AssetManager::UpdateTwin()
{
    nlohmann::json twinReportedJson;
    twinReportedJson[COMMAND_INFO] = m_commandJson;
    twinReportedJson[TYPE] = "reported_twin";
    twinReportedJson["apps"][m_processName][STATUS] = "Running";
    twinReportedJson["apps"][m_processName]["asset_configuration"][m_assetId][TELEMETRY_MODE] = m_deviceMode;
    m_propertiesCB( twinReportedJson ); 
}

void AssetManager::SendNotificationToCloud( nlohmann::json config, int caseId )
{
    nlohmann::json connectionFailJson;
    connectionFailJson[TIMESTAMP] = GetTimeStamp();
    connectionFailJson[COMMAND_INFO] = m_commandJson;
    switch(caseId)
    {
        case 1:
        {
            std::string sid = config["slave_id"];
            connectionFailJson[TYPE] = "notification";
            connectionFailJson[MESSAGE] = sid + " provisioned successfully.";
            connectionFailJson[EVENT] = "slave provisioned";
            if( config.contains( SUB_JOB_ID ) )
            {
                connectionFailJson[SUB_JOB_ID] = config[SUB_JOB_ID];
            }
            connectionFailJson["slave_id"] = config["slave_id"];
            connectionFailJson[DEVICE_ID] = m_assetId;
        }
        break;
        case 2:
        {
            std::string sid = config["slave_id"];
            connectionFailJson[TYPE] = "notification";
            connectionFailJson[MESSAGE] = sid + " slave deprovisioned successfully.";
            connectionFailJson[EVENT] = "sensor deprovision";
            if( config.contains(SUB_JOB_ID) )
            {
                connectionFailJson[SUB_JOB_ID] = config[SUB_JOB_ID];
            }
            connectionFailJson["slave_id"] = sid;
            connectionFailJson[DEVICE_ID] = m_assetId;
            std::cout << " **************************** connectionFailJson : " << connectionFailJson << "\n\n";
        }
        break;   
        case 3:
        {
            connectionFailJson[SUB_JOB_ID] = config[SUB_JOB_ID];
            connectionFailJson[TYPE] = "notification";
            connectionFailJson[MESSAGE] = "Successfully configured to turbo mode";
            connectionFailJson[CONFIGURATION][TELEMETRY_MODE] = config[TELEMETRY_MODE];
            connectionFailJson[DEVICE_ID] = m_assetId;
            connectionFailJson[TIMESTAMP] = GetTimeStamp();
            connectionFailJson[COMMAND_INFO] = m_commandJson;
        }
        break;
        
        case 4:
        {
            connectionFailJson[SUB_JOB_ID] = config[SUB_JOB_ID];
            connectionFailJson[TYPE] = "notification";
            connectionFailJson[MESSAGE] = "Successfully configured to normal mode";
            connectionFailJson[CONFIGURATION][TELEMETRY_MODE] = "normal";
            connectionFailJson[TIMESTAMP] = GetTimeStamp();
            connectionFailJson[DEVICE_ID] = m_assetId;
            connectionFailJson[COMMAND_INFO] = m_commandJson;
        }
        break;
        
        case 5:
        {
            std::string deviceMode = config[TELEMETRY_MODE];
            connectionFailJson[SUB_JOB_ID] = config[SUB_JOB_ID];
            connectionFailJson[COMMAND_INFO] = m_commandJson;
            connectionFailJson[TYPE] = "error";
            connectionFailJson[MESSAGE] = "Already configured " + deviceMode + " mode.";
            connectionFailJson[DEVICE_ID] = m_assetId;
            connectionFailJson[TIMESTAMP] = GetTimeStamp();
        }
        break;
    }
    
    if( connectionFailJson.contains(MESSAGE) )
    {
        std::cout << " **************************** connectionFailJson1 : " << connectionFailJson << "\n\n";
        m_propertiesCB( connectionFailJson );
    }
}


void AssetManager::RegisterPropertiesCB( std::function<void(nlohmann::json)> cb )
{
    std::cout <<" AssetManager :: RegisterPropertiesCB() : callback set " <<  std::endl;
	m_propertiesCB = cb;
}

void AssetManager::PropertiesReceiver( nlohmann::json jsonObj )
{
	m_propertiesCB( jsonObj );
}
