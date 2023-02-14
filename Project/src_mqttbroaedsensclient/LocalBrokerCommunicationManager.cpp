#include "LocalBrokerCommunicationManager.h"

/**
 * @brief Create an LocalBrokerCommunicationManager	:	It will initilize the MQTTCommunicationWrapper and 
 * 														register the call backs
 */
LocalBrokerCommunicationManager::LocalBrokerCommunicationManager()
{
	std::stringstream logg;
	m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
	m_mqttConfiguration = ReadAndSetConfiguration( LOCAL_MQTT_CONFIGURATION_FILE );
	
	if( !m_mqttConfiguration.is_null() )
	
	{
		m_mqttCommunicationObj = new MQTTCommunicationWrapper( m_mqttConfiguration );
	
		if( m_mqttCommunicationObj )
		{
			std::cout << "MQTT_Broadsens::MQTTCommunicationWrapper object Created Successfully" << std::endl;
			m_mqttCommunicationObj->MQTTConnectionStatusCB(std::bind( &LocalBrokerCommunicationManager::ConnectionEstablish, this));
			m_mqttCommunicationObj->SendDataToLocalBrokerCB(std::bind( &LocalBrokerCommunicationManager::DeviceDataReceiver, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		}
		else
		{
			logg.str("");
			logg << "MQTT_Broadsens::LocalBrokerCommunicationManager::LocalBrokerCommunicationManager()  :  Message : Local Broker Communication object creation Failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
	}
}

/**
 * @brief destroy an LocalBrokerCommunicationManager	:	It will deinitilize MQTTCommunicationWrapper.
 * 
 */
LocalBrokerCommunicationManager::~LocalBrokerCommunicationManager()
{
	if( m_mqttCommunicationObj )
	{
		delete m_mqttCommunicationObj;
	}
}

/**
 * @brief destroy an DataRecevier	:	This method will bind with device data recevier call back which will receive the  
 * 										device data. 
 * 
 * @param nlohmann::json jsonObject :	content of the request which is send by cloud.
 */
void LocalBrokerCommunicationManager::DeviceDataReceiver( char * data, std::uint32_t payloadLen, char *topic )
{
	std::cout << "MQTT_Broadsens::LocalBrokerCommunicationManager::DeviceDataReceiver : data" << data << std::endl;	
	m_localdataCB( data, payloadLen, topic );
}

/**
 * @brief RegisterCB() :	Register the callback. If any subscribed topic has been received the data 
 *						 	then this call back has been required for redirect the data to particuler 
 * 							method.
 * 
 * */
void LocalBrokerCommunicationManager::RegisterCB( std::function<void(char *, std::uint32_t payloadLen, char *topic)> cb )
{
	m_localdataCB = cb;
}

//not required now: we need to implement it in future.
void LocalBrokerCommunicationManager::ConnectionEstablish()
{
	
}

/**
 * @brief destroy an PublishData	:	This method will redirect to particuler method in wrapper class 
 * 												
 * @param std::string payload		:	the actual payload/message.
 * @param std::string topic			:	the topic on which this message will be published.
 * 
 * @return : return true if successfully Published otherwise return false.
 */
bool LocalBrokerCommunicationManager::PublishData( std::string payload, std::string topic )
{
	return m_mqttCommunicationObj->Publish( payload, topic );
}

/**
 * @brief destroy an SubscribeTopic	:	This method will redirect to Publish method in wrapper class 
 * 												
 * @param std::string topic	:	the topic on which this message will be Subscribe.
 * 
 * @return : return true if successfully subscribed otherwise return false.
 */
bool LocalBrokerCommunicationManager::SubscribeTopic( std::string topic )
{
	return m_mqttCommunicationObj->Subscribe(topic);
}