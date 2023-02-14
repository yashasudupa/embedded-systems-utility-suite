#include "MQTT_Broadsense_client.h"

#define MAKEWORD( l, h )	((short)(((char)(l)) | ((short)((char)(h))) << 8))

extern "C" 
{
	void DestroyObject( MQTTBroadsensClient* object )
	{
		delete object;
	}
	
	DeviceLibraryWrapper * CreateObject( std::string processName )
	{
		return new MQTTBroadsensClient( processName ) ;
	}
}

MQTTBroadsensClient::MQTTBroadsensClient( std::string processName ):
	m_processName( processName ),m_ThreadRunning( 1 ) 
{
	m_configFilePath = "./config/" + m_processName + "/";
    
    m_commandJson[APP_NAME] = processName;
	m_commandJson[APP_ID] = GetProcessIdByName(processName);
	m_commandJson[COMMAND_TYPE] = RESPONSE;
}


MQTTBroadsensClient::~MQTTBroadsensClient()
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

nlohmann::json MQTTBroadsensClient::Discoverdevice()
{
	std::string configFilePath = m_configFilePath + REGISTER_DEVICES_JSON;
	m_discoverdeviceConfigJson = ReadAndSetConfiguration( configFilePath );
	return m_discoverdeviceConfigJson;
}

void MQTTBroadsensClient::controlcommandsprocess (nlohmann::json payload, MQTTRequestResponseManager* m_mqttRequestResponseObj, std::string control_topic)
{
	try
    {
		//std::unique_lock lk1(m_mqttRequestResponseObj->mtx);
		std::unique_lock<std::mutex> lk(m_mqttRequestResponseObj->mtx);
		
		//unsigned int priority = 0;
		m_dataCB_bs(payload, control_topic);
					
		//7. Wait until the condition satisfies from a receiver thread
		m_mqttRequestResponseObj->cv.wait(lk);
		/*
		if ((mq_receive (mqd, buffer, attr.mq_msgsize, &priority)) == -1)
			printf ("Failed to receive message\n");
		else
			printf ("Received [priority %u]: '%s'\n", priority, buffer);
			 */
		
		std::cout << "MQTTBroadsensClient::controlcommandsprocess (nlohmann::json payload) notified before strcmp" << std::endl;
		
		std::string msg = m_mqttRequestResponseObj->buffer;
		
		int no_of_times_msg_recvd = 1;
		while(std::strcmp(&msg.c_str()[0], "not received") == 0)
		{
			m_dataCB_bs(payload, control_topic);				
			usleep(100);
		
			//7. Wait until the condition satisfies from a receiver thread
			m_mqttRequestResponseObj->cv.wait(lk);
			
			std::cout << "MQTTBroadsensClient::controlcommandsprocess (nlohmann::json payload) notified after strcmp" << std::endl;
			
			/*
			if ((mq_receive (mqd, buffer, attr.mq_msgsize, &priority)) == -1)
			{
				printf ("Failed to receive message\n"); 
				continue;
			}
			else
				printf ("Received [priority %u]: '%s'\n", priority, buffer);
			*/
			
			msg = m_mqttRequestResponseObj->buffer;
			
			//Condition check for 3 retries.
			no_of_times_msg_recvd++;
			
			if(no_of_times_msg_recvd > 2)
			{
				break;
			}
		}	
		m_mqttRequestResponseObj->buffer.clear();
	}	
	catch( nlohmann::json::exception &e )
    {
        std::cout << e.id << " : " << e.what() << std::endl;
    }
    catch( ... )
    {
        std::cout << "ReadProperties::SendMQTTControlCommands() - Unknown exception occured." << std::endl;
    }
}

