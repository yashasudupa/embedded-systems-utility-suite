#include "MQTTCommunicationWrapper.h"

/**
 * @brief Create an MQTTCommunicationWrapper :	It will initilize the mosquitto broker and
 * 												try to connect with local mqtt broker.
 */
MQTTCommunicationWrapper::MQTTCommunicationWrapper( nlohmann::json mqttConfig ):
	m_mqttConfigJson( mqttConfig ),
	m_mosq( NULL )
{

	m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
	/* Required before calling other mosquitto functions */
	mosquitto_lib_init();
	ConnectMqttBroker();
}

/**
 * @brief destroy an MQTTCommunicationWrapper :	It will deinitilize the mosquitto broker,Disconnect with local broker,
												and clear the memory.
 * 
 */
MQTTCommunicationWrapper::~MQTTCommunicationWrapper()
{
	DiconnectMqttBroker();
	mosquitto_lib_cleanup();
}

/**
 * @brief ConnectMqttBroker() :	Connect to an MQTT broker.
 * 
 * @return : return true if successfully Connected with Cloud otherwise return false.
 */
bool MQTTCommunicationWrapper::ConnectMqttBroker()
{
	std::stringstream logg;
	try
	{
		std::string hostAddress = m_mqttConfigJson["mqtt_details"]["host_address"];
		long portNumber = m_mqttConfigJson["mqtt_details"]["port_number"];
		
		/* Create a new client instance.
		 * id = NULL -> ask the broker to generate a client id for us
		 * clean session = true -> the broker should remove old sessions when we connect
		 * obj = NULL -> we aren't passing any of our private data for callbacks
		 */
		m_mosq = mosquitto_new(NULL, true, this);
		mosquitto_connect_callback_set( m_mosq, OnConnect );
		mosquitto_message_callback_set( m_mosq, OnMessage );
		mosquitto_disconnect_callback_set( m_mosq, OnDisconnect );
		
		if( m_mosq )
		{
			int rc = mosquitto_connect( m_mosq, hostAddress.c_str(), portNumber, 60 );
			
			if( rc != MOSQ_ERR_SUCCESS )
			{
				mosquitto_destroy(m_mosq);
				logg.str("");
				logg << "MQTTCommunicationWrapper::ConnectMqttBroker()  :  Message : MQTT create connection failed. Error Info : " << mosquitto_strerror( rc );
				m_exceptionLoggerObj->LogError( logg.str() );
			}
			else
			{
				logg.str("");
				logg << "MQTTCommunicationWrapper::ConnectMqttBroker()  :  Message : MQTT Connection Established Successfully. ";
				m_exceptionLoggerObj->LogInfo( logg.str() );
				LoopStart();
				return true;
			}
		}
		else
		{
			logg.str("");
			logg << "MQTTCommunicationWrapper::ConnectMqttBroker()  :  Message : Connection handle not created.. ";
			m_exceptionLoggerObj->LogError( logg.str() );
			
		}
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "MQTTCommunicationWrapper::ConnectMqttBroker()  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "MQTTCommunicationWrapper::ConnectMqttBroker()  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	return false;
}

/**
 * @brief ConnectMqttBroker() :	Disconnect to an MQTT broker.
 * 
 * @return : return true if successfully disconnect otherwise return false.
 */
bool MQTTCommunicationWrapper::DiconnectMqttBroker()
{
	std::stringstream logg;
	try
	{
		if( m_mosq )
		{
			mosquitto_loop_stop( m_mosq, true );
			mosquitto_destroy(m_mosq);
			logg.str("");
			logg << "MQTTCommunicationWrapper::DiconnectMqttBroker()  :  Message : Disconnect from the MQTT broker. ";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			return true;
		}
		
		logg.str("");
		logg << "MQTTCommunicationWrapper::DiconnectMqttBroker()  :  Message : Failed to Disconnect from the MQTT broker. ";
		m_exceptionLoggerObj->LogError( logg.str() );
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "MQTTCommunicationWrapper::DiconnectMqttBroker()  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "MQTTCommunicationWrapper::DiconnectMqttBroker()  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	return false;
}

/**
 * @brief Publish() :	Publish a message on a given topic.
 * 
 * @param std::string payload	:	the actual payload/message.
 * @param std::string topic		:	the topic on which this message will be published.
 * 
 * @return : return true if successfully Published otherwise return false.
 */
bool MQTTCommunicationWrapper::Publish( std::string payload, std::string topic )
{
	std::stringstream logg;
	try
	{
		/* Publish the message
		 * mosq - our client instance
		 * *mid = NULL - we don't want to know what the message id for this message is
		 * topic = "example/temperature" - the topic on which this message will be published
		 * payloadlen = strlen(payload) - the length of our payload in bytes
		 * payload - the actual payload
		 * qos = 2 - publish with QoS 2 for this example
		 * retain = false - do not use the retained message feature for this message
		 */
		if( m_mosq )
		{
			int rc = mosquitto_publish(m_mosq, NULL, topic.c_str(), payload.length(), payload.c_str(), QOS, false);
			if( rc != MOSQ_ERR_SUCCESS )
			{
				logg.str("");
				logg << "MQTTCommunicationWrapper::Publish()  Message : Error while publishing payload. Error Info : " << mosquitto_strerror(rc);
				m_exceptionLoggerObj->LogException( logg.str() );
			}
			else
			{
				std::cout << "Publish data successfully : Payload : " << payload << std::endl;
				std::cout << "Publish data successfully : Topic : " << topic << std::endl;
				return true;
			}
		}
		else
		{
			std::cout << "MQTTCommunicationWrapper::Publish() : Connection object not created.. " << std::endl;
		}
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "MQTTCommunicationWrapper::Publish()  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "MQTTCommunicationWrapper::Publish()  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	return false;
}

/**
 * @brief Subscribe() :	Subscribe to a topic.
 *
 * @param std::string topic	:	the topic on which this message will be Subscribe.
 * 
 * @return : return true if successfully Subscribed otherwise return false.
 */
bool MQTTCommunicationWrapper::Subscribe( std::string topic )
{
	std::stringstream logg;
	try
	{
		if( m_mosq &&  strcmp( topic.c_str(), "") != 0 ) 
		{
			int rc = mosquitto_subscribe( m_mosq, NULL, topic.c_str(), QOS );
			
			if( rc != MOSQ_ERR_SUCCESS )
			{
				logg.str("");
				logg << "MQTTCommunicationWrapper::Subscribe()  Message : Error while subscribing topic. Error Info : " << mosquitto_strerror(rc); 
				m_exceptionLoggerObj->LogError( logg.str() );
				mosquitto_disconnect(m_mosq);
			}
			else
			{
				logg.str("");
				logg << "MQTTCommunicationWrapper::Subscribe()  Message : Subscribed Successfully.. Topic : " << topic; 
				m_exceptionLoggerObj->LogInfo( logg.str() );
				return true;
			}
		}
		else
		{
			logg.str("");
			logg << "MQTTCommunicationWrapper::Subscribe()  Message : Connection object not created.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "MQTTCommunicationWrapper::Subscribe()  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "MQTTCommunicationWrapper::Subscribe()  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	return false;
}

/**
 * @brief OnDisconnect() :	This is called when the broker has received the DISCONNECT command
 *							and has disconnected the client.
 *
 * @param struct mosquitto *mosq	:	the mosquitto instance making the callback.
 * @param void *obj	:	the user data provided in mosquitto_new.
 * @param int rc 	:	integer value indicating the reason for the disconnect.  A value of 0 means 
 * 						the client has called mosquitto_disconnect. Any other value indicates that 
 * 						the disconnect is unexpected.
 * 
 */
void MQTTCommunicationWrapper::OnDisconnect( struct mosquitto *mosq, void *obj, int rc )
{
	std::stringstream logg;
	MQTTCommunicationWrapper *self = (MQTTCommunicationWrapper*)obj;
	logg.str("");
	logg << "MQTTCommunicationWrapper::OnDisconnect()  Message : Disconnect from the MQTT broker.";
	self->m_exceptionLoggerObj->LogInfo( logg.str() );
}

/**
 * @brief OnConnect() 				:	This is called when the broker sends a CONNACK message in response to a connection.
 *
 * @param struct mosquitto *mosq	:	the mosquitto instance making the callback.
 * @param void *obj					:	the user data provided in mosquitto_new.
 * @param int reason_code 			:	the return code of the connection response.  
 * 										The values are defined by the MQTT protocol version in use.
 * 
 */
void MQTTCommunicationWrapper::OnConnect( struct mosquitto *mosq, void *obj, int reasonCode )
{
	std::stringstream logg;
	MQTTCommunicationWrapper *self = (MQTTCommunicationWrapper*)obj;
	try
	{
		/* Print out the connection result. mosquitto_connack_string() produces an
		 * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
		 * clients is mosquitto_reason_string().
		 */
		if( reasonCode != 0 )
		{
			logg.str("");
			logg << "MQTTCommunicationWrapper::OnConnect()  Message : Disconnect from the MQTT broker. Error Info : " << mosquitto_connack_string( reasonCode );
			self->m_exceptionLoggerObj->LogError( logg.str() );
			mosquitto_disconnect(mosq);
		}
		else
		{
			logg.str("");
			logg << "MQTTCommunicationWrapper::OnConnect()  Message : Connection established successfully with the MQTT broker.";
			self->m_exceptionLoggerObj->LogInfo( logg.str() );
			self->m_connectionStatusCB();
		}
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "MQTTCommunicationWrapper::OnConnect()  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		self->m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "MQTTCommunicationWrapper::OnConnect()  Message : Unknown exception occured.";
		self->m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief LoopStart() 	:	This is part of the threaded client interface. Call this once to start a 
 * 							new thread to process network traffic. This provides an alternative to
 *							repeatedly calling mosquitto_loop yourself.
 */
void MQTTCommunicationWrapper::LoopStart()
{
	int rc = mosquitto_loop_start( m_mosq );
	if( rc != MOSQ_ERR_SUCCESS )
	{
		mosquitto_destroy( m_mosq );
		std::cout << "Error: " << mosquitto_strerror(rc) << std::endl;
	}
}

/**
 * @brief OnMessage() 							:	Set the message callback.  This is called when a message is received 
 * 													from the broker.
 * 
 * @param struct mosquitto *mosq				:	the mosquitto instance making the callback.
 * @param void *obj								:	the user data provided in mosquitto_new.
 * @param const struct mosquitto_message *msg	:	The message data.  This variable and associated memory will be
 *													freed by the library after the callback completes. The client 
 * 													should make copies of any of the data it requires. 
 */
void MQTTCommunicationWrapper::OnMessage( struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg )
{
	/* This blindly prints the payload, but the payload can be anything so take care. */
	MQTTCommunicationWrapper *self = (MQTTCommunicationWrapper*)obj;
	self->setData(msg);
}

void MQTTCommunicationWrapper::setData( const struct mosquitto_message *msg )
{
	//printf("Subscribe topic : %s, payload : %s\n", msg->topic, (char *)msg->payload);
    
    std::stringstream logg;
    logg.str("");
    logg << "MQTTCommunicationWrapper::OnConnect()  Message : Connection established successfully with the MQTT broker.";
    m_exceptionLoggerObj->LogInfo( logg.str() );
    
	//callback
	m_dataCB( (char *)msg->payload );
}

/**
 * @brief SendDataToLocalBrokerCB() :	Set the message callback. This will called when a message is received
 *										from the broker.
 * @param std::function<void(std::string)> cb	:	this represents message callback function pointer.
 */
void MQTTCommunicationWrapper::SendDataToLocalBrokerCB( std::function<void(std::string)> cb )
{
	m_dataCB = cb;
}

/**
 * @brief MQTTConnectionStatusCB() :	Set the connection callback. This will called when 
 * 										connection established successfully.
 */
void MQTTCommunicationWrapper::MQTTConnectionStatusCB( std::function<void()> cb )
{
	m_connectionStatusCB = cb;
}