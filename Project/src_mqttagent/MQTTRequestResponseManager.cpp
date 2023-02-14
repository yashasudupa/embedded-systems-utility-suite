#include "MQTTRequestResponseManager.h"

MQTTRequestResponseManager::MQTTRequestResponseManager():
	m_cloudConnectionStatus( false )
{
	m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
	MaintainPersistency();
}

MQTTRequestResponseManager::~MQTTRequestResponseManager()
{
	
}

//return structure
ResponseStruct MQTTRequestResponseManager::ExecuteCommand( nlohmann::json dataJson )
{
	std::stringstream logg;
	ResponseStruct responseStructObj;
	try
	{
		if( !dataJson.is_null() )
		{
			if( dataJson[COMMAND_INFO][COMMAND_TYPE].is_null() )
			{
				logg << "MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : 'type' key not found in respected json. JSON : " << dataJson; 
				m_exceptionLoggerObj->LogError( logg.str() );
				responseStructObj.status = FAILURE;
				responseStructObj.responseMetaInformation = "'type' key not found in respected json";
				return responseStructObj;
			}
			
			std::string commandtype = dataJson[COMMAND_INFO][COMMAND_TYPE];
			
			if( commandtype == "app_register" ) 
			{
				responseStructObj = ValidateData( dataJson, APP_REGISTER_CASE );
				if( responseStructObj.status == SUCCESS )
				{	
					APP_DETAILS *appDetailObj = new APP_DETAILS;
					long appId = dataJson[COMMAND_INFO][APP_ID];
					appDetailObj->appName = dataJson[COMMAND_INFO][APP_NAME];
					m_appDetailMap[appId] = appDetailObj;
					m_persistentJson[REGISTER_DEVICES][appDetailObj->appName][APP_ID] = appId;
					if( !m_persistentJson.is_null() )
					{
						WriteConfiguration( COMMUNICATIONAPP_PERSISTENCY_CONFIG, m_persistentJson );
					}
				}
				return responseStructObj;
			}
			else if( commandtype == "device_register" )
			{
				long appId = dataJson[COMMAND_INFO][APP_ID];
				std::string appName = dataJson[COMMAND_INFO][APP_NAME];
				auto it = m_appDetailMap.find( appId );
				if( it != m_appDetailMap.end() )
				{
					APP_DETAILS *appDetailObj = it->second;
					std::string deviceId = dataJson[COMMAND_INFO][DEVICE_ID];
					appDetailObj->deviceIdSet.insert( deviceId );
					//nlohmann::json deviceIdInfoJson;
                    //deviceIdInfoJson[SUB_JOB_ID] = dataJson[SUB_JOB_ID];
					//deviceIdInfoJson[TYPE] = "reported_twin";
					m_persistentJson[REGISTER_DEVICES][appName][DEVICE_ID] = nullptr;
					for (auto itr = appDetailObj->deviceIdSet.begin(); itr != appDetailObj->deviceIdSet.end(); itr++)
					{
						//deviceIdInfoJson[REGISTERED_DEVICES][appName].push_back( *itr );
						m_persistentJson[REGISTER_DEVICES][appName][DEVICE_ID].push_back( *itr );
					}
					
					if( !m_persistentJson.is_null() )
					{
						WriteConfiguration( COMMUNICATIONAPP_PERSISTENCY_CONFIG, m_persistentJson );
					}
					
					std::string publishReportedJson = PUBLISH_PREFIX + m_gatewayId + COMMUNICATORAPP_RESPONSE_PREFIX ;
					responseStructObj.status = SUCCESS;
					responseStructObj.responseMetaInformation = "Device register successfully.";	
					//m_dataCB( deviceIdInfoJson, publishReportedJson );
					return responseStructObj;
				}
				responseStructObj.status = FAILURE;
				responseStructObj.responseMetaInformation = "Device register Failed.";	
				return responseStructObj;
			}
			else if( commandtype == "device_deregister" )
			{
				long appId = dataJson[COMMAND_INFO][APP_ID];
				std::string appName = dataJson[COMMAND_INFO][APP_NAME];
				std::string deviceId = dataJson[COMMAND_INFO][DEVICE_ID];
				auto it = m_appDetailMap.find( appId );
				bool flag = true;
				std::string publishReportedJson = PUBLISH_PREFIX + m_gatewayId + COMMUNICATORAPP_RESPONSE_PREFIX ;
				if( it != m_appDetailMap.end() )
				{
					APP_DETAILS *appDetailObj = it->second;
					auto it1 = appDetailObj->deviceIdSet.find( deviceId );
					nlohmann::json deviceIdInfoJson;
                    deviceIdInfoJson[SUB_JOB_ID] = dataJson[SUB_JOB_ID];
					if ( appDetailObj->deviceIdSet.empty() )
					{
						logg << "MQTTRequestResponseManager::ExecuteCommand()  GatewayID : " << m_gatewayId << ",  Message : Not single device is registerd." ; 
						m_exceptionLoggerObj->LogError( logg.str() );
						//deviceIdInfoJson[REGISTERED_DEVICES][appName] = nullptr;
						deviceIdInfoJson[APPS][appName]["asset_configuration"] = nullptr;
						
					}
					else if( it1 != appDetailObj->deviceIdSet.end() )
					{
						appDetailObj->deviceIdSet.erase (it1);
						
						deviceIdInfoJson[TYPE] = "reported_twin";
						m_persistentJson[REGISTER_DEVICES][appName][DEVICE_ID] = nullptr;
                        deviceIdInfoJson[APPS][appName]["asset_configuration"][deviceId] = nullptr;
						for (auto itr = appDetailObj->deviceIdSet.begin(); itr != appDetailObj->deviceIdSet.end(); itr++)
						{
							//deviceIdInfoJson[REGISTERED_DEVICES][appName].push_back( *itr );
							m_persistentJson[REGISTER_DEVICES][appName][DEVICE_ID].push_back( *itr );
							
						}
						
						if ( appDetailObj->deviceIdSet.empty() )
						{
                            deviceIdInfoJson[APPS][appName]["asset_configuration"] = nullptr;
							//deviceIdInfoJson[REGISTERED_DEVICES][appName] = nullptr;
							m_persistentJson[REGISTER_DEVICES][appName][DEVICE_ID] = nullptr;
						}
						
						if( !m_persistentJson.is_null() )
						{
							WriteConfiguration( COMMUNICATIONAPP_PERSISTENCY_CONFIG, m_persistentJson );
						}
						
						logg << "MQTTRequestResponseManager::ExecuteCommand()  GatewayID : " << m_gatewayId << ",  Message : Device deregistered successfully. DeviceID : " << deviceId ; 
						m_exceptionLoggerObj->LogInfo( logg.str() );
						responseStructObj.status = SUCCESS;
						responseStructObj.responseMetaInformation = "asset deregister successfully.";	
						m_dataCB( deviceIdInfoJson, publishReportedJson );	
						flag = false;
					}
				}
				
				nlohmann::json responseErrorInfoJson;
				if( flag )
				{
					responseErrorInfoJson[SUB_JOB_ID] = dataJson[SUB_JOB_ID];
					responseErrorInfoJson[TIMESTAMP] = GetTimeStamp();
					responseErrorInfoJson[MESSAGE] = "asset not registered";
					responseErrorInfoJson[STATUS] = "failure";
					responseErrorInfoJson[DEVICE_ID] = deviceId;
					responseErrorInfoJson[TYPE] = "c2dmessage";
					m_dataCB( responseErrorInfoJson, publishReportedJson );
					
					logg << "MQTTRequestResponseManager::ExecuteCommand()  GatewayID : " << m_gatewayId << ",  Message : Device not registered. DeviceID : " << deviceId ; 
					m_exceptionLoggerObj->LogInfo( logg.str() );
					responseStructObj.status = FAILURE;
					responseStructObj.responseMetaInformation = "Device not registered.";
					
				}
				else
				{
					responseErrorInfoJson[SUB_JOB_ID] = dataJson[SUB_JOB_ID];
					responseErrorInfoJson[TIMESTAMP] = GetTimeStamp();
					responseErrorInfoJson[MESSAGE] = "Deregister asset successfully";
					responseErrorInfoJson[STATUS] = "success";
					responseErrorInfoJson[DEVICE_ID] = deviceId;
					responseErrorInfoJson[TYPE] = C2DMESSAGE;
					m_dataCB( responseErrorInfoJson, publishReportedJson );
					
					logg << "MQTTRequestResponseManager::ExecuteCommand()  GatewayID : " << m_gatewayId << ",  Message : Device deregistered successfully. DeviceID : " << deviceId ; 
					m_exceptionLoggerObj->LogInfo( logg.str() );
				}
				return responseStructObj;
			}
			else if( commandtype == "response" )
			{
				responseStructObj = ValidateData( dataJson, RESPONSE_CASE );
				if( responseStructObj.status == SUCCESS )
				{
					responseStructObj = ResponseHandler( dataJson );
				}
				return responseStructObj;
			}
			else if( commandtype == "request" )
			{
				responseStructObj = ValidateData( dataJson, REQUEST_CASE );
				if( responseStructObj.status == SUCCESS )
				{
					responseStructObj = RequestHandler( dataJson );//TBD Add logs
				}
				else
				{
					nlohmann::json responseErrorInfoJson;
					responseErrorInfoJson[SUB_JOB_ID] = dataJson[SUB_JOB_ID];
					responseErrorInfoJson[TIMESTAMP] = GetTimeStamp();
					responseErrorInfoJson[MESSAGE] = responseStructObj.responseMetaInformation;
					responseErrorInfoJson[STATUS] = "failure";
					responseErrorInfoJson[TYPE] = "c2dmessage";
					ResponseHandler( responseErrorInfoJson );	
				}

				return responseStructObj;
			}
			else if( commandtype == CONNECTION_STATUS )
			{
				m_cloudConnectionStatus = dataJson[CONNECTION_STATUS];
                m_persistentJson[CONNECTION_STATUS] = m_cloudConnectionStatus;
					
                if( !m_persistentJson.is_null() )
                {
                    WriteConfiguration( COMMUNICATIONAPP_PERSISTENCY_CONFIG, m_persistentJson );
                }
                
				if( m_cloudConnectionStatus )
				{
					std::string publishRequestData = PUBLISH_PREFIX + m_gatewayId + DATACACHER_CACHED_DATA_REQUEST_PREFIX;
					m_dataCB( dataJson, publishRequestData );
				}
				responseStructObj.status = SUCCESS;
				responseStructObj.responseMetaInformation = "Publish connection status to CacherAgent Successfully";
				return responseStructObj;
			}
			else if( commandtype == CACHED_DATA )
			{
				std::string publishData = PUBLISH_PREFIX + m_gatewayId + COMMUNICATORAPP_RESPONSE_PREFIX;
				m_dataCB( dataJson, publishData );
				responseStructObj.status = SUCCESS;
				responseStructObj.responseMetaInformation = "Cached data published to GatewayAgent Successfully";
				return responseStructObj;
			}
			else if( commandtype == BROADSENS_RESPONSE )
			{
				std::string publishData = PUBLISH_PREFIX + m_gatewayId + COMMUNCATOR_TO_GW_PREFIX;
				m_dataCB( dataJson, publishData );
				responseStructObj.status = SUCCESS;
				responseStructObj.responseMetaInformation = "Broadsens data published to GatewayAgent Successfully";
				
				//std::cout << " "
				
				return responseStructObj;
			}
			#ifndef TRB_GATEWAY
			else if(commandtype == DEVICE_DATA_BACKUP)
			{
				logg.str("");
				logg << "MQTTRequestResponseManager::ExecuteCommand  - commandtype entered";
				m_exceptionLoggerObj->LogInfo( logg.str() );
				
				std::string publishRequestData = PUBLISH_PREFIX + m_gatewayId + DATACACHER_CACHED_DATA_REQUEST_PREFIX;
				m_dataCB( dataJson, publishRequestData );
			}
			#endif
			else
			{
				logg << "MQTTRequestResponseManager::ExecuteCommand()  GatewayID : " << m_gatewayId << ",  Message : Invalid command type received. Command Type : " << commandtype ; 
				m_exceptionLoggerObj->LogError( logg.str() );
				responseStructObj.status = FAILURE;
				responseStructObj.responseMetaInformation = "Invalid command type received.";
				return responseStructObj;
			}
		}
		else
		{
			logg << "MQTTRequestResponseManager::ExecuteCommand()  GatewayID : " << m_gatewayId << ",  Message : Received empty Json." ; 
			m_exceptionLoggerObj->LogError( logg.str() );
			responseStructObj.status = FAILURE;
			responseStructObj.responseMetaInformation = "Received empty Json";
			return responseStructObj;
		}
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "MQTTRequestResponseManager::ExecuteCommand  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "MQTTRequestResponseManager::ExecuteCommand  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	
	responseStructObj.status = FAILURE;
	responseStructObj.responseMetaInformation = "Unknown Exception Occured";
	return responseStructObj;
}

ResponseStruct MQTTRequestResponseManager::ValidateData( nlohmann::json &dataJson, int caseId )
{
	std::stringstream logg;
	ResponseStruct responseStructObj;
	try
	{
		switch( caseId )
		{
			case APP_REGISTER_CASE:
				{
					if( dataJson[COMMAND_INFO][APP_ID].is_null() )
					{
						logg.str( std::string() );
						logg << "MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : 'appid' key not found in respected json. JSON : " << dataJson; 
						m_exceptionLoggerObj->LogError( logg.str() );
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'appid' key not found in respected json";
						return responseStructObj;
					}
					
					if( dataJson[COMMAND_INFO][APP_NAME].is_null() )
					{
						logg.str( std::string() );
						logg << "MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : 'app_name' key not found in respected json. JSON : " << dataJson; 
						m_exceptionLoggerObj->LogError( logg.str() );
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'app_name' key not found in respected json";
						return responseStructObj;
					}
					logg.str( std::string() );
					logg << "MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : Validate Register request Successfully. "; 
					m_exceptionLoggerObj->LogInfo( logg.str() );
					responseStructObj.status = SUCCESS;
					responseStructObj.responseMetaInformation = "Validate Register request Successfully.";
					return responseStructObj;
				}
				break;
			case RESPONSE_CASE://devicemanager to cloud
				{
					if( dataJson[COMMAND_INFO][APP_ID].is_null() )
					{
						logg.str( std::string() );
						logg << "MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : 'appid' key not found in respected json. JSON : " << dataJson; 
						m_exceptionLoggerObj->LogError( logg.str() );
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'appid' key not found in respected json";
						return responseStructObj;
					}
					
					if( dataJson[COMMAND_INFO][APP_NAME].is_null() )
					{
						logg.str( std::string() );
						logg << "MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : 'app_name' key not found in respected json. JSON : " << dataJson; 
						m_exceptionLoggerObj->LogError( logg.str() );
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'app_name' key not found in respected json";
						return responseStructObj;
					}
	
					long appId = dataJson[COMMAND_INFO][APP_ID];
					auto it = m_appDetailMap.find( appId );
					
					if( it != m_appDetailMap.end() )
					{
						/*logg.str( std::string() );
						logg << "MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : Validate Response Successfully. "; 
						m_exceptionLoggerObj->LogInfo( logg.str() );*/
						responseStructObj.status = SUCCESS;
						responseStructObj.responseMetaInformation = "Validate Response Successfully.";
						return responseStructObj;
					}
					logg.str( std::string() );
					logg << "MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : Respected Application Not registerd. AppName : " << dataJson[COMMAND_INFO][APP_NAME]; 
					logg << " App_Id : " << dataJson[COMMAND_INFO][APP_ID];
                    m_exceptionLoggerObj->LogError( logg.str() );
					responseStructObj.status = FAILURE;
					responseStructObj.responseMetaInformation = "Respected Application Not registerd.";
					return responseStructObj;
				}
				break;
			case REQUEST_CASE:
				{
					bool responseStatus = false;
					std::string commandStr = dataJson[COMMAND];
					if( commandStr == SET_DEVICE_RULES || commandStr == REPLACE_DEVICE_RULES || commandStr == DELETE_DEVICE_RULES  )
					{
						//TBD validate rule engine request
						responseStructObj.status = SUCCESS;
						responseStructObj.responseMetaInformation = "Validate request Successfully.";
						return responseStructObj;
					}
                    
                    if ( !dataJson.contains( SUB_JOB_ID ) && commandStr != "register_rule_device_properties" )
                    {
                        logg.str( std::string() );
                        logg << "MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : 'sub_job_id' key not found in respected json. JSON : " << dataJson; 
                        m_exceptionLoggerObj->LogError( logg.str() );
                        responseStructObj.status = FAILURE;
                        responseStructObj.responseMetaInformation = "'sub_job_id' key not found in respected json";
                        return responseStructObj;
                    }
					
					for ( auto i : m_appDetailMap )
					{
						APP_DETAILS *appDetailsObj = i.second;
						if( commandStr == REGISTER_DEVICES || commandStr == SET_POLLING_INFO || commandStr == DEREGISTER_DEVICES || commandStr == SET_CONFIGURATION )
						{
							if( dataJson[APP_NAME].is_null() )
							{
								logg.str( std::string() );
								logg << "MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : 'app_name' key not found in respected json. JSON : " << dataJson; 
								m_exceptionLoggerObj->LogError( logg.str() );
								responseStructObj.status = FAILURE;
								responseStructObj.responseMetaInformation = "'app_name' key not found in respected json";
								return responseStructObj;
							}
							
							if( !dataJson.contains("assets") )
							{
								if( dataJson[COMMAND_INFO][APP_NAME].is_null() )
								{
									logg.str( std::string() );
									logg << "MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : 'devices' key not found in respected json. JSON : " << dataJson; 
									m_exceptionLoggerObj->LogError( logg.str() );
									responseStructObj.status = FAILURE;
									responseStructObj.responseMetaInformation = "'devices' key not found in respected json";
									return responseStructObj;
								}
							}
							
							std::string appName = dataJson[APP_NAME];
							if( appName == appDetailsObj->appName )
							{
								responseStructObj.status = SUCCESS;
								responseStructObj.responseMetaInformation = "Request Validate Successfully.";
								return responseStructObj;
							}
						}
						else //if( commandStr == SET_VALUE_CANAGE || commandStr == CHANGE_DEVICE_MODE || commandStr == SET_PROPERTIES || commandStr == "register_rule_device_properties"  )
						{
							std::string deviceId = dataJson[DEVICE_ID];
							auto it = appDetailsObj->deviceIdSet.find( deviceId );
							if( it != appDetailsObj->deviceIdSet.end() )
							{
								if( *it ==  deviceId )
								{
									dataJson[APP_NAME] = appDetailsObj->appName;
									logg.str( std::string() );
									logg << "MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : Request Validate Successfully."; 
									m_exceptionLoggerObj->LogInfo( logg.str() );
									responseStructObj.status = SUCCESS;
									responseStructObj.responseMetaInformation = "Request Validate Successfully.";
									return responseStructObj;
								}
							}
						}
					}
				}
				break;
		}	
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str( std::string() );
		logg << "MQTTRequestResponseManager::ValidateData  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str( std::string() );
		logg << "MQTTRequestResponseManager::ValidateData  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	responseStructObj.status = FAILURE;
	responseStructObj.responseMetaInformation = "Unknown exception occured.";
	return responseStructObj;
}

ResponseStruct MQTTRequestResponseManager::ResponseHandler( nlohmann::json responseJson )
{
	std::stringstream logg;
	ResponseStruct responseStructObj;
	try
	{
		std::string publishLiveData = PUBLISH_PREFIX + m_gatewayId + COMMUNICATORAPP_RESPONSE_PREFIX;
		std::string publishCachedData = PUBLISH_PREFIX + m_gatewayId + DATACACHER_CACHED_DATA_PREFIX;
		std::string publishRuleEngine = PUBLISH_PREFIX + m_gatewayId + COMMUNICATOR_RULE_ENGINE_RESPONSE_PREFIX;
		
		if( m_dataCB )
		{
			//m_cloudConnectionStatus = false; use for cached data testing
			if( ( m_cloudConnectionStatus || responseJson[TYPE] == NOTIFICATION || responseJson[TYPE] == C2DMESSAGE || responseJson[TYPE] == "reported_twin" ) && responseJson[TYPE] != RULE_ENGINE_DATA )
			{
				m_dataCB( responseJson, publishLiveData );
			}
			else if( responseJson[TYPE] == RULE_ENGINE_DATA )
			{
				m_dataCB( responseJson, publishRuleEngine );
			}
			else
			{
				m_dataCB( responseJson, publishCachedData );
			}
			responseStructObj.status = SUCCESS;
			responseStructObj.responseMetaInformation = "Publish response successfully.";
			return responseStructObj;
		}
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "MQTTRequestResponseManager::ResponseHandler  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "MQTTRequestResponseManager::ResponseHandler  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	responseStructObj.status = FAILURE;
	responseStructObj.responseMetaInformation = "Publish response Failed. Unknown exception occured.";
	return responseStructObj;
}
	
ResponseStruct MQTTRequestResponseManager::RequestHandler( nlohmann::json requestJson )
{
	std::stringstream logg;
	ResponseStruct responseStructObj;
	try
	{
		std::string publishTopic = "";
		std::string commandStr = requestJson[COMMAND];
		if( commandStr == SET_DEVICE_RULES || commandStr == REPLACE_DEVICE_RULES || commandStr == DELETE_DEVICE_RULES )
		{
            logg.str("");
		logg << "MQTTRequestResponseManager::RequestHandler  requestJson Rule Engine : " << requestJson ;
		m_exceptionLoggerObj->LogDebug( logg.str() );
        
			publishTopic = PUBLISH_PREFIX + m_gatewayId + RULE_ENGINE_REQUEST_PREFIX;
		}
		else
		{
			std::string appName = requestJson[APP_NAME];
			long appId = GetProcessIdByName( appName );
			if( appId > 0 )
			{
				std::string appIdStr = std::to_string( appId );
				publishTopic = PUBLISH_PREFIX + m_gatewayId + DEVICEAPP_PREFIX + appIdStr + REQUEST_PREFIX;
			}
			else
			{
				nlohmann::json responseErrorInfoJson;
				responseErrorInfoJson[SUB_JOB_ID] = requestJson[SUB_JOB_ID];
				responseErrorInfoJson[TIMESTAMP] = GetTimeStamp();
				responseErrorInfoJson[MESSAGE] = appName + " application not started.";
				responseErrorInfoJson[STATUS] = "failure";
				responseErrorInfoJson[TYPE] = "c2dmessage";
				ResponseHandler( responseErrorInfoJson );
			}
		}
		
		if( m_dataCB )
		{
			m_dataCB( requestJson, publishTopic );
			responseStructObj.status = SUCCESS;
			responseStructObj.responseMetaInformation = "Publish Request successfully.";
			return responseStructObj;
		}
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "MQTTRequestResponseManager::RequestHandler  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "MQTTRequestResponseManager::RequestHandler  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	responseStructObj.status = FAILURE;
	responseStructObj.responseMetaInformation = "Publish Request Failed. Unknown exception occured.";
	return responseStructObj;
}

void MQTTRequestResponseManager::RegisterCB( std::function<void(nlohmann::json, std::string)> cb )
{
	m_dataCB = cb;
}

void MQTTRequestResponseManager::SetGatewayId( std::string gatewayId )
{
	m_gatewayId = gatewayId;
}

bool MQTTRequestResponseManager::MaintainPersistency()
{
	std::stringstream logg;
	try
	{
		nlohmann::json jsonObj;
		jsonObj = ReadAndSetConfiguration( COMMUNICATIONAPP_PERSISTENCY_CONFIG );
		if( !jsonObj.is_null() )
		{
			m_persistentJson = jsonObj;
            if( !m_persistentJson[CONNECTION_STATUS].is_null() )
            {
                m_cloudConnectionStatus = m_persistentJson[CONNECTION_STATUS];
            }
			
			for ( auto& x : jsonObj[REGISTER_DEVICES].items() )
			{
				APP_DETAILS *appDetailStructObj = new APP_DETAILS;
				nlohmann::json jsonValueObj = x.value();
				appDetailStructObj->appName = x.key();
				long appId = jsonValueObj[APP_ID];
				
				for ( auto& deviceId : jsonValueObj[DEVICE_ID] )
				{
					std::string dvId = deviceId;
					appDetailStructObj->deviceIdSet.insert( dvId );
				}
				
				m_appDetailMap[appId] = appDetailStructObj;
			}
			return true;
		}
		else
		{
			logg.str("");
			logg << "MQTTRequestResponseManager::MaintainPersistency  GatewayID : " << m_gatewayId << ",  Message : Received empty persistency json from file";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "MQTTRequestResponseManager::MaintainPersistency  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "MQTTRequestResponseManager::MaintainPersistency  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	return false;
}