void MQTTBroadsensClient::SendMQTTControlCommands(void)
{
	
	try
    {
		
		/* Open the message queue for reading */
		//mqd = mq_open ("/OpenCSF_MQ", O_RDONLY | O_CREAT);
		//assert (mqd != -1);
		
		/* Get the message queue attributes */
		//assert (mq_getattr (mqd, &attr) != -1);
		
		//assert (buffer != NULL);
		
		//unsigned int priority = 0;
		
		std::string passwd_config = "\"fftbrsns\"";
		std::string passwd = "{\"passwd\":" + passwd_config;
		
		std::string start_config = "true";
		std::string start = "\"start\":" + start_config;
		
		std::string group_config = "\"04\"";
		std::string group = "\"group\":" + group_config;
		
		auto mode_config = 4;
		std::string mode = "\"mode\":" + std::to_string(mode_config);
		
		auto rate_config = 3200;
		std::string rate = "\"rate\":" + std::to_string(rate_config);
		
		auto points_config = 4096;
		std::string points = "\"points\":" + std::to_string(points_config);
		
		auto range_config = 16;
		std::string range = "\"range\":" + std::to_string(range_config);
		
		//std::string daq_payload = passwd + start + group + mode + rate + points;

		
		//int fftMeasure_config = 2;
		//std::string fftMeasure = "\fftMeasure\":" + std::to_string(fftMeasure_config) + ",";
		
		
		int filterType_config = 3;
		std::string filterType = "filterType:" + std::to_string(filterType_config);
		
		int filterOrder_config = 0;
		std::string filterOrder = "filterOrder:" + std::to_string(filterOrder_config);

		std::cout << "Before m_localBrokerObj is created" << std::endl;

		RegisterCB( std::bind(&MQTTBroadsensClient::DataPublisher, this, std::placeholders::_1, std::placeholders::_2) );

		m_localBrokerObj = new LocalBrokerCommunicationManager();
	
		m_mqttRequestResponseObj = m_mqttRequestResponseObj->GetInstance();
		
		
		if( m_localBrokerObj )
		{
			//TODO
			//1. DAQ Enable and get Sensor Info
			
			
			//std::unique_lock lk(m_mqttRequestResponseObj->mtx);
			std::unique_lock<std::mutex> lk(m_mqttRequestResponseObj->mtx);
			
			
			//m_mqttRequestResponseObj->mtx.lock();
			
			//DAQ enable
			std::string control_topic = "control/DAQ";
			
			std::string daq_payload = passwd + "," + start + "," + group + "," + 
									  mode + "," + rate + "," + points + "," + range + "}";
			
			std::cout << "ReadProperties::StartDeviceStateGetterThread()" << std::endl;
			std::cout << "daq_payload : " << daq_payload << std::endl;

			
			nlohmann::json daq_payload_json = nlohmann::json::parse(daq_payload);
			//nlohmann::json daq_payload_json = daq_payload;  
			
			m_dataCB_bs(daq_payload_json, control_topic);
			
			
			/*
			if ((mq_receive (mqd, buffer, attr.mq_msgsize, &priority)) == -1)
				printf ("Failed to receive message\n"); 
			else
				printf ("Received [priority %u]: '%s'\n", priority, buffer);
			*/
			
			
			//m_mqttRequestResponseObj->cv.wait(lk);
			
			//controlcommandsprocess (daq_payload_json, m_mqttRequestResponseObj, control_topic);
			
			// Wait Until DAQ Status disable
			//GlobalOperationsObj->cv.wait(lk);
			//usleep(100);
			// DAQ Disable and DAQ Enable
			
			m_mqttRequestResponseObj->buffer.clear();
			
			sleep(1);
			
			if (control_topic == "control/DAQ")
			{
				// Check for DAQ status
				control_topic = "control/DAQStatus";
				passwd = passwd + "}";
				nlohmann::json passwd_json = nlohmann::json::parse(passwd);
				
				m_dataCB_bs(passwd_json, control_topic);
				
				controlcommandsprocess (passwd_json, m_mqttRequestResponseObj, control_topic);
				/*
				if ((mq_receive (mqd, buffer, attr.mq_msgsize, &priority)) == -1)
					printf ("Failed to receive message\n"); 
				else
					printf ("Received [priority %u]: '%s'\n", priority, buffer);
					 */
				
					
				m_mqttRequestResponseObj->cv.wait(lk);
				//usleep(100);
				
				std::string daqstatusbool = m_mqttRequestResponseObj->buffer;
				
				std::cout << "Control/DAQ : daqstatusbool : " << daqstatusbool << std::endl;
				
				m_mqttRequestResponseObj->buffer.clear();
				//5. Wait Until DAQ Status disable
				while(daqstatusbool == "true")
				{
					m_dataCB_bs(passwd_json, control_topic);
					//GlobalOperationsObj->cv.wait(lk);
					
					controlcommandsprocess (passwd_json, m_mqttRequestResponseObj, control_topic);
					
					m_mqttRequestResponseObj->cv.wait(lk);
					/*
					if ((mq_receive (mqd, buffer, attr.mq_msgsize, &priority)) == -1)
					{
						printf ("Failed to receive message\n"); 
						continue;
					}
					else
						printf ("Received [priority %u]: '%s'\n", priority, buffer);
						 */
					
					daqstatusbool = m_mqttRequestResponseObj->buffer;
					usleep(100);
					m_mqttRequestResponseObj->buffer.clear();
					 
					
					//Waiting condition needs to be checked.
				}
				std::cout << "Control/DAQ : daqstatusbool : " << daqstatusbool << std::endl;
			}
			
			/*
			control_topic = "control/sensorList";
				
			nlohmann::json passwd_json = nlohmann::json::parse(passwd);
				
			if ((mq_receive (mqd, buffer, attr.mq_msgsize, &priority)) == -1)
				printf ("Failed to receive message\n"); 
			else
				printf ("Received [priority %u]: '%s'\n", priority, buffer);
			
			controlcommandsprocess (passwd_json, buffer, control_topic);
			
			nlohmann::json  m_devicesRegisterJson = ReadAndSetConfiguration( LOCAL_MQTT_SNS_FILE );
			for(auto & [ID , CONFIG] : m_devicesRegisterJson.items())
			{
				
				int fftSensor_config = atoi(ID.c_str());
				std::string fftSensor = "\fftSensor\":" + std::to_string(fftSensor_config) + ",";
				
				//6. Acceleration -- 0Hz - 6 KHz  -- 1 value (16G) 
			
				int fftMeasure_config = 1;
				std::string fftMeasure = "\fftMeasure\":" + std::to_string(fftMeasure_config) + ",";
				
				int cutoff_config = 0;
				std::string cutoff = "\cutoff\":" + std::to_string(cutoff_config) + ",";

				int cutoff2_config = 6000;
				std::string cutoff2 = "\cutoff2\":" + std::to_string(cutoff2_config) + ",";
				
				std::string fft_payload = passwd + fftSensor +  fftMeasure + filterType +
											filterOrder + cutoff + cutoff2;

				std::cout << "void ManageGroupData::StartDeviceStateGetterThread()" << std::endl;
				std::cout << "fft_payload : " << fft_payload << std::endl;
				
				nlohmann::json fft_payload_json = nlohmann::json::parse(fft_payload);
				controlcommandsprocess (fft_payload_json, buffer, control_topic);
				
				//8. Acceleration Time Wave(0Hz-6KHz) - X,Y,Z (16G / 16K)
				
				fftMeasure_config = 1;
				fftMeasure = "\fftMeasure\":" + std::to_string(fftMeasure_config) + ",";
				
				cutoff_config = 0;
				cutoff = "\cutoff\":" + std::to_string(cutoff_config) + ",";

				cutoff2_config = 6000;
				cutoff2 = "\cutoff2\":" + std::to_string(cutoff2_config) + ",";
				
				fft_payload = passwd + fftSensor +  fftMeasure + filterType +
											filterOrder + cutoff + cutoff2;

				std::cout << "void ManageGroupData::StartDeviceStateGetterThread()" << std::endl;
				std::cout << "fft_payload : " << fft_payload << std::endl;
					
				fft_payload_json = nlohmann::json::parse(fft_payload);
				controlcommandsprocess (fft_payload_json, buffer, control_topic);
				
				//10. Velocity time wave plot(2Hz-1KHz) - X,Y,Z (16G / 16K) 
				
				fftMeasure_config = 2;
				fftMeasure = "\fftMeasure\":" + std::to_string(fftMeasure_config) + ",";
				
				cutoff_config = 2;
				cutoff = "\cutoff\":" + std::to_string(cutoff_config) + ",";

				cutoff2_config = 1000;
				cutoff2 = "\cutoff2\":" + std::to_string(cutoff2_config) + ",";
				
				fft_payload = passwd + fftSensor +  fftMeasure + filterType +
											filterOrder + cutoff + cutoff2;

				std::cout << "void ManageGroupData::StartDeviceStateGetterThread()" << std::endl;
				std::cout << "fft_payload : " << fft_payload << std::endl;
					
				fft_payload_json = nlohmann::json::parse(fft_payload);
				controlcommandsprocess (fft_payload_json, buffer, control_topic);
			
				//12. Velocity (mm/s) RMS (2Hz-1kHz) -- 1 Value (16G)
				
				fftMeasure_config = 2;
				fftMeasure = "\fftMeasure\":" + std::to_string(fftMeasure_config) + ",";
				
				cutoff_config = 2;
				cutoff = "\cutoff\":" + std::to_string(cutoff_config) + ",";

				cutoff2_config = 1000;
				cutoff2 = "\cutoff2\":" + std::to_string(cutoff2_config) + ",";
				
				fft_payload = passwd + fftSensor +  fftMeasure + filterType +
											filterOrder + cutoff + cutoff2;

				std::cout << "void ManageGroupData::StartDeviceStateGetterThread()" << std::endl;
				std::cout << "fft_payload : " << fft_payload << std::endl;
					
				fft_payload_json = nlohmann::json::parse(fft_payload);
				controlcommandsprocess (fft_payload_json, buffer, control_topic);
				
				//14. Velocity (mm/s) FFT plot (2Hz-1kHz) --- X,Y,Z (16G / 8K) 
				fftMeasure_config = 2;
				fftMeasure = "\fftMeasure\":" + std::to_string(fftMeasure_config) + ",";
				
				cutoff_config = 2;
				cutoff = "\cutoff\":" + std::to_string(cutoff_config) + ",";

				cutoff2_config = 1000;
				cutoff2 = "\cutoff2\":" + std::to_string(cutoff2_config) + ",";
				
				fft_payload = passwd + fftSensor +  fftMeasure + filterType +
											filterOrder + cutoff + cutoff2;

				fft_payload_json = nlohmann::json::parse(fft_payload);
				controlcommandsprocess (fft_payload_json, buffer, control_topic);
				
				//16. Velocity (mm/s) FFT plot (0Hz-120Hz) (400Hz and 16K sample points) --- --- X,Y,Z (16G / 8K)
				
				fftMeasure_config = 2;
				fftMeasure = "\fftMeasure\":" + std::to_string(fftMeasure_config) + ",";
				
				cutoff_config = 0;
				cutoff = "\cutoff\":" + std::to_string(cutoff_config) + ",";

				cutoff2_config = 120;
				cutoff2 = "\cutoff2\":" + std::to_string(cutoff2_config) + ",";
				
				fft_payload = passwd + fftSensor +  fftMeasure + filterType +
											filterOrder + cutoff + cutoff2;

				fft_payload_json = nlohmann::json::parse(fft_payload);
				controlcommandsprocess (fft_payload_json, buffer, control_topic);
				
				//18. Acceleration (g) RMS (500Hz-6kHz) -- 1 Value (16G)
				
				fftMeasure_config = 1;
				fftMeasure = "\fftMeasure\":" + std::to_string(fftMeasure_config) + ",";
				
				cutoff_config = 500;
				cutoff = "\cutoff\":" + std::to_string(cutoff_config) + ",";

				cutoff2_config = 6000;
				cutoff2 = "\cutoff2\":" + std::to_string(cutoff2_config) + ",";
				
				fft_payload = passwd + fftSensor +  fftMeasure + filterType +
											filterOrder + cutoff + cutoff2;

				fft_payload_json = nlohmann::json::parse(fft_payload);
				controlcommandsprocess (fft_payload_json, buffer, control_topic);
				//Sleep for 3 days in an infinite loop inside a thread
			}
			 */
		}
		 
		
		/* Clean up the allocated memory and message queue */
		//usleep(100);
		delete m_localBrokerObj;
	}
	
	catch( nlohmann::json::exception &e )
    {
        std::cout << e.id << " : " << e.what() << std::endl;
    }
    catch( ... )
    {
        std::cout << "ReadProperties::SendMQTTControlCommands() - Unknown exception occured." << std::endl;
    }
	
	
}


