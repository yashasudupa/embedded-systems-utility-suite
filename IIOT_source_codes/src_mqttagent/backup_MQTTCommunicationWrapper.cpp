
#include "MQTTCommunicationWrapper.h"

MQTTCommunicationWrapper::MQTTCommunicationWrapper( nlohmann::json mqttConfig ):
	m_mqttConfigJson(mqttConfig)
{
	/* Required before calling other mosquitto functions */
	mosquitto_lib_init();
	ConnectMqttBroker();
}

MQTTCommunicationWrapper::~MQTTCommunicationWrapper()
{
	DiconnectMqttBroker();
	mosquitto_lib_cleanup();
}

//TBD = get info from file
bool MQTTCommunicationWrapper::ConnectMqttBroker()
{
	std::string hostAddress = m_mqttConfigJson["mqtt_details"]["host_address"];
	long portNumber = m_mqttConfigJson["mqtt_details"]["port_number"];
	
	/* Create a new client instance.
	 * id = NULL -> ask the broker to generate a client id for us
	 * clean session = true -> the broker should remove old sessions when we connect
	 * obj = NULL -> we aren't passing any of our private data for callbacks
	 */
	m_mosq = mosquitto_new(NULL, true, this);
	mosquitto_connect_callback_set(m_mosq, OnConnect);
	mosquitto_message_callback_set(m_mosq, OnMessage);
	mosquitto_disconnect_callback_set(m_mosq, OnDisconnect);
	
	if( m_mosq )
	{
		int rc = mosquitto_connect( m_mosq, hostAddress.c_str(), portNumber, 60 );
		
		if(rc != MOSQ_ERR_SUCCESS)
		{
			mosquitto_destroy(m_mosq);
			std::cout << "Error: " << mosquitto_strerror(rc) << std::endl;
		}
		else
		{
			std::cout << "Connection established " << std::endl;
			LoopStart();
			return true;
		}
	}
	else
	{
		std::cout << "MQTTCommunicationWrapper::ConnectMqttBroker() : Connection object not created.. " << std::endl;
	}
	return false;
}

bool MQTTCommunicationWrapper::DiconnectMqttBroker()
{
	if( m_mosq )
	{
		mosquitto_loop_stop( m_mosq, true );
		mosquitto_destroy(m_mosq);
	}
}

bool MQTTCommunicationWrapper::Publish( std::string payload, std::string topic )
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
		if(rc != MOSQ_ERR_SUCCESS)
		{
			std::cout << "Error publishing : " << mosquitto_strerror(rc) << std::endl;
		}
		else
		{
			std::cout << "Publish data successfully : Payload : " << payload << std::endl;
			return true;
		}
	}
	else
	{
		std::cout << "MQTTCommunicationWrapper::Publish() : Connection object not created.. " << std::endl;
	}
	return false;
}

bool MQTTCommunicationWrapper::Subscribe( std::string topic )
{
	if( m_mosq )
	{
		int rc = mosquitto_subscribe(m_mosq, NULL, topic.c_str(), QOS);
		if(rc != MOSQ_ERR_SUCCESS){
			fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
			/* We might as well disconnect if we were unable to subscribe */
			mosquitto_disconnect(m_mosq);
		}
		else
		{
			std::cout << "MQTTCommunicationWrapper::Subscribe() : Subscribed Successfully.. Topic :  " << topic << std::endl;
		}
	}
	else
	{
		std::cout << "MQTTCommunicationWrapper::Subscribe() : Connection object not created.. " << std::endl;
	}
}

void MQTTCommunicationWrapper::OnDisconnect(struct mosquitto *mosq, void *obj, int rc)
{
	MQTTCommunicationWrapper *self = (MQTTCommunicationWrapper*)obj;
    printf("MQTT disconnect, error: %d: %s\n",rc, mosquitto_strerror(rc));
}

void MQTTCommunicationWrapper::OnConnect(struct mosquitto *mosq, void *obj, int reason_code)
{
	MQTTCommunicationWrapper *self = (MQTTCommunicationWrapper*)obj;
	/* Print out the connection result. mosquitto_connack_string() produces an
	 * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
	 * clients is mosquitto_reason_string().
	 */
	if(reason_code != 0)
	{
		std::cout << " \n\n ************** on_connect : " << mosquitto_connack_string(reason_code) << std::endl;
		/* If the connection fails for any reason, we don't want to keep on
		 * retrying in this example, so disconnect. Without this, the client
		 * will attempt to reconnect. */
		mosquitto_disconnect(mosq);
	}
	else
	{
		std::cout << " \n\n ************** on_connect : " << mosquitto_connack_string(reason_code) << std::endl;
		self->m_connectionStatusCB();
	}
}

void MQTTCommunicationWrapper::LoopStart()
{
	int rc = mosquitto_loop_start(m_mosq);
	if(rc != MOSQ_ERR_SUCCESS)
	{
		mosquitto_destroy(m_mosq);
		std::cout << "Error: " << mosquitto_strerror(rc) << std::endl;
	}
}

void MQTTCommunicationWrapper::OnMessage(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	/* This blindly prints the payload, but the payload can be anything so take care. */
	MQTTCommunicationWrapper *self = (MQTTCommunicationWrapper*)obj;
	self->setData(msg);
}


void MQTTCommunicationWrapper::setData( const struct mosquitto_message *msg )
{
	//printf("Subscribe topic : %s, payload : %s\n", msg->topic, (char *)msg->payload);
	//callback
	m_dataCB( (char *)msg->payload );
}

void MQTTCommunicationWrapper::SendDataToLocalBrokerCB(std::function<void(std::string)> cb)
{
	m_dataCB = cb;
}

void MQTTCommunicationWrapper::MQTTConnectionStatusCB( std::function<void()> cb )
{
	m_connectionStatusCB = cb;
}