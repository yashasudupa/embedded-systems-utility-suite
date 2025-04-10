#include "CommunicatorManager.h"

/**
 * @brief Create an CommunicatorManager	:	It will call the initilize method. 
 * 
 * @param std::string gatewayId 		:	it represent the cloud registerd gateway Id.
 */
CommunicatorManager::CommunicatorManager( std::string gatewayId ):
		m_gatewayId(gatewayId), m_ThreadRunning(1), c2d_task("false"),
                no_of_sensors (0), first_iteration (0), c2d_task_thrown("false"),
                size_of_sensor_array (0)
{
	m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
	if( m_exceptionLoggerObj == NULL )
	{
		exit(0);
	}
	
	InitCommunicatorManager();
}

CommunicatorManager* CommunicatorManager::m_CommunicatorManager = nullptr;

CommunicatorManager* CommunicatorManager::GetInstance(std::string DeviceID)
{
	std::stringstream logg;
	//m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
     if (!m_CommunicatorManager)
     {
        m_CommunicatorManager = new CommunicatorManager(DeviceID);
     }
     return m_CommunicatorManager;
}

/**
 * @brief destroy an CommunicatorManager	:	It will deinitilize the CommunicatorManager .
 */
CommunicatorManager::~CommunicatorManager()
{
	if(m_externalBrokerObj)
	{
		delete m_externalBrokerObj;
	}
	
	if( m_mqttRequestResponseObj )
	{
		delete m_mqttRequestResponseObj;
	}
    
    m_threadObj.join();
    m_threadObj2.join();
}