bool MQTTBroadsensClient::Connectdevice ( std::string deviceId, nlohmann::json jsonObj )
{
    try
    {
        auto it = m_connectionMap.find(deviceId);
        if (it == m_connectionMap.end())
        {
			
			ExceptionLogger *exceptionLoggerObj = exceptionLoggerObj->GetInstance();
	
			if( exceptionLoggerObj )
			{
			//logg
				exceptionLoggerObj->Init( COMMUNICOTOR_AGENT_LOG_FILE, "MQTTAgent" );
			}
			else
			{

			}

			//TODO : Create device configuration
			/*

			JSON :
			{ip, port number, topics}
			*/
			
			//TODO Replace GatewayAgent with the corresponding variable
			std::string getwayID = "GatewayAgent";
			std::cout << getwayID << std::endl;
			CommunicatorManager *communicationManagerObj = new CommunicatorManager( getwayID );

			if( communicationManagerObj )
			{
				exceptionLoggerObj->LogInfo("MQTT_Broadsens::CommunicationApp :: Main  Message : CommunicatorManager object create successfully");
			}
			else
			{
				exceptionLoggerObj->LogError("MQTT_Broadsens::CommunicationApp :: Main  Message : CommunicatorManager object creation failed.");
			}
			
			GlobalOperationsObj = new GlobalOperations();
		
			std::cout << "**********************************" << std::endl;
			std::cout << "bool ReadProperties::ConnectToDevice() start" << std::endl;
			std::cout << "**********************************" << std::endl;
		
			m_ThreadRunning = 1;
			m_threadObj = std::thread([this](){
			while( m_ThreadRunning )
			{
				SendMQTTControlCommands();
				//GlobalOperationsObj->mtx.unlock();
				sleep(60*60*24*3);
			}
			});
			
            //AssetManager *assetManagerObj = new AssetManager( deviceId, m_processName );
			/*
            if( assetManagerObj )
            {
                m_connectionMap[deviceId] = assetManagerObj;
                assetManagerObj->RegisterPropertiesCB(std::bind( &MQTTBroadsensClient::PropertiesReceiver, this, std::placeholders::_1));
                assetManagerObj->LoadProperties();
            }
			 */
        }
    }
    catch( nlohmann::json::exception &e )
	{
		std::cout << e.id << " : " << e.what() << std::endl;
	}
	return true;
}

