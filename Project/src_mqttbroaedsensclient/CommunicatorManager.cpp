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
		//std::string subBroadsensGtmpTopic = BROADSENS_GTEMP_PREFIX;
		//std::string subBroadsensGWInfoTopic = BROADSENS_GW_INFO;
		//std::string subBroadsensGvibTopic = BROADSENS_GVIB_PREFIX;
		std::string subBroadsensFftTopic = BROADSENS_FFT_PREFIX;
		std::string subBroadsensInfoTopic = BROADSENS_SNS_INFO_PREFIX;
		std::string subBroadsensAtempTopic = BROADSENS_ATEMP_PREFIX;
		std::string subBroadsensAcceTopic = BROADSENS_ACCE_PREFIX;
		std::string subBroadsensDAQStatusTopic = BROADSENS_DAQSTATUS;
		std::string	subBroadsensMQTTfft = BROADSENS_MQTTfft;
		
		m_mqttRequestResponseObj = m_mqttRequestResponseObj->GetInstance();
		if( m_mqttRequestResponseObj )
		{
			logg.str("");
			logg << "MQTT_Broadsens::CommunicatorManager::InitCommunicatorManager()  GatewayID : " << m_gatewayId << ",  Message : MQTTRequestResponseManager object created Successfully.";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			
			m_mqttRequestResponseObj->SetGatewayId( m_gatewayId);
			m_mqttRequestResponseObj->RegisterCB( std::bind( &CommunicatorManager::DataPublisher, this, std::placeholders::_1, std::placeholders::_2) );
		}
		else
		{
			logg.str("");
			logg << "MQTT_Broadsens::CommunicatorManager::InitCommunicatorManager()  GatewayID : " << m_gatewayId << ",  Message : MQTTRequestResponseManager object creation Failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
        
        m_localBrokerObj = new LocalBrokerCommunicationManager();
		if( m_localBrokerObj )
		{
			logg << "MQTT_Broadsens::CommunicatorManager::InitCommunicatorManager()  GatewayID : " << m_gatewayId << ",  Message : LocalBrokerCommunicationManager object created Successfully.";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			
			m_localBrokerObj->RegisterCB( std::bind( &CommunicatorManager::ReceiveSubscribedData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) );
			
			/*
			m_localBrokerObj->RegisterCB(
			*/
			
			//m_localBrokerObj->SubscribeTopic( subBroadsensGtmpTopic );
			//m_localBrokerObj->SubscribeTopic( subBroadsensGWInfoTopic );
			//m_localBrokerObj->SubscribeTopic( subBroadsensGvibTopic );
			//m_localBrokerObj->SubscribeTopic( subBroadsensFftTopic );
			//m_localBrokerObj->SubscribeTopic( subBroadsensInfoTopic );
			//m_localBrokerObj->SubscribeTopic( subBroadsensAtempTopic );			
			//m_localBrokerObj->SubscribeTopic( subBroadsensAcceTopic );
			m_localBrokerObj->SubscribeTopic( subBroadsensDAQStatusTopic);
			m_localBrokerObj->SubscribeTopic( subBroadsensMQTTfft);
			
		}
		else
		{
			logg.str( std::string() );
			logg << "MQTT_Broadsens::CommunicatorManager::InitCommunicatorManager()  GatewayID : " << m_gatewayId << ",  Message : LocalBrokerCommunicationManager object creation Failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
		
		SendNotificationToCloud();
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "MQTT_Broadsens::CommunicatorManager::InitCommunicatorManager  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "MQTT_Broadsens::CommunicatorManager::InitCommunicatorManager  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
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
		logg << "MQTT_Broadsens::CommunicatorManager::SendNotificationToCloud  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief ReceiveSubscribedData	:	This method will receive the data from GatewayAgent, RuleEngine, DeviceApp, CachingAgent
 * 									and call the execute method.
 * 
 */ 
void CommunicatorManager::ReceiveSubscribedData( char *payload, std::uint32_t payloadLen, char *topic)
{
	//nlohmann::json dataJson = nlohmann::json::parse(data);
	if( m_mqttRequestResponseObj )
	{
		m_mqttRequestResponseObj->ExecuteCommand( payload, payloadLen, topic);
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