void CommunicatorManager::controlcommandsprocess (nlohmann::json payload, MQTTRequestResponseManager* m_mqttRequestResponseObj, std::string control_topic)
{
    std::stringstream logg;
	try
	    {		
		m_dataCB_bs(payload, control_topic);					
		std::string msg = m_mqttRequestResponseObj->buffer;
		
		int no_of_times_msg_recvd = 1;
		while(std::strcmp(&msg.c_str()[0], "not received") == 0)
		{
			m_dataCB_bs(payload, control_topic);				
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
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "MQTT_Broadsens::CommunicatorManager::controlcommandsprocess  " << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "MQTT_Broadsens::CommunicatorManager::controlcommandsprocess  " << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

bool CommunicatorManager::InitialiseSensorDataMap (std::map<std::string, std::string> &data_map)
{
	std::stringstream logg;
	try
	{
		std::string passwd_config = "\"fftbrsns\"";
		std::string passwd = "{\"passwd\":" + passwd_config;
			
		std::string start_config = "true";
		std::string start = "\"start\":" + start_config;
		
		//TODO
		nlohmann::json m_SensorListJson = ReadAndSetConfiguration( LOCAL_MQTT_SNS_FILE );
		//nlohmann::json  m_SensorListJson;
		//Yet to be done
        
        m_DataPointsJson = ReadAndSetConfiguration( UC6_BROADSENS_PATH );
        
        if(!(m_SensorListJson.empty() && m_DataPointsJson.empty()))
        {
            std::string fftSensor;
            int fftSensorConfig;
            
            for( auto& [grp, Value] : m_SensorListJson.items() )
            {
                for (auto value : Value)
                {
                    for( auto& [cutoff_range, parameters_json] : m_DataPointsJson.items() )
                    {
                        long param_value = parameters_json["DAQ_M"];
                        mode_config = std::to_string(param_value);
                        std::string mode = "\"mode\":" + mode_config;
                        
                        param_value = parameters_json["Hz"];
                        rate_config = std::to_string(param_value);
                        std::string rate = "\"rate\":" + rate_config;
                        
                        param_value = parameters_json["G"];
                        range_config = std::to_string(param_value);
                        std::string range = "\"range\":" + range_config;
                        
                        param_value = parameters_json["Filter"];
                        filterType_config = std::to_string(param_value);
                        std::string filterType = "\"filterType\":" + filterType_config;
                        
                        param_value = parameters_json["Order"];
                        filterOrder_config = std::to_string(param_value);
                        std::string filterOrder = "\"filterOrder\":" + filterOrder_config;

                        param_value = parameters_json["Samp_P"];
                        points_config = std::to_string(param_value);
                        std::string points = "\"fftPoints\":" + points_config;
                        
                        int acce=1,vel=2, meas;
                        meas=(parameters_json["ms"] == "Acceleration") ? acce : vel;
                                
                        fftMeasure_config = std::to_string(meas);
                        
                        param_value = parameters_json["cutoff1"];
                        cutoff_config = std::to_string(param_value);
                        
                        param_value = parameters_json["cutoff2"];
                        cutoff2_config = std::to_string(param_value);
                        
                        //"0_6000_A":{"DAQ_M":4,"Hz":12800,"G":16,"DAQ_P":16834,"Samp_P":16834,"Filter":0,"Order":0,"cutoff1":0,"cutoff2":0,"ms":"Acceleration"}
                        int id_string = value;
                        fftSensorConfig =  id_string ;
                        fftSensor = "\"fftSensor\":" + std::to_string(fftSensorConfig);		
                        std::string fft_payload = passwd + "," + fftSensor + "," + "\"fftMeasure\":" + fftMeasure_config + "," + 
                                                  filterType + "," + filterOrder + "," + "\"cutoff\":" + cutoff_config + "," + 
                                                  "\"cutoff2\":" + cutoff2_config + "," 
                                                  + points + "}";
                        
                        std::string control_topic = "control/FFT";
                        
                        data_map.emplace(std::pair<std::string, std::string> (fft_payload, control_topic));
                    }
                }    
            }
            return true;
        }		
        return false;
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "MQTT_Broadsens::CommunicatorManager::InitialiseSensorDataMap   " << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "MQTT_Broadsens::CommunicatorManager::InitialiseSensorDataMap  GatewayID : "  << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

void CommunicatorManager::wait_for_variable_to_update(std::string &flag)
{
    int timeout = 60;
    while(timeout-- && (flag == "false"))
    {
        sleep(1);
    }
    
    flag = "true";
    return;
}

int CommunicatorManager::wait_for_mosq_to_update(const int &mosq_state)
{
    std::stringstream logg;
    
    int timeout = 60;
    while(timeout-- && (mosq_state == 0))
    {
        sleep(1);
    }
    
    if(mosq_state == 1)
    {
        return 1;
    }
    
    return -2;
}

/*
* passwd”: MQTT control password set at the gateway’s “MQTT config” page. Data type is string
• “fftSensor”: Sensor ID. Only use the number part. For example, if the sensor serial number is 
SVT200-A-00032, then you only need to use number 32. Data type is string
• “fftPoints”: FFT points for analysis. Valid numbers are 2048, 4096, 8192 and 16384. Data type is 
integer
• “fftMeasure”: Measure acceleration or velocity. 1: acceleration; 2: velocity. Data type is integer
• “filterType”: 0: no filter; 1: high pass filter; 2: low pass filter; 3: bandpass filter. Data type is 
integer
• “filterOrder”: order of the filter. Valid values are: 4, 8 or 12. If filterType is equal to 0, then this 
value will be ignored. Data type is integer
• “cutoff”: Cutoff frequency for high pass, low pass or bandpass filter. Data type is integer
• “cutoff2”: 2nd cutoff frequency. This value is only valid for bandpass filter (filterType is equal to 
3). Data type is integer
*/
void CommunicatorManager::control_fft_as_cd(nlohmann::json config)
{
    std::stringstream logg;
    try
	{
         mtx.lock();
         c2d_task = "true";
         
         m_mqttRequestResponseObj->fft_request_type = "fft_c2d_request";
         
        nlohmann::json uc6_broadsens_json = ReadAndSetConfiguration( UC6_BROADSENS_PATH );
        nlohmann::json jsonData;        
        nlohmann::json newEntry;
        
        unsigned long long sleep_time_long = config[RESTINTERVAL];
        if(sleep_time_long >= (no_of_sensors) * MIN_SLEEP_TIME)
        {   
            sleep_time = sleep_time_long;                 
            logg.str( std::string() );
            logg << "CommunicatorManager::control_fft_as_cd( ) Message : sleep_time is set : " << sleep_time; 
            m_exceptionLoggerObj->LogInfo( logg.str() ); 
            c2d_task_thrown = "true";
        }
        else
        {
            c2d_task = "false";
            
            std::string message = "Invalid sleep input given for the C2D FFT requests made";
            std::string event = "FFT C2D error";
            SendNotificationToCloud(message, event);
            mtx.unlock();
            return;
        }
        

        std::string meas;
        
        unsigned long long abs_cutoff1_Value = config[CUTOFF1];
        
        //cutoff between 0 and 6000
            
        unsigned long long abs_cutoff2_Value = config[CUTOFF2];
        
        if(!(abs_cutoff1_Value >=1 && abs_cutoff1_Value <=6000))
        {
            std::string message = "Invalid Cutoff 1 input given for the C2D FFT requests made";
            std::string event = "FFT C2D error";
            SendNotificationToCloud(message, event);
            c2d_task = "false";
            mtx.unlock();
            return;
        }

        //cutoff between 0 and 6000
            
        //int abs_cutoff2_Value = std::stoi(cutoff2_config_c2d);
        
        if(!(abs_cutoff2_Value >=1 && abs_cutoff1_Value <=6000))
        {
            std::string message = "Invalid Cutoff 2 input given for the C2D FFT requests made";
            std::string event = "FFT C2D error";
            SendNotificationToCloud(message, event);
            c2d_task = "false";
            mtx.unlock();
            return;
        }
                
        unsigned long long abs_fftMeasure_Value = config[FFTMEASURE];
        //meas=(abs_fftMeasure_Value == 1) ? "A" : "V";
        
        meas=(config[FFTMEASURE] == 1) ? "A" : "V";
        
        std::string key = std::to_string(abs_cutoff1_Value) + "_" +
                          std::to_string(abs_cutoff2_Value) + "_" +
                          meas;

        unsigned long long abs_filterType_Value = config[FILTERTYPE];
        
        meas=(config[FFTMEASURE] == 1) ? "Acceleration" : "Velocity";
        
        //uc6_broadsens_json[key]["ms"] = meas;
        
        if (abs_filterType_Value !=0 && abs_filterType_Value !=1 && abs_filterType_Value !=2 && abs_filterType_Value !=3)
        {
                //return;
            c2d_task = "false";
            std::string message = "Invalid Filter input given for the C2D FFT requests made";
            std::string event = "FFT C2D error";
            SendNotificationToCloud(message, event);
            mtx.unlock();
            return;
        }
        
        
        std::string filterType = "\"filterType\":" + std::to_string(abs_filterType_Value);
        
        unsigned long long abs_filterOrder_Value = config[FILTERORDER];
        
        if (abs_filterOrder_Value !=4 && abs_filterOrder_Value !=8 && abs_filterOrder_Value !=12)
        {
                //return;
                c2d_task = "false";
                
                std::string message = "Invalid Filter order input given for the C2D FFT requests made";
                std::string event = "FFT C2D error";
                SendNotificationToCloud(message, event);
                mtx.unlock();
                return;
        }
        
        std::string filterOrder = "\"filterOrder\":" + std::to_string(abs_filterOrder_Value);            
        
        unsigned long long abs_fftPoints_Value = config[FFTPOINTS];
        
        if (abs_fftPoints_Value!=2048 && abs_fftPoints_Value!=4096 && abs_fftPoints_Value!=8192 && abs_fftPoints_Value!=16384)
        {
                //return;
                c2d_task = "false";
                
                std::string message = "Invalid FFT points input given for the C2D FFT requests made";
                std::string event = "FFT C2D error";
                SendNotificationToCloud(message, event);
                mtx.unlock();
                return;
        }
        
        std::string points = "\"fftPoints\":" + std::to_string(abs_fftPoints_Value);        
        unsigned long long abs_rate_Value = config[RATE];
        
        if (abs_rate_Value!=50 && abs_rate_Value!=100 && abs_rate_Value!=200 && abs_rate_Value!=400 && 
            abs_rate_Value!=800 && abs_rate_Value!=1600 && abs_rate_Value!=3200 && abs_rate_Value != 6400 && abs_rate_Value != 12800 && 
            abs_rate_Value != 25600)
        {
            c2d_task = "false";
            
            std::string message = "Invalid Rate input given for the C2D FFT requests made";
            std::string event = "FFT C2D error";
            SendNotificationToCloud(message, event);
            mtx.unlock();
            return;
        }
        
        std::string rate = "\"rate\":" + std::to_string(abs_rate_Value);
        
        unsigned long long abs_mode_value = config[MODE];
        
        if (!(abs_mode_value >=0 && abs_mode_value <=9))
        {
            c2d_task = "false";
            
            std::string message = "Invalid Mode input given for the C2D FFT requests made";
            std::string event = "FFT C2D error";
            SendNotificationToCloud(message, event);
            mtx.unlock();
            return;
        }
        
        std::string mode  = "\"mode\":" + std::to_string(abs_mode_value);
        if((abs_fftMeasure_Value != 1) && (abs_fftMeasure_Value != 2))
        {
            //return;
            c2d_task = "false";
            
            std::string message = "Invalid FFT Measure input given for the C2D FFT requests made";
            std::string event = "FFT C2D error";
            SendNotificationToCloud(message, event);
            mtx.unlock();
            return;
        }
        
        newEntry = {
            {"ms", meas},
            {"Hz", abs_rate_Value},
            {"DAQ_P", abs_fftPoints_Value},
            {"G", },
            {"Samp_P", abs_fftPoints_Value},
            {"Filter", abs_filterType_Value},
            {"Order", abs_filterOrder_Value},
            {"cutoff1", abs_cutoff1_Value},
            {"cutoff2", abs_cutoff2_Value}
        };
        
        addOrUpdateEntry(jsonData, key, newEntry);
    
        if(m_externalBrokerObj->m_mqttCommunicationObj->mosq_state)
        {
            
            std::string passwd_config = "\"fftbrsns\"";
            std::string passwd = "{\"passwd\":" + passwd_config;
            
            m_mqttRequestResponseObj->br_cutoff = abs_cutoff1_Value;
            
            m_mqttRequestResponseObj->br_cutofftwo = abs_cutoff2_Value;
            
            nlohmann::json m_SensorListJson = ReadAndSetConfiguration( LOCAL_MQTT_SNS_FILE );
            //nlohmann::json  m_SensorListJson;
            //Yet to be done
            std::string fftSensor;
            std::string fftSensorConfig;
            for( auto& [grp, Value] : m_SensorListJson.items() )
            {
                for (auto value : Value)
                {
                    int value_temp = value;
                    std::string id_string = std::to_string(value_temp);
                    fftSensorConfig =  id_string ;
                    fftSensor = "\"fftSensor\":" + fftSensorConfig;		
                    
                    std::string fft_payload = passwd + "," + points + "," + "\"fftMeasure\":" + 
                                              std::to_string(abs_fftMeasure_Value) + "," + filterType + "," + filterOrder + "," + "\"cutoff\":" + 
                                              std::to_string(abs_cutoff1_Value) + "," + mode + "," + rate + "," + "\"cutoff2\":" + 
                                              std::to_string(abs_cutoff2_Value) + "," + fftSensor + "}";
                                      
                    std::string control_topic = "control/FFT";
                    
                    nlohmann::json daq_payload_json = nlohmann::json::parse(fft_payload);
                    
                    m_dataCB_bs(daq_payload_json, control_topic);
                    if(!receive_with_timeout())
                    {
                        std::string message = "Did not receive response for the C2D FFT requests made";
                        std::string event = "FFT C2D error";
                        SendNotificationToCloud(message, event);
                        mtx.unlock();
                    }
                }
            }
        }
        c2d_task = "false";
        c2d_task_thrown = "true";
        mtx.unlock();
    }
    catch(nlohmann::json::exception &e)
	{
        c2d_task = "false";
        mtx.unlock();
		logg.str("");
		logg << "MQTT_Broadsens::CommunicatorManager::control_fft_as_cd  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
        c2d_task = "false";
        mtx.unlock();
		logg.str("");
		logg << "MQTT_Broadsens::CommunicatorManager::control_fft_as_cd  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

bool CommunicatorManager::receive_with_timeout() 
{	
    std::stringstream logg;
    try
	{
        auto start_time = std::chrono::steady_clock::now();
        while (true) {

            // Check if a message has been received
            if (m_externalBrokerObj->m_mqttCommunicationObj->g_message_payload != NULL) {                
                ptr_to_message_payload = m_externalBrokerObj->m_mqttCommunicationObj->g_message_payload;
                
                m_externalBrokerObj->m_mqttCommunicationObj->g_message_payload = NULL;
                return true;
            }

            // Check if the timeout has elapsed
            auto current_time = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count() > TIMEOUT_MS) {
                std::cout << "CommunicatorManager::receive_with_timeout() payload : Timeout has occurred" << std::endl;
                m_mqttRequestResponseObj->mtx.unlock();
                m_mqttRequestResponseObj->app_sync_flag = "true";
                m_mqttRequestResponseObj->req_response_sync_mqttfft = "true";
                m_mqttRequestResponseObj->req_response_sync = "true";
                m_mqttRequestResponseObj->sensors_list_updated = "true";
                return false;
            }
        }
        return false;
    }
    catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "MQTT_Broadsens::CommunicatorManager::receive_with_timeout()  GatewayID : " << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "MQTT_Broadsens::CommunicatorManager::receive_with_timeout()  GatewayID : " << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

void CommunicatorManager::SendSensorDataList ()
{
    std::stringstream logg;
    try
	{
        
        std::string passwd_config = "\"fftbrsns\"";
        std::string passwd = "{\"passwd\":" + passwd_config;
        
        std::string control_topic = "control/sensorList";
        
        std::string daq_payload = passwd + "}";
        
        if(m_externalBrokerObj->m_mqttCommunicationObj->mosq_state)
        {					
            nlohmann::json daq_payload_json = nlohmann::json::parse(daq_payload);
            while(c2d_task == "true");
            
            
            m_dataCB_bs(daq_payload_json, control_topic);
            
            if(!receive_with_timeout())
            {
                std::string message = "Error receiving response after issuing the SensorsList Control command";
                std::string event = "No response from Broadsens";
                SendNotificationToCloud(message, event);
            }
        }	
    }
    catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "MQTT_Broadsens::CommunicatorManager::SendSensorDataList()  GatewayID : "  << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "MQTT_Broadsens::CommunicatorManager::SendSensorDataList()  GatewayID : "  << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

void CommunicatorManager::DisableDAQ(struct mosquitto *mosq)
{
    std::stringstream logg;
    try
	{
        std::string passwd_config = "\"fftbrsns\"";
        std::string passwd = "{\"passwd\":" + passwd_config;
            
        std::string start_config = "false";
        std::string start_daq = "\"start\":" + start_config;

        std::string daq_payload = passwd + "," + start_daq + "}";
        
        std::string control_topic = "control/DAQ";
        
        if(m_externalBrokerObj->m_mqttCommunicationObj->mosq_state)
        {					
            nlohmann::json daq_payload_json = nlohmann::json::parse(daq_payload);
            m_dataCB_bs(daq_payload_json, control_topic);    
            if(!receive_with_timeout())
            {
                std::string message = "Error receiving response after issuing the DAQStatus Control command";
                std::string event = "No response from Broadsens";
                SendNotificationToCloud(message, event);
            }
        }
    }
    catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "CommunicatorManager::DisableDAQ()  GatewayID : "  << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "CommunicatorManager::DisableDAQ()  GatewayID : "  << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

int CommunicatorManager::SendFFTRequests (const auto & parameters, const auto & Sensors)
{
    std::stringstream logg;
    std::string sensor_config;
    try
	{
        std::stringstream logg;
        std::string meas;
        nlohmann::json daq_payload_json;
        auto it = FFTDataMap.rbegin();
    
        nlohmann::json m_SensorListJson = ReadAndSetConfiguration( LOCAL_MQTT_SNS_FILE );
        
        for (const auto & sensorId : Sensors)
        { 
            size_of_sensor_array += 1;
        }
        
        //{"P1":[184,187]}
        if(!m_SensorListJson.empty())
        {
            for (const auto & sensorId : Sensors)
            {            
                std::stringstream logg;
                if(no_of_sensors + 1 == size_of_sensor_array && m_mqttRequestResponseObj->fft_request_type == "last_request_iteration")
                {
                   m_mqttRequestResponseObj->fft_request_type = "last_request_iteration_last_value"; 
                }
                
                auto now = std::chrono::steady_clock::now();
                m_mqttRequestResponseObj->mtx.try_lock_until(now + std::chrono::seconds(20));
                
                for (it = FFTDataMap.rbegin(); it != FFTDataMap.rend(); ++it)
                {
                    if(m_externalBrokerObj->m_mqttCommunicationObj->mosq_state)
                    {	
                        daq_payload_json = nlohmann::json::parse(it->first);
                        
                        meas=(daq_payload_json[FFTMEASURE] == 1) ? "A" : "V";
                	std::string measurement = parameters["ms"];
                        measurement = measurement.c_str()[0];
                                                
                        if(parameters["cutoff1"] == daq_payload_json["cutoff"] && 
                            parameters["cutoff2"] == daq_payload_json["cutoff2"] &&
                            sensorId == daq_payload_json["fftSensor"] && 
                            measurement == meas)
                        {                            
                            m_mqttRequestResponseObj->br_cutoff = daq_payload_json["cutoff"];
                            m_mqttRequestResponseObj->br_cutofftwo = daq_payload_json["cutoff2"];
                            m_mqttRequestResponseObj->DAQ_Points = daq_payload_json["fftPoints"];
                            break;
                        }
                    }
		}
                
                if(!m_DataPointsJson.empty())
                {
                    std::string key = std::to_string(m_mqttRequestResponseObj->br_cutoff) + "_" +
                                      std::to_string(m_mqttRequestResponseObj->br_cutofftwo) + "_" +
                                      meas;
                    
                    m_mqttRequestResponseObj->Hz = m_DataPointsJson[key]["Hz"];
                }
                
                if(sensor_config != daq_payload_json["fftSensor"].dump())
                {
                    sensor_config = daq_payload_json["fftSensor"].dump();
                    
                    logg.str("");
                    logg << "CommunicatorManager::SendFFTRequests Sensor ID : " << sensor_config;
                    m_exceptionLoggerObj->LogInfo( logg.str() );
                    no_of_sensors += 1;
                }
                
                std::string message = "sid:" + sensor_config + " " + std::to_string(m_mqttRequestResponseObj->br_cutoff) + 
                                                                    ":" + std::to_string(m_mqttRequestResponseObj->br_cutofftwo) + 
                                                                    "_" + *meas.c_str();
                std::string event = "FFT_DATA";
                SendNotificationToCloud(message, event);

                while(c2d_task == "true");
                mtx.lock();
                m_dataCB_bs(daq_payload_json, it->second);
                mtx.unlock();
                
                logg.str("");
                logg << "CommunicatorManager::SendFFTRequests fft request point : " << daq_payload_json << std::endl; 
                m_exceptionLoggerObj->LogInfo( logg.str() );
                
                if(!receive_with_timeout())
                {
                    std::string message = "sid:" + sensor_config + " " + std::to_string(m_mqttRequestResponseObj->br_cutoff) + 
                                                                    ":" + std::to_string(m_mqttRequestResponseObj->br_cutofftwo) + 
                                                                    "_" + *meas.c_str();;
                    std::string event = "FFT_DATA response error";
                    SendNotificationToCloud(message, event);
                }
                
                wait_for_variable_to_update(m_mqttRequestResponseObj->req_response_sync_mqttfft);                
                m_mqttRequestResponseObj->mtx.unlock();

                if(m_mqttRequestResponseObj->fft_request_type == "last_request_iteration_last_value" )
                {
                    logg.str("");
                    logg << "CommunicatorManager::SendFFTRequests last_request_iteration_last_value entered"; 
                    m_exceptionLoggerObj->LogInfo( logg.str() );
                    
                    auto now = std::chrono::steady_clock::now();
                    m_mqttRequestResponseObj->mtx.try_lock_until(now + std::chrono::seconds(20));
                    m_mqttRequestResponseObj->mtx.unlock();
                }

                m_mqttRequestResponseObj->req_response_sync_mqttfft = "false";
                if(!m_externalBrokerObj->m_mqttCommunicationObj->mosq_state)
                {
                    return -2;
                }
            }
        }
        return 1;
    }
    catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "CommunicatorManager::SendFFTRequests()  GatewayID : "  << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "CommunicatorManager::SendFFTRequests()  GatewayID : "  << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

int CommunicatorManager::UpdateSensorsList (auto start_time)
{
    std::stringstream logg;
    try
	{
        while(c2d_task == "true");
                
        std::string passwd_config = "\"fftbrsns\"";
        std::string passwd = "{\"passwd\":" + passwd_config;
            
        std::string start_config = "true";
        std::string start_daq = "\"start\":" + start_config;
        
        long daq_mode = 4;
        std::string mode = "\"mode\":" + std::to_string(daq_mode);
        
        //int rate_config = 3200;
        long rate_hz = 25600;
        std::string rate = "\"rate\":" + std::to_string(rate_hz);
        
        long daq_points = 1024;
        std::string points = "\"points\":" + std::to_string(daq_points);
        
        //int range_config = 16;
        long daq_range = 2;
        std::string range = "\"range\":" + std::to_string(daq_range);

        //DAQ enable
        std::string control_topic = "control/DAQ";
        
        static std::string first_time = "true";
        static nlohmann::json m_SensorListJson;
        if(first_time == "true")
        {
            m_SensorListJson = ReadAndSetConfiguration( LOCAL_MQTT_SNS_FILE );
            first_time = "false";
        }
            
        //{"P1":[184,187]}
        for( auto& [grp, Value] : m_SensorListJson.items() )
        {
            
            control_topic = "control/DAQ";
            std::string group_config = "\"" + grp + "\"";
            std::string group = "\"group\":" + group_config;		
            
            std::string daq_payload = passwd + "," + start_daq + "," + group + "," + 
                      mode + "," + rate + "," + points + "," + range + "}";
            
            std::stringstream logg;
            
            while(c2d_task == "true");
            sleep(1);
            
            auto status = wait_for_mosq_to_update(m_externalBrokerObj->m_mqttCommunicationObj->mosq_state);
            
            if (status == -2)
            {
                return -2;
            }
            
            if(m_externalBrokerObj->m_mqttCommunicationObj->mosq_state)
            {					
                nlohmann::json daq_payload_json = nlohmann::json::parse(daq_payload);
                m_dataCB_bs(daq_payload_json, control_topic);        
                                        
                logg.str("");
                logg << "CommunicatorManager::UpdateSensorsList () daq_payload : " << daq_payload;
                m_exceptionLoggerObj->LogInfo( logg.str() );
                
                daq_group = grp;
                std::string message = "DAQ request for group : "+ daq_group + " sent successfully";
                std::string event = "DAQ request sent successfully";
                SendNotificationToCloud(message, event);

                sleep(1);
                m_mqttRequestResponseObj->app_sync_flag = "false";
                MonitorDAQStatus(start_time);
                start_time = std::chrono::steady_clock::now();
                sleep(2);
            }
            
            
            auto now = std::chrono::steady_clock::now();
            m_mqttRequestResponseObj->mtx.try_lock_until(now + std::chrono::seconds(20));                
        }
        
        static int timeout = 3;
        while(timeout-->0)
        {                        
            SendSensorDataList();
            wait_for_variable_to_update(m_mqttRequestResponseObj->sensors_list_updated);
            m_mqttRequestResponseObj->sensors_list_updated = "false";
            m_mqttRequestResponseObj->mtx.unlock();
            m_mqttRequestResponseObj->no_of_sns_info++;
            
            logg.str("");
            logg << "CommunicatorManager::UpdateSensorsList () Sensor List called after DAQ is enabled" << std::endl; 
            m_exceptionLoggerObj->LogInfo( logg.str() );
            
            sensor_list_new_json = ReadAndSetConfiguration(LOCAL_MQTT_SNS_FILE);
            if(!sensor_list_new_json.empty())
            {
                break;
            }
            {
                auto start_time = std::chrono::steady_clock::now();
                sleep(1);
                //timeout = timeout-1;
                int status = UpdateSensorsList(start_time);
                if (status == -2)
                {
                    return -2;
                }
            }
            
        }                    
        //InitialiseSensorDataMap
        if(sensors_list_json != sensor_list_new_json && sensor_list_new_json.empty() == 0)
        {
            FFTDataMap.clear();
            sensors_list_json = sensor_list_new_json;
            
            auto now = std::chrono::steady_clock::now();
            m_mqttRequestResponseObj->mtx.try_lock_until(now + std::chrono::seconds(20));
            
            if(InitialiseSensorDataMap(FFTDataMap) == false)
            {
                return -1;
            }
            m_mqttRequestResponseObj->mtx.unlock();
        }
        first_time = "true";
        return 1;
    }
    catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "CommunicatorManager::UpdateSensorsList  GatewayID : "  << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "CommunicatorManager::UpdateSensorsList  GatewayID : "  << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}


/* {"0_120_V":{"DAQ_M":4,"DAQ_P":16384,"Filter":0,"G":16,"Hz":400,"Order":0,"Samp_P":16384,"cutoff1":0,"cutoff2":120,"ms":"Velocity"},
 * "0_6000_A":{"DAQ_M":4,"DAQ_P":16384,"Filter":0,"G":16,"Hz":12800,"Order":0,"Samp_P":16384,"cutoff1":0,"cutoff2":6000,"ms":"Acceleration"},
 * "0_6000_V":{"DAQ_M":4,"DAQ_P":16384,"Filter":0,"G":16,"Hz":12800,"Order":0,"Samp_P":16384,"cutoff1":0,"cutoff2":6000,"ms":"Velocity"},
 * "2_1000_V":{"DAQ_M":4,"DAQ_P":16384,"Filter":0,"G":16,"Hz":3200,"Order":0,"Samp_P":16384,"cutoff1":2,"cutoff2":1000,"ms":"Velocity"},
 * "500_6000_A":{"DAQ_M":4,"DAQ_P":16384,"Filter":3,"G":16,"Hz":12800,"Order":4,"Samp_P":16384,"cutoff1":500,"cutoff2":6000,"ms":"Acceleration"}}
*/

/*Sensor List
DAQ enable based on Group
Sensor List
DAQ enable Based on Freq(i.e 0-6000) and send FFT
DAQ enable Based on Freq(i.e 2-1000) and send FFT
*/

int CommunicatorManager::EnableDAQ (auto start_time)
{
    std::stringstream logg;
    try
	{
        nlohmann::json m_DataPointsJson = ReadAndSetConfiguration( UC6_BROADSENS_PATH );
        int no_of_times_zero_sixt = 0;
        nlohmann::json fftparameter_points;
        for( auto& [key, parameters] : m_DataPointsJson.items() )
        {
            no_of_sensors = 0;
            fftparameter_points = parameters;
            while(c2d_task == "true");
            
            m_mqttRequestResponseObj->fft_request_type = "";
            if(key == "0_120_V")
            {
                m_mqttRequestResponseObj->fft_request_type = "first_request_iteration";                
            }
            
            if(key == "500_6000_A")
            {
                m_mqttRequestResponseObj->fft_request_type = "last_request_iteration";
            }
            //Send Sensors List first
            if(key == "0_6000_A" || key == "0_6000_V")
            {
                no_of_times_zero_sixt++;
            }
            else
            {
                no_of_times_zero_sixt = 0;
            }
            
            
            std::string passwd_config = "\"fftbrsns\"";
            std::string passwd = "{\"passwd\":" + passwd_config;
                
            std::string start_config = "true";
            std::string start_daq = "\"start\":" + start_config;
            
            long daq_mode = parameters["DAQ_M"];
            std::string mode = "\"mode\":" + std::to_string(daq_mode);
            
            //int rate_config = 3200;
            long rate_hz = parameters["Hz"];
            std::string rate = "\"rate\":" + std::to_string(rate_hz);
            
            long points_config;
            if(!parameters.empty())
            {
                long points_long = parameters["Samp_P"];
                if(!m_DataPointsJson.empty())
                {
                    points_config = points_long;
                }
                else
                {
                    points_config = 16384;
                }
            }
                    
            long daq_points = parameters["DAQ_P"];
            std::string points = "\"points\":" + std::to_string(daq_points);
            
            //int range_config = 16;
            long daq_range = parameters["G"];
            std::string range = "\"range\":" + std::to_string(daq_range);

            //DAQ enable
            std::string control_topic = "control/DAQ";
            
            nlohmann::json m_SensorListJson = ReadAndSetConfiguration( LOCAL_MQTT_SNS_FILE );
            if(!m_SensorListJson.empty())
            {
                for( auto& [grp, Value] : m_SensorListJson.items() )
                {
                    if(no_of_times_zero_sixt == 0 || no_of_times_zero_sixt == 1)
                    {
                        control_topic = "control/DAQ";
                        std::string group_config = "\"" + grp + "\"";
                        std::string group = "\"group\":" + group_config;		
                        
                        std::string daq_payload = passwd + "," + start_daq + "," + group + "," + 
                                  mode + "," + rate + "," + points + "," + range + "}";
                        
                        std::stringstream logg;
                        
                        while(c2d_task == "true");
                        sleep(1);
                        
                        auto status = wait_for_mosq_to_update(m_externalBrokerObj->m_mqttCommunicationObj->mosq_state);
                        
                        if (status == -2)
                        {
                            return -2;
                        }
                        
                        if(m_externalBrokerObj->m_mqttCommunicationObj->mosq_state)
                        {					
                            nlohmann::json daq_payload_json = nlohmann::json::parse(daq_payload);
                            m_dataCB_bs(daq_payload_json, control_topic);        
                                                    
                            logg.str("");
                            logg << "CommunicatorManager::EnableDAQ () daq_payload : " << daq_payload;
                            m_exceptionLoggerObj->LogInfo( logg.str() );
                            
                            daq_group = grp;
                            std::string message = "DAQ request for group : "+ daq_group + " sent successfully";
                            std::string event = "DAQ request sent successfully";
                            SendNotificationToCloud(message, event);

                            sleep(1);
                            m_mqttRequestResponseObj->app_sync_flag = "false";
                            MonitorDAQStatus(start_time);
                            start_time = std::chrono::steady_clock::now();
                            sleep(2);
                        }
                    }
                    auto status = SendFFTRequests(fftparameter_points, Value);
                    if (status == -2)
                    {
                        return -2;
                    }
                }
                size_of_sensor_array = 0;
            }
        }
    }
    catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "CommunicatorManager::EnableDAQ()  GatewayID : "  << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "CommunicatorManager::EnableDAQ()  GatewayID : "  << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

void CommunicatorManager::MonitorDAQStatus (auto start_time)
{
    std::stringstream logg;
    try
	{
        std::string control_topic = "control/DAQStatus";
        
        std::string daq_payload;
        
        std::string passwd_config = "\"fftbrsns\"";
        std::string passwd = "{\"passwd\":" + passwd_config;
        
        daq_payload = passwd + "}";
        if(m_externalBrokerObj->m_mqttCommunicationObj->mosq_state)
        {					
            nlohmann::json daq_payload_json = nlohmann::json::parse(daq_payload);
            m_dataCB_bs(daq_payload_json, control_topic);
            
            
            if(!receive_with_timeout())
            {
                std::string message = "Error receiving response after issuing the DAQStatus Control command";
                std::string event = "No response from Broadsens";
                SendNotificationToCloud(message, event);
                return;
            }
            else
            {
                sleep(2);
                wait_for_variable_to_update(m_mqttRequestResponseObj->app_sync_flag);
                m_mqttRequestResponseObj->app_sync_flag = "false";
                
                std::chrono::duration<double, std::milli> elapsed {std::chrono::steady_clock::now() - start_time};
              
                if(m_mqttRequestResponseObj->daqstatus == "false")
                {
                    return;
                }

                auto mosq_state = m_externalBrokerObj->m_mqttCommunicationObj->mosq_state;
                if (mosq_state == 0)
                {
                    logg.str("");
                     logg << "CommunicatorManager::MonitorDAQStatus Data acquisiton is suspended because of mqtt disconnection";
                     m_exceptionLoggerObj->LogInfo( logg.str() );
                    return;
                }
                
                if (elapsed.count() > 120000)
                {
                    if (m_mqttRequestResponseObj->daqstatus == "true")
                    {
                        sleep(2);
                        DisableDAQ (m_externalBrokerObj->m_mqttCommunicationObj->m_mosq);
                         logg.str("");
                         logg << "CommunicatorManager::MonitorDAQStatus Data acquisiton is suspended ";
                         m_exceptionLoggerObj->LogInfo( logg.str() );
                    }
                    return;
                }
                sleep(1);
                return MonitorDAQStatus(start_time);
            }
        }	
    }
    catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "CommunicatorManager::MonitorDAQStatus  GatewayID : "  << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "CommunicatorManager::MonitorDAQStatus  GatewayID : "  << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

void CommunicatorManager::addOrUpdateEntry(nlohmann::json& jsonData, const std::string& key, const nlohmann::json& entry) 
{
    jsonData[key]["DAQ_M"] = entry["DAQ_M"];
    jsonData[key]["Hz"] = entry["Hz"];
    jsonData[key]["G"] = entry["G"];
    jsonData[key]["DAQ_P"] = entry["DAQ_P"];
    jsonData[key]["Samp_P"] = entry["Samp_P"];
    jsonData[key]["Filter"] = entry["Filter"];
    jsonData[key]["Order"] = entry["Order"];
    jsonData[key]["cutoff1"] = entry["cutoff1"];
    jsonData[key]["cutoff2"] = entry["cutoff2"];
    jsonData[key]["ms"] = entry["ms"];

    std::string fileName = UC6_BROADSENS_PATH;
    WriteConfiguration(fileName, jsonData);
    
}

bool CommunicatorManager::wait_until_response(int timeout_ms)
{
    int timeout = timeout_ms;
    while(timeout-- && (m_mqttRequestResponseObj->req_response_sync_mqttfft == "false"))
    {
        sleep(1);
    }
    if(timeout == 0)
    {
        return 0;
    }
    return 1;
}


int CommunicatorManager::SendMQTTControlCommands(void)
{
	std::stringstream logg;
	try
	{
	    m_mqttRequestResponseObj = m_mqttRequestResponseObj->GetInstance();
	    if( m_externalBrokerObj )
		{
	            m_mqttRequestResponseObj->sensors_list_updated = "false";    
	            m_mqttRequestResponseObj->sensors_list_times = 0;
				//Send Sensors List first
	            m_mqttRequestResponseObj->no_of_sns_info = 0;
	                          
	            auto now = std::chrono::steady_clock::now();
	            m_mqttRequestResponseObj->mtx.try_lock_until(now + std::chrono::seconds(20));
	            
		    SendSensorDataList();
	            wait_for_variable_to_update(m_mqttRequestResponseObj->sensors_list_updated);
	            m_mqttRequestResponseObj->sensors_list_updated = "false";
	            m_mqttRequestResponseObj->mtx.unlock();
	            m_mqttRequestResponseObj->no_of_sns_info++;
	
	            logg.str("");
	            logg << "CommunicatorManager::SendMQTTControlCommands Sensors List is sent";
	            m_exceptionLoggerObj->LogInfo( logg.str() );
		    //Initialize all the data
	            
	            wait_for_variable_to_update(m_mqttRequestResponseObj->req_response_sync);
	            m_mqttRequestResponseObj->req_response_sync = "false";
	            
	            start_time = std::chrono::steady_clock::now();            
	            int status = UpdateSensorsList (start_time);
	            
	            if (status == -2)
	            {
	                return -2;
	            }
	            
	            start_time = std::chrono::steady_clock::now();
	            status = EnableDAQ (start_time);
	            
	            if (status == -2)
	            {
	                return -2;
		    } 
	            if(m_mqttRequestResponseObj->fft_request_type != "last_request_iteration_last_value" )
	            {
	                std::string readFilepath = "/opt/IoT_Gateway/Common/Broadsens.7z" ;
	                std::string removeZipFile = "rm -r " + readFilepath;
	                std::stringstream logg;
	                if ( system( removeZipFile.c_str() ) == 0 )
	                {
	                    logg.str( std::string() );
	                    logg << "CloudCommunicationWrapper::UploadBlobStorageInMultipleBlocks ,  Message : Broadsens zip file Removed Successfully ";
	                    self->m_exceptionLoggerObj->LogInfo( logg.str() );
	                }
	                
	                std::string filepath = "/opt/IoT_Gateway/Common/Broadsens";
	                removeZipFile = "rm -r " + filepath;
	                //changed
	                if ( system( removeZipFile.c_str() ) == 0 )
	                {
	                    logg.str( std::string() );
	                    logg << "CloudCommunicationWrapper::UploadBlobStorageInMultipleBlocks,  Message : Broadsens folder Removed Successfully ";
	                    self->m_exceptionLoggerObj->LogInfo( logg.str() );
	                }
	                
	                //filepath = "/opt/IoT_Gateway/Common/Broadsens";
	                std::string createcmd = "mkdir " + filepath; 
	                system( createcmd.c_str() );
	                
	                std::string iteratorCheckFile = "/opt/IoT_Gateway/Common/iteration_check.json";
	                removeZipFile = "rm -r " + iteratorCheckFile; 
	                system( removeZipFile.c_str() );
	            }
		}
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "MQTT_Broadsens::CommunicatorManager::SendMQTTControlCommands()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "MQTT_Broadsens::CommunicatorManager::SendMQTTControlCommands()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

void CommunicatorManager::reset_handler()
{
    std::stringstream logg;
    try
	{
        sleep(4);
        auto start {std::chrono::steady_clock::now()};
        
        sleep(1);
        std::chrono::duration<double, std::milli> elapsed {std::chrono::steady_clock::now() - start};
        
        while (elapsed.count() < 80000)
        { 
            bool status = m_externalBrokerObj->m_mqttCommunicationObj->SensorsDataReception;
            //
            while(c2d_task == "true");
            sleep(1);
            
            if(elapsed.count() > 60000 && status == false)
            {       
                ResetBroadsensGateway();	
                break;
            }
            elapsed = std::chrono::steady_clock::now() - start;
        }
        return;
    }
    catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "CommunicatorManager::reset_handler()  GatewayID : " <<  ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "CommunicatorManager::reset_handler()  GatewayID : " << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
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
		
		m_mqttRequestResponseObj = m_mqttRequestResponseObj->GetInstance();
		if( m_mqttRequestResponseObj )
		{
			logg.str("");
			logg << "MQTT_Broadsens::CommunicatorManager::InitCommunicatorManager()  GatewayID : " << m_gatewayId << ",  Message : MQTTRequestResponseManager object created Successfully.";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			
			m_mqttRequestResponseObj->RegisterCB( std::bind( &CommunicatorManager::PropertiesReceiver, this, std::placeholders::_1 ) );
			m_mqttRequestResponseObj->SetGatewayId( m_gatewayId);
		}
		else
		{
			logg.str("");
			logg << "MQTT_Broadsens::CommunicatorManager::InitCommunicatorManager()  GatewayID : " << m_gatewayId << ",  Message : MQTTRequestResponseManager object creation Failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
        
        m_externalBrokerObj = new ExternalBrokerCommunicationManager();
		if( m_externalBrokerObj )
		{
			logg << "MQTT_Broadsens::CommunicatorManager::InitCommunicatorManager()  GatewayID : " << m_gatewayId << ",  Message : LocalBrokerCommunicationManager object created Successfully.";
			m_exceptionLoggerObj->LogInfo( logg.str() );

			//Callback for publishing to external broker
			RegisterCB( std::bind( &CommunicatorManager::DataPublisher, this, std::placeholders::_1, std::placeholders::_2) );
			
			//Callback for response
			m_externalBrokerObj->RegisterCB( std::bind( &CommunicatorManager::ReceiveSubscribedData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) );
			
            nlohmann::json jsonData;
            
            std::string uc6_broadsens = UC6_BROADSENS_PATH;
            std::string createcmd = "touch " + uc6_broadsens; 
            system( createcmd.c_str() );
            
            //Read
            m_DataPointsJson = ReadAndSetConfiguration( UC6_BROADSENS_PATH );
            
            if(m_DataPointsJson.empty())
            {
                
                //Adding 0_6000_A entry
                nlohmann::json newEntry = {
                    {"DAQ_M", 4},
                    {"Hz", 12800},
                    {"G", 16},
                    {"DAQ_P", 16384},
                    {"Samp_P", 16384},
                    {"Filter", 0},
                    {"Order", 0},
                    {"cutoff1", 0},
                    {"cutoff2", 6000},
                    {"ms", "Acceleration"}
                };

                std::string key = "0_6000_A";  // Replace with the desired key
                addOrUpdateEntry(jsonData, key, newEntry);
                newEntry.clear();
                
                //Adding 0_6000_V entry
                newEntry = {
                    {"DAQ_M", 4},
                    {"Hz", 12800},
                    {"G", 16},
                    {"DAQ_P", 16384},
                    {"Samp_P", 16384},
                    {"Filter", 0},
                    {"Order", 0},
                    {"cutoff1", 0},
                    {"cutoff2", 6000},
                    {"ms", "Velocity"}
                };

                key = "0_6000_V";  // Replace with the desired key
                addOrUpdateEntry(jsonData, key, newEntry);
                newEntry.clear();
                
                //Adding 2_1000_V entry
                newEntry = {
                    {"DAQ_M", 4},
                    {"Hz", 3200},
                    {"G", 16},
                    {"DAQ_P", 16384},
                    {"Samp_P", 16384},
                    {"Filter", 0},
                    {"Order", 0},
                    {"cutoff1", 2},
                    {"cutoff2", 1000},
                    {"ms", "Velocity"}
                };

                key = "2_1000_V";  // Replace with the desired key
                addOrUpdateEntry(jsonData, key, newEntry);
                newEntry.clear();
                //Adding 0_120_V entry
                
                newEntry = {
                    {"DAQ_M", 4},
                    {"Hz", 400},
                    {"G", 16},
                    {"DAQ_P", 16384},
                    {"Samp_P", 16384},
                    {"Filter", 0},
                    {"Order", 0},
                    {"cutoff1", 0},
                    {"cutoff2", 120},
                    {"ms", "Velocity"}
                };

                key = "0_120_V";  // Replace with the desired key
                addOrUpdateEntry(jsonData, key, newEntry);
                newEntry.clear();
                
                //Adding 500_6000_A entry
                newEntry = {
                    {"DAQ_M", 4},
                    {"Hz", 12800},
                    {"G", 16},
                    {"DAQ_P", 16384},
                    {"Samp_P", 16384},
                    {"Filter", 3},
                    {"Order", 4},
                    {"cutoff1", 500},
                    {"cutoff2", 6000},
                    {"ms", "Acceleration"}
                };

                key = "500_6000_A";  // Replace with the desired key
                addOrUpdateEntry(jsonData, key, newEntry);
                newEntry.clear();    
            }
            m_mqttRequestResponseObj = m_mqttRequestResponseObj->GetInstance();
            
            std::string  m_deviceFileName = "/opt/IoT_Gateway/GatewayAgent/config/MQTTBroadsensClient/register_devices.json";
			nlohmann::json  m_devicesRegisterJson = ReadAndSetConfiguration( m_deviceFileName );
			for( auto& x : m_devicesRegisterJson["assets"].items() )
			{
				m_mqttRequestResponseObj->device_id = x.key();
			}

	    nlohmann::json sleep_interval_check = ReadAndSetConfiguration( UC6_SLEEP_INTERVAL_PATH );
            if(sleep_interval_check.empty() == 0)
            {
                logg.str( std::string() );
                logg << "CommunicatorManager::InitCommunicatorManager Entered ";
                m_exceptionLoggerObj->LogInfo( logg.str() ); 
                
                
                auto time_check = std::chrono::system_clock::now();
                time_check.time_since_epoch();
                    
                // Convert system_clock to long long
                auto TimeLL = std::chrono::duration_cast<std::chrono::milliseconds>(
                    time_check.time_since_epoch()
                ).count();
                
                unsigned long long now = TimeLL;
                    
                unsigned long long sleep_start = sleep_interval_check["sleep_start"];
                unsigned long long sleep_interval = sleep_interval_check["sleep_interval"];
                auto duration_after_sleep = (sleep_interval*1000 - (TimeLL - sleep_start));
                unsigned long long sleep_end = sleep_interval_check["sleep_end"];
                
                nlohmann::json sleepjsonData;
                std::string sleep_fileName = UC6_SLEEP_INTERVAL_PATH;
                sleepjsonData["sleep_start"] = TimeLL;
                sleepjsonData["sleep_interval"] = duration_after_sleep/1000;
                
                WriteConfiguration(sleep_fileName, sleepjsonData);
                
                if(sleep_start > sleep_end)
                {
                    logg.str( std::string() );
                    logg << "CommunicatorManager::InitCommunicatorManager Time duration since the device is turned off (in secs): " << duration_after_sleep/1000; 
                    m_exceptionLoggerObj->LogInfo( logg.str() ); 
                    sleep(duration_after_sleep/1000);
                }
                
                std::string removeZipFile = "rm -r " + sleep_fileName;
                if ( system( removeZipFile.c_str() ) == 0 )
                {
                    logg.str( std::string() );
                    logg << "CommunicatorManager::InitCommunicatorManager Message : Sleep folder Removed Successfully ";
                    m_exceptionLoggerObj->LogInfo( logg.str() );
                }
            }
            
            std::string sleep_fileName = UC6_SLEEP_INTERVAL_PATH;
            
            auto start = std::chrono::system_clock::now();
            
            // Convert system_clock to long long
            auto currentTimeLL = std::chrono::duration_cast<std::chrono::milliseconds>(
                start.time_since_epoch()
            ).count();
                        
            nlohmann::json sleepjsonData;
            sleepjsonData["sleep_start"] = currentTimeLL;
            
            WriteConfiguration(sleep_fileName, sleepjsonData);
            
            std::string createfilecmd = "touch " + sleep_fileName; 
            system( createfilecmd.c_str() );    
                
			m_ThreadRunning = 1;
			m_threadObj = std::thread([this](){
			while( m_ThreadRunning )
			{
                //start epoch
				std::stringstream logg;
				thread_sleeping = false;
                m_externalBrokerObj->m_mqttCommunicationObj->SensorsDataReception = false;
            
                // Please Use WriteConfiguration

                
                if(!m_externalBrokerObj->m_mqttCommunicationObj->mosq_state)
				{
                    sleep(30);
					m_externalBrokerObj->m_mqttCommunicationObj->Mosquitto_reconnect();
                    continue;
				}
				
                while(m_externalBrokerObj->m_mqttCommunicationObj->mosq_state == 0);
                
                int status = 0;
				if(m_externalBrokerObj->m_mqttCommunicationObj->mosq_state)
				{
					status = SendMQTTControlCommands();
				}
                
				thread_sleeping = true;
                //end time
                auto end = std::chrono::system_clock::now();
                end.time_since_epoch();
                    
                // Convert system_clock to long long
                auto endTimeLL = std::chrono::duration_cast<std::chrono::milliseconds>(
                    end.time_since_epoch()
                ).count();
                                    
                nlohmann::json sleepjsonData;
                if(status == -1)
                {
                    logg.str( std::string() );
                    logg << "MQTT_Broadsens::CommunicatorManager::InitCommunicatorManager()  Message : Invalid Sensors List or UC_6 Broadsens List.";
                    m_exceptionLoggerObj->LogError( logg.str() );
                }
                
                if(status == -2)
                {
                    logg.str( std::string() );
                    logg << "MQTT_Broadsens::CommunicatorManager::InitCommunicatorManager()  Message : MQTT Communication is lost.";
                    m_exceptionLoggerObj->LogError( logg.str() );
                }
                
                if (first_iteration != no_of_sensors && c2d_task_thrown == "false")
                {
                    logg.str( std::string() );
                    logg << "CommunicatorManager::InitCommunicatorManager Message : First iteration executed"; 
                    logg << "c2d_task_thrown : " << c2d_task_thrown;
                    m_exceptionLoggerObj->LogInfo( logg.str() ); 
                    
                    sleep_time=(no_of_sensors == 0) ? DEFAULT_SLEEP_TIME : no_of_sensors * MIN_SLEEP_TIME;
                }
                else if(no_of_sensors == 0)
                {
                    sleep_time = DEFAULT_SLEEP_TIME;
                }
                first_iteration = no_of_sensors;
                no_of_sensors = 0;
                
                //Add the sleep implementation from phase 1 to phase 2
                
                nlohmann::json sleep_interval_content = ReadAndSetConfiguration( UC6_SLEEP_INTERVAL_PATH );
            
                logg.str( std::string() );
                logg << "CommunicatorManager::InitCommunicatorManager Message : sleep value : " << sleep_time; 
                m_exceptionLoggerObj->LogInfo( logg.str() ); 

                unsigned long long start_long = sleep_interval_content["sleep_start"];
                
                auto startTime = std::chrono::system_clock::time_point(
                                std::chrono::milliseconds(start_long)
                                );
                
                auto duration = end - startTime;
                
                // Convert duration to seconds
                unsigned long long duration_seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
                
                logg.str( std::string() );
                logg << "CommunicatorManager::InitCommunicatorManager Message : sleep duration : " << sleep_time/1000 - duration_seconds; 
                m_exceptionLoggerObj->LogInfo( logg.str() ); 
                
                
                auto start = std::chrono::system_clock::now();
            
                // Convert system_clock to long long
                auto currentTimeLL = std::chrono::duration_cast<std::chrono::milliseconds>(
                    start.time_since_epoch()
                ).count();
                    
                std::string sleep_fileName = UC6_SLEEP_INTERVAL_PATH;
                sleepjsonData["sleep_start"] = currentTimeLL;
                sleepjsonData["sleep_interval"] = sleep_time/1000 - duration_seconds;
                sleepjsonData["sleep_end"] = endTimeLL;                
                
                WriteConfiguration(sleep_fileName, sleepjsonData);
                
                if(duration_seconds < sleep_time/1000)
                {
                        sleep(sleep_time/1000 - duration_seconds);
                }
                else
                {
                    continue;
                }
            
                std::string removeZipFile = "rm -r " + sleep_fileName;
                if ( system( removeZipFile.c_str() ) == 0 )
                {
                    logg.str( std::string() );
                    logg << "Message : Sleep folder Removed Successfully ";
                    m_exceptionLoggerObj->LogInfo( logg.str() );
                }
                
                std::string createfilecmd = "touch " + sleep_fileName; 
                system( createfilecmd.c_str() );                                    
                WriteConfiguration(sleep_fileName, sleepjsonData);
			}
			});	
            
            m_thread_2_running = true;
            m_threadObj2 = std::thread([this](){
			while(m_thread_2_running)
			{
                std::stringstream logg;
				if(!thread_sleeping)
				{
					
					reset_handler();
                    sleep(1000);
				}
			}
            }); 
		}
		else
		{
			logg.str( std::string() );
			logg << "MQTT_Broadsens::CommunicatorManager::InitCommunicatorManager()  GatewayID : " << m_gatewayId << ",  Message : LocalBrokerCommunicationManager object creation Failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
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

void CommunicatorManager::ResetBroadsensGateway()
{
    std::stringstream logg;
	try
	{
        //m_mqttRequestResponseObj->mtx.lock();
        auto now = std::chrono::steady_clock::now();
        m_mqttRequestResponseObj->mtx.try_lock_until(now + std::chrono::seconds(20));

        c2d_task = "true";
        std::string passwd_config = "\"fftbrsns\"";
        std::string passwd = "{\"passwd\":" + passwd_config;
        std::string reset_config = "true";
        std::string reset = "\"reset\":" + reset_config;

        //RESET Broadsens Gateway
        
        std::string control_topic = "control/reset";
        std::string daq_payload = passwd + "," + reset + "}";	

        if(m_externalBrokerObj->m_mqttCommunicationObj->mosq_state)
        {	

            nlohmann::json daq_payload_json = nlohmann::json::parse(daq_payload);
            int rc = mosquitto_publish(m_externalBrokerObj->m_mqttCommunicationObj->m_mosq, NULL, control_topic.c_str(), daq_payload.length(), daq_payload.c_str(), 1, false);

            if( rc == MOSQ_ERR_SUCCESS )
            {
                //Along with the message
                std::string message = "Broadsens is reset successfully";
                std::string event = "Abnormality in the Broadsens";
                SendNotificationToCloud(message, event);
            }
        }
        c2d_task = "false";
        m_mqttRequestResponseObj->mtx.unlock();
    }
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "CommunicatorManager::ResetBroadsensGateway  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "CommunicatorManager::ResetBroadsensGateway  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief SendNotificationToCloud	:	This method will send the restart app notification to cloud.
 * 
 */ 
void CommunicatorManager::SendNotificationToCloud(std::string message, std::string event)
{
	std::stringstream logg;
	try
	{
        std::string app_name = "MQTTBroadsensClient";
        long appId = GetProcessIdByName( app_name );
		nlohmann::json connectionFailJson;
		std::string publishNotification = PUBLISH_PREFIX + m_gatewayId + COMMUNICATORAPP_RESPONSE_PREFIX;
		connectionFailJson[TYPE] = "notification";
		connectionFailJson[MESSAGE] = message;
		connectionFailJson[EVENT] = event;
        std::string epoch = GetTimeStampMilli();
        connectionFailJson[TIMESTAMP] = std::stod(epoch);
		connectionFailJson[COMMAND_INFO][COMMAND_TYPE] = "response";
        connectionFailJson["Moni"] = publishNotification;
        
        if(appId > 0)
        {
            connectionFailJson[COMMAND_INFO][APP_ID] = appId;
            connectionFailJson[COMMAND_INFO][APP_NAME] = app_name;
        }
		//std::string publishMsg = connectionFailJson.dump();
        PropertiesReceiver(connectionFailJson);
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
 * @brief RegisterCB	:	This method will receive the data from GatewayAgent, RuleEngine, DeviceApp, CachingAgent
 * 									and call the execute method.
 * 
 */ 

void CommunicatorManager::RegisterCB( std::function<void(nlohmann::json, std::string)> cb )
{
	m_dataCB_bs = cb;
}

void CommunicatorManager::RegisterDACB( std::function<void(nlohmann::json)> cb )
{
	m_propertiesCB = cb;
}

/**
 * @brief ReceiveSubscribedData		:	This method will receive the data, and punlish topic. It will send the data
 * 										to the received topic.
 * 
 */ 
void CommunicatorManager::DataPublisher( nlohmann::json dataJson, std::string publishTopic )
{
	std::string data = dataJson.dump();	
	m_externalBrokerObj->PublishData( data, publishTopic );
}


void CommunicatorManager::PropertiesReceiver( nlohmann::json jsonObject )
{
	m_propertiesCB( jsonObject );
}