nlohmann::json MQTTBroadsensClient::LoadDeviceProperties( std::string deviceId, std::string processName )
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
                    assetManagerObj->RegisterPropertiesCB(std::bind( &MQTTBroadsensClient::PropertiesReceiver, this, std::placeholders::_1));
                    assetManagerObj->LoadProperties();
                }
            }
            else
            {
                assetManagerObj = it->second;
                assetManagerObj->RegisterPropertiesCB(std::bind( &MQTTBroadsensClient::PropertiesReceiver, this, std::placeholders::_1));
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
nlohmann::json MQTTBroadsensClient::GetDeviceState( nlohmann::json devicePropertyJson )
{
	nlohmann::json obj;	
	return obj;
}

// input = json
bool MQTTBroadsensClient::SetDeviceState( nlohmann::json devicePropertyJson )
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


short MQTTBroadsensClient::WriteSetPoint( nlohmann::json devicePropertyJson )
{
	return -1;
}


void MQTTBroadsensClient::SetConfig( nlohmann::json config )
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
		else if(configJson == SET_THRESHOLD)
		{
			
			std::string deviceId = config[DEVICE_ID];
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
            
            std::cout << "MQTT_Broadsense_client::SetConfig() : deviceId - " << deviceId << std::endl;
            
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


void MQTTBroadsensClient::PropertiesReceiver( nlohmann::json jsonObject )
{
	m_dataCB( jsonObject );
}


bool MQTTBroadsensClient::SendC2DResponse( nlohmann::json jsonObj, std::string message, std::string deviceId )
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

/**
 * @brief RegisterCB	:	This method will receive the data from GatewayAgent, RuleEngine, DeviceApp, CachingAgent
 * 									and call the execute method.
 * 
 */ 

void MQTTBroadsensClient::RegisterCB( std::function<void(nlohmann::json, std::string)> cb )
{
	m_dataCB_bs = cb;
}

/**
 * @brief DataPublisher	:	This method will receive the data from GatewayAgent, RuleEngine, DeviceApp, CachingAgent
 * 									and call the execute method.
 * 
 */ 
void MQTTBroadsensClient::DataPublisher( nlohmann::json dataJson, std::string publishTopic )
{
	std::string data = dataJson.dump();	
	m_localBrokerObj->PublishData( data, publishTopic );
}