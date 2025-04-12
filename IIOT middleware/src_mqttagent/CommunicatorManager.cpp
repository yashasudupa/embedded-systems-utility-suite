#include "CommunicatorManager.h"

/**
 * @brief Create an CommunicatorManager	:	It will call the initilize method. 
 * 
 * @param std::string gatewayId 		:	it represent the cloud registerd gateway Id.
 */
CommunicatorManager::CommunicatorManager( std::string gatewayId ):
				m_gatewayId(gatewayId)
{
	m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
	if( m_exceptionLoggerObj == NULL )
	{
		exit(0);
	}
	
	InitCommunicatorManager();
}

/**
 * @brief destroy an CommunicatorManager	:	It will deinitilize the CommunicatorManager .
 */
CommunicatorManager::~CommunicatorManager()
{
	if(m_localBrokerObj)
	{
		delete m_localBrokerObj;
	}
	
	if( m_mqttRequestResponseObj )
	{
		delete m_mqttRequestResponseObj;
	}
}

/**
 * @brief InitCommunicatorManager	:	This method will initilize MQTTRequestResponseManager and LocalBrokerCommunicationManager.
 * 
 */ 
void CommunicatorManager::InitCommunicatorManager()
{
	std::stringstream logg;
	try
	{
		std::string subRegisterTopic = PUBLISH_PREFIX + m_gatewayId + DEVICEAPP_REGISTER_PREFIX;
		std::string subResponseTopic = PUBLISH_PREFIX + m_gatewayId + DEVICEAPP_RESPONSE_PREFIX;
		std::string subRequestTopic  = PUBLISH_PREFIX + m_gatewayId + COMMUNICATORAPP_REQUEST_PREFIX;
		std::string subConnectionStatusTopic = PUBLISH_PREFIX + m_gatewayId + CLOUD_CONNECTIONSTATUS_PREFIX;
		std::string subCachedDataTopic = PUBLISH_PREFIX + m_gatewayId + DATACACHER_CACHED_DATA_UPLOAD_PREFIX;
		std::string subRuleEngineTopic = PUBLISH_PREFIX + m_gatewayId + RULE_ENGINE_RESPONSE_PREFIX;
		std::string subBroadsensToCloud = PUBLISH_PREFIX + m_gatewayId + BROADSENS_TO_COMMUNICATOR_PREFIX;
		
		m_mqttRequestResponseObj = new MQTTRequestResponseManager();
		if( m_mqttRequestResponseObj )
		{
			logg.str("");
			logg << "CommunicatorManager::InitCommunicatorManager()  GatewayID : " << m_gatewayId << ",  Message : MQTTRequestResponseManager object created Successfully.";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			
			m_mqttRequestResponseObj->SetGatewayId( m_gatewayId);
			m_mqttRequestResponseObj->RegisterCB( std::bind( &CommunicatorManager::DataPublisher, this, std::placeholders::_1, std::placeholders::_2) );
		}
		else
		{
			logg.str("");
			logg << "CommunicatorManager::InitCommunicatorManager()  GatewayID : " << m_gatewayId << ",  Message : MQTTRequestResponseManager object creation Failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
        
        m_localBrokerObj = new LocalBrokerCommunicationManager();
		if( m_localBrokerObj )
		{
			logg << "CommunicatorManager::InitCommunicatorManager()  GatewayID : " << m_gatewayId << ",  Message : LocalBrokerCommunicationManager object created Successfully.";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			
			m_localBrokerObj->RegisterCB( std::bind( &CommunicatorManager::ReceiveSubscribedData, this, std::placeholders::_1) );
			m_localBrokerObj->SubscribeTopic( subRegisterTopic );
			m_localBrokerObj->SubscribeTopic( subResponseTopic );
			m_localBrokerObj->SubscribeTopic( subRequestTopic );
			m_localBrokerObj->SubscribeTopic( subConnectionStatusTopic );
			m_localBrokerObj->SubscribeTopic( subCachedDataTopic );
			m_localBrokerObj->SubscribeTopic( subRuleEngineTopic );
			m_localBrokerObj->SubscribeTopic( subBroadsensToCloud );
		}
		else
		{
			logg.str( std::string() );
			logg << "CommunicatorManager::InitCommunicatorManager()  GatewayID : " << m_gatewayId << ",  Message : LocalBrokerCommunicationManager object creation Failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
		
		SendNotificationToCloud();
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "CommunicatorManager::InitCommunicatorManager  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "CommunicatorManager::InitCommunicatorManager  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief SendNotificationToCloud	:	This method will send the restart app notification to cloud.
 * 
 */ 
void CommunicatorManager::SendNotificationToCloud()
{
	std::stringstream logg;
	try
	{
		nlohmann::json connectionFailJson;
		std::string publishNotification = PUBLISH_PREFIX + m_gatewayId + COMMUNICATORAPP_RESPONSE_PREFIX;
		connectionFailJson[TYPE] = "notification";
		connectionFailJson[MESSAGE] = "MQTTAgent Application has restarted";
		connectionFailJson[EVENT] = "application_restart";
		connectionFailJson[TIMESTAMP] = GetTimeStamp();
		connectionFailJson[COMMAND_INFO][COMMAND_TYPE] = "response";
		std::string publishMsg = connectionFailJson.dump();
		m_localBrokerObj->PublishData( publishMsg.c_str(), publishNotification );
	}
	catch( ... )
	{
		logg.str("");
		logg << "CommunicatorManager::SendNotificationToCloud  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief ReceiveSubscribedData	:	This method will receive the data from GatewayAgent, RuleEngine, DeviceApp, CachingAgent
 * 									and call the execute method.
 * 
 */ 
void CommunicatorManager::ReceiveSubscribedData( std::string data )
{
	nlohmann::json dataJson = nlohmann::json::parse(data);
	if( m_mqttRequestResponseObj )
	{
		m_mqttRequestResponseObj->ExecuteCommand( dataJson );
	}
}

/**
 * @brief ReceiveSubscribedData		:	This method will receive the data, and punlish topic. It will send the data
 * 										to the received topic.
 * 
 */ 
void CommunicatorManager::DataPublisher( nlohmann::json dataJson, std::string publishTopic )
{
	std::string data = dataJson.dump();	
	m_localBrokerObj->PublishData( data, publishTopic );
}
