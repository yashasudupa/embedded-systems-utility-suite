#include "MQTTRequestResponseManager.h"

MQTTRequestResponseManager* MQTTRequestResponseManager::m_MQTTRqstRspInstance = nullptr;

MQTTRequestResponseManager* MQTTRequestResponseManager::GetInstance()
{
	 if (!m_MQTTRqstRspInstance)
	 {
		m_MQTTRqstRspInstance = new MQTTRequestResponseManager();
	 }
	 return m_MQTTRqstRspInstance;
}


MQTTRequestResponseManager::MQTTRequestResponseManager():
	m_cloudConnectionStatus( false )
{
	m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
	MaintainPersistency();
}

MQTTRequestResponseManager::~MQTTRequestResponseManager()
{
	
}
double Msb_bit_result( uint8_t high_byte ,uint8_t low_byte, double scale)
{
	uint16_t tmp = ((high_byte<<8)|low_byte);
	
	double Acceleration_value = 0;
		if((tmp&0x8000)==0x8000) /*check sign bit */
	
		{
					tmp=~tmp+1; /* sign bit is 1, negative number */
					Acceleration_value = (-tmp)*scale;
		}
		else
		{
					Acceleration_value=tmp*scale; /* sign bit is 0, positive number */
		}
				//std::cout << " ************** in function def Acceleration_value "<<Acceleration_value<< std::endl;
				return Acceleration_value;
				tmp=0;

}
std::string MQTTRequestResponseManager::SensorType( int sns_type)
{
	/*
	 * SVT100_T=0x00, SVT200_V=0x01, SVT100_A_SVT200_A=0x10, 
			SVT101_A_SVT201_A=0x11, SVT100_T_TS=0x20, SVT200_V_VS=0x21, 
			SVT100_A_SVT200_A_0x10=0x30, SVT100_A_SVT200_A_0x11=0x31}
			 */
	std::string SensorType;
	
	if (sns_type == SVT100_T)
	{
		SensorType = "SVT100_T";
	}
	
	if (sns_type == SVT200_V)
	{
		SensorType = "SVT200_V";
	}
	
	if (sns_type == SVT100_A_SVT200_A)
	{
		SensorType = "SVT100_A_SVT200_A";
	}
	
	if (sns_type == SVT101_A_SVT201_A)
	{
		SensorType = "SVT101_A_SVT201_A";
	}
	
	if (sns_type == SVT100_T_TS)
	{
		SensorType = "SVT100_T_TS";
	}
	
	if (sns_type == SVT200_V_VS)
	{
		SensorType = "SVT200_V_VS";
	}
	
	if (sns_type == SVT100_A_SVT200_A_0x10)
	{
		SensorType = "SVT100_A_SVT200_A_0x10";
	}
	
	if (sns_type == SVT100_A_SVT200_A_0x11)
	{
		SensorType = "SVT100_A_SVT200_A_0x11";
	}
	return SensorType;
}

nlohmann::json MQTTRequestResponseManager::BroadsensMessagesProcess( BROADSENS_MQTT_MESSAGE &brsns_msg, std::uint32_t &payloadLen)
{
	
	//std::lock_guard lk(mtx);
	nlohmann::json sns_data_json;
	
	std::string brsns_msg_cplus_json = brsns_msg.mqtt_data;
	
	//nlohmann::json parsed_brsns_msg = nlohmann::json::parse(brsns_msg_cplus_json);
	
	std::cout << "BroadsensMessagesProcess brsns_msg_cplus_json : " << brsns_msg_cplus_json << std::endl;
	
	//std::cout << "BroadsensMessagesProcess brsns_msg_json : " << parsed_brsns_msg << std::endl;
	
	if (nlohmann::json::accept(brsns_msg_cplus_json) || brsns_msg.mqtt_topic == )
	{
		nlohmann::json brsns_msg_json;
		
		std::lock_guard<std::mutex> lock(mtx);
	
		std::cout << "BroadsensMessagesProcess brsns_msg_json : after accepting " << std::endl;
		
		std::string data =  brsns_msg.mqtt_data;
		brsns_msg_json["payload"] = nlohmann::json::parse(data);
		
		//mqd_t mqd = mq_open ("/OpenCSF_MQ", O_EXCL | O_WRONLY,  0600, NULL);

		/*
		if (mqd == -1)
		{
			perror ("mq_open");
			exit (1);
		}
		*/
	
		//GlobalOperations *GlobalOperationsObj = new GlobalOperations();
		
		if (brsns_msg.mqtt_topic == BROADSENS_MQTTfft)
		{
		
			//TODO Cast brsns_msg.mqtt_data to a json variable brsns_msg_json
			
			if (daqstatus = false && fft_msg.cutoff == 0 && fft_msg.cutofftwo == 6000)
			{
				if(brsns_msg_json.empty())
				{
					std::string msg = "not received";
					//mq_send (mqd, &msg.c_str()[0], sizeof(msg), 10);
					buffer = msg;
					cv.notify_one();
				}
				if(brsns_msg_json["payload"]["measurements"] == "Acceleration")
				{
					//TODO 
					std::string msg = "received";
					//mq_send (mqd, &msg.c_str()[0], sizeof(msg), 10);
					
					buffer = msg;
					cv.notify_one();
					
					sns_data_json["SensorID"] = brsns_msg_json["sensorId"];
					sns_data_json["xAxis"]=brsns_msg_json["xAxis"];
					sns_data_json["yAxis"]=brsns_msg_json["yAxis"];
					sns_data_json["zAxis"]=brsns_msg_json["zAxis"];
					
				}
				//GlobalOperationsObj->cv.notify_all();
			}
			
			if (daqstatus = false && fft_msg.cutoff == 2 && fft_msg.cutofftwo == 1000)
			{
				if(brsns_msg_json.empty())
				{
					std::string msg = "not received";
					//mq_send (mqd, &msg.c_str()[0], sizeof(msg), 10);
					buffer = msg;
					cv.notify_one();
				}
				
				if(brsns_msg_json["payload"]["measurements"] == "Velocity")
				{
					std::string msg = "received";
					//mq_send (mqd, &msg.c_str()[0], sizeof(msg), 10);
					
					buffer = msg;
					cv.notify_one();
					//TODO 
					
					sns_data_json["SensorID"] = brsns_msg_json["sensorId"];
					sns_data_json["rmsx"]=brsns_msg_json["rmsx"];
					sns_data_json["rmsy"]=brsns_msg_json["rmsy"];
					sns_data_json["rmsz"]=brsns_msg_json["rmsz"];
			
				}
				//GlobalOperationsObj->cv.notify_all();
			}
			
			if (daqstatus = false && fft_msg.cutoff == 0 && fft_msg.cutofftwo == 120)
			{
				if(brsns_msg_json.empty())
				{
					std::string msg = "not received";
					//mq_send (mqd, &msg.c_str()[0], sizeof(msg), 10);
					buffer = msg;
					cv.notify_one();
				}
				if(brsns_msg_json["payload"]["measurements"] == "Velocity")
				{
					//TODO 
					std::string msg = "received";
					//mq_send (mqd, &msg.c_str()[0], sizeof(msg), 10);
					buffer = msg;
					cv.notify_one();
					
					sns_data_json["SensorID"] = brsns_msg_json["sensorId"];
					sns_data_json["fftFreq"]=brsns_msg_json["fftFreq"];
					sns_data_json["fftxAxis"]=brsns_msg_json["fftxAxis"];
					sns_data_json["fftyAxis"]=brsns_msg_json["fftyAxis"];
					sns_data_json["fftzAxis"]=brsns_msg_json["fftzAxis"];
				}
				//GlobalOperationsObj->cv.notify_all();
			}

			if (daqstatus = false && fft_msg.cutoff == 500 && fft_msg.cutofftwo == 6000)
			{
				if(brsns_msg_json.empty())
				{
					std::string msg = "not received";
					//mq_send (mqd, &msg.c_str()[0], sizeof(msg), 10);
					buffer = msg;
					cv.notify_one();
				}
				if(brsns_msg_json["payload"]["measurements"] == "Velocity")
				{
					//TODO 
					std::string msg = "received";
					//mq_send (mqd, &msg.c_str()[0], sizeof(msg), 10);
					
					buffer = msg;
					cv.notify_one();
					
					sns_data_json["SensorID"] = brsns_msg_json["sensorId"];
					sns_data_json["rmsx"]=brsns_msg_json["rmsx"];
					sns_data_json["rmsy"]=brsns_msg_json["rmsy"];
					sns_data_json["rmsz"]=brsns_msg_json["rmsz"];
			
				}
				//GlobalOperationsObj->cv.notify_all();
			}
		}
		//BROADSENS_DAQSTATUS
		else if(brsns_msg.mqtt_topic == BROADSENS_DAQSTATUS)
		{	
			const char *ptr_to_msg = brsns_msg.mqtt_data;
			if(brsns_msg_json.empty())
			{
				std::string msg = "not received";
				//mq_send (mqd, &msg.c_str()[0], sizeof(msg), 10);
				buffer = msg;
				cv.notify_one();
			}
			else
			{
				std::string msg = "received";
				//mq_send (mqd, &msg.c_str()[0], sizeof(msg), 10);
				buffer = msg;
				
				std::cout << "Broadsens before notifying " << std::endl;
				
				cv.notify_one();
				
				std::cout << "Broadsens after notifying, msg received " << std::endl;
				
				daqstatus = *ptr_to_msg;
				//mq_send (mqd, ptr_to_msg, sizeof(daqstatus), 10);
				buffer = std::to_string(daqstatus);				
				cv.notify_one();
				std::cout << "Broadsens after notifying, daqstatus " << daqstatus << std::endl;
			}
			//GlobalOperationsObj->cv.notify_all();
		}
		
		//BROADSENS_CONTROL_FFT
		else if(brsns_msg.mqtt_topic == BROADSENS_CONTROL_FFT)
		{
			//TODO
			if(brsns_msg_json.empty())
			{
				std::string msg = "not received";
				//mq_send (mqd, &msg.c_str()[0], sizeof(msg), 10);
				buffer = msg;
				cv.notify_one();
			}
			else
			{
				std::string msg = "received";
				//mq_send (mqd, &msg.c_str()[0], sizeof(msg), 10);
				buffer = msg;
				cv.notify_one();
				
				fft_msg.cutoff = brsns_msg_json["cutoff"];
				fft_msg.cutofftwo = brsns_msg_json["cutoff2"];
				//Parse fftSensor, fftMeasure, filtertype, filterOrder
				//cutoff, cutoff2 and copy to the global variable 'fft_msg'.
			}
		}
		//BROADSENS_SNS_INFO
		else if(brsns_msg.mqtt_topic == BROADSENS_SNS_INFO_PREFIX)
		{
			//TODO
			if(brsns_msg_json.empty())
			{
				std::string msg = "not received";
				//mq_send (mqd, &msg.c_str()[0], sizeof(msg), 10);
				buffer = msg;
				cv.notify_one();
			}
			else
			{
				std::string msg = "received";
				buffer = msg;
				cv.notify_one();
				if(!brsns_msg_json["payload"]["SVTA"].empty())
				{
					for (auto item : brsns_msg_json["payload"]["SVTA"])
					{
						nlohmann::json value;
						
						std::string key = item["payload"]["SVTA"]["ID"];		
						
						value["name"] = item["payload"]["SVTA"]["name"];
						value["GroupNumber"] = item["payload"]["SVTA"]["grp"];
						sensor_idlist.emplace(key, value);
					}
				}
				
				if(!brsns_msg_json["payload"]["SVTV"].empty())
				{
					for (auto item : brsns_msg_json["payload"]["SVTV"])
					{
						nlohmann::json value;
						
						std::string key = item["payload"]["SVTV"]["ID"];		
						
						value["name"] = item["payload"]["SVTV"]["name"];
						value["GroupNumber"] = item["payload"]["SVTV"]["grp"];
						sensor_idlist.emplace(key, value);
					}
				}
				
				nlohmann::json sensor_idlist_json = sensor_idlist;
				WriteConfiguration(LOCAL_MQTT_SNS_FILE,sensor_idlist_json);
			}
		}
		
		//delete GlobalOperationsObj;
		return sns_data_json;
	}
	
	std::cout << "**************************************************** " << std::endl;
	std::cout << "MQTTRequestResponseManager::BroadsensMessagesProcess  topic : " <<  brsns_msg.mqtt_topic
	<< "data :" << brsns_msg.mqtt_data << std::endl;
	std::cout << "*********************************payloadLen******************* "<<payloadLen << std::endl;
	
    int mqtttopic=0;
    if(brsns_msg.mqtt_topic == BROADSENS_ACCE_PREFIX)
    {mqtttopic=1;
      std::cout << " topic :BROADSENS_ACCE_PREFIX " <<  std::endl;
    }
    else if (brsns_msg.mqtt_topic == BROADSENS_GTEMP_PREFIX)
    {mqtttopic=2;
            std::cout << " topic :BROADSENS_GTEMP_PREFIX " <<  std::endl;
    }
    else if (brsns_msg.mqtt_topic == BROADSENS_ATEMP_PREFIX)
    {mqtttopic=3;
      std::cout << " topic :BROADSENS_ATEMP_PREFIX " <<  std::endl;
    }
     else if (broadsens_msg_queue.front().mqtt_topic == BROADSENS_GW_INFO )
    {
	  mqtttopic=4;
      std::cout << " topic :BROADSENS_GW_INFO " <<  std::endl;
    }
     else if (broadsens_msg_queue.front().mqtt_topic == BROADSENS_PREFIX )
    {
		mqtttopic=5;
	}

	std::uint8_t *payload = (uint8_t *)&brsns_msg.mqtt_data;
	printf("MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand()  : data[0] %x\n", payload[0]);
	printf("MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand()  : data[1] %x\n", payload[1]);
	printf("MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand()  : data[2] %x\n", payload[2]);
	printf("MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand()  : data[3] %x\n", payload[3]);
	printf("MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand()  : data[4] %x\n", payload[4]);
	printf("MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand()  : data[5] %x\n", payload[5]);
	printf("MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand()  : data[6] %x\n", payload[6]);
	printf("MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand()  : data[7] %x\n", payload[7]);
		
	printf("MQTT_Broadsens::Subscribe topic : %s, payload : %s\n", brsns_msg.mqtt_topic, payload);
    nlohmann::json tst_data_json;
	double scale;
    double scalev;
    double scaleg;
	const char *ptr_to_msg = brsns_msg.mqtt_data;
	
	sns_data_json["SensorsType : "] = SensorType(ptr_to_msg[0]);
	std::cout << "**********sensor type is *********** " <<ptr_to_msg[0]<< std::endl;
	switch(ptr_to_msg[0])
	{ 
		case 0x00: 
		{
					scale =0.0078125;std::cout << "sensor type :0 \n" <<  std::endl;
					break;
		}
		case 0x01: 
		{           scale =0.000879;
					scalev=409.6;std::cout << "sensor type :01\n " <<  std::endl;
					scaleg=2367.135;
					break;  
		}
		case 0x10: 
		 {          scale =0.00024414;std::cout << "sensor type :10\n " <<  std::endl;
					break;
						
		 }
		case 0x11: 
		{           scale =0.000879;std::cout << "sensor type :11 \n" <<  std::endl;
					break;
		}       
		case 0x20:
		{           scale =0.00048828;std::cout << "sensor type :20 " <<  std::endl;
					break;
					
		}
		case 0x21: 
		{           scale =0.000879;std::cout << "sensor type :21" <<  std::endl;
					break;
		}
		case 0x30: 
		{          scale =0.00195318;std::cout << "sensor type :30 " <<  std::endl;
					break;
		}
		case 0x31: 
		{            scale =0.000879;std::cout << "sensor type :31 " <<  std::endl;
					 break;
		}
		default  :  
		{            std::cout << "**********sensor type is undefined************ " << std::endl;
					break;
					
		}
	}
    /*enum broadsens_msg_queue.front().mqtt_topic
        {  
           BROADSENS_ACCE_PREFIX=1,BROADSENS_GTEMP_PREFIX=2,BROADSENS_ATEMP_PREFIX=3,BROADSENS_SNS_INFO_PREFIX=4,
           BROADSENS_FFT_PREFIX=5,BROADSENS_GW_INFO=6,BROADSENS_PREFIX=7 };*/
	switch(mqtttopic)
    {        
       case 1:
       { 
           if(brsns_msg.mqtt_data)
		{
			 uint8_t high_byte = ptr_to_msg[1];
			uint8_t low_byte = ptr_to_msg[2];
			
			sns_data_json["SensorId : "] = std::to_string(high_byte<<8|low_byte);
            int count=0;
			
			
            
			
			for (int i=3; i<(payloadLen-3); i=i+6)
			{      
				for (int j=0; j<TOTAL_NO_OF_AXIS_WITH_COORDINATES; j=j+2)
				{   					
					uint8_t low_byte = ptr_to_msg[i+j];
					uint8_t high_byte = ptr_to_msg[i+j+1];
                    //double acceleration=Msb_bit_result(high_byte,low_byte,scale);
					tst_data_json["Acce_index"][j][count]= Msb_bit_result(high_byte,low_byte,scale);
                    
				}
                count++;
			}	
            {
             sns_data_json["X-axis acceleration :"]=  tst_data_json["Acce_index"][0];
             sns_data_json["Y-axis acceleration :"]=  tst_data_json["Acce_index"][2];
             sns_data_json["Z-axis acceleration :"] =  tst_data_json["Acce_index"][4];
             
            }
			
			{
				sns_data_json["DAQ mode : "] = ptr_to_msg[payloadLen-2];
				sns_data_json["DAQ rate : "] = ptr_to_msg[payloadLen-1];
			}
			
			std::cout << "**************************************************** " << std::endl;
			std::cout << "MQTTRequestResponseManager::BroadsensMessagesProcess  topic : " << broadsens_msg_queue.front().mqtt_topic  
			<< "sns_data_json :" << sns_data_json << std::endl;
			std::cout << "**************************************************** " << std::endl;
	     }
		
	     break;
			
			
		}
       
       case 2:
       {
          if(brsns_msg.mqtt_data)
		{
		//	const char *ptr_to_msg = brsns_msg.mqtt_data;
		//	sns_data_json["SensorsType : "] = SensorType(ptr_to_msg[0]);
			
			{
				uint8_t high_byte = ptr_to_msg[1];
				uint8_t low_byte = ptr_to_msg[2];
			
				uint16_t snsid = (high_byte<<8 | low_byte);
				sns_data_json["SensorId : "] = snsid;
			}
			
			{
				uint8_t high_byte = ptr_to_msg[3];
				uint8_t low_byte = ptr_to_msg[4];
				
				uint16_t value = (high_byte<<8|low_byte) *0.0078125;

				
				sns_data_json["TemperatureValue : "] = Msb_bit_result(high_byte,low_byte,scale);
			}
			
			sns_data_json["GrpNumber : "] = std::to_string(ptr_to_msg[5]);
			
			sns_data_json["EndingByte : "] = std::to_string(ptr_to_msg[6]) + std::to_string(ptr_to_msg[7]);
		}
		
		 std::cout << "**************************************************** " << std::endl;
		 std::cout << "MQTTRequestResponseManager::BroadsensMessagesProcess  topic : " << broadsens_msg_queue.front().mqtt_topic 
		  << "sns_data_json :" << sns_data_json << std::endl;
		 std::cout << "**************************************************** " << std::endl;
         break;
       }
       
       case 3:
       {
          if(brsns_msg.mqtt_data)
		{
		
			
			{
				uint8_t high_byte = ptr_to_msg[3];
				uint8_t low_byte = ptr_to_msg[4];
			
				uint16_t snsid = (high_byte<<8 | low_byte);
				sns_data_json["SensorId : "] = snsid;
			
			}
			
			{
			
				uint8_t high_byte = ptr_to_msg[5];
				uint8_t low_byte = ptr_to_msg[6];
				
				sns_data_json["TemperatureValue : "] = Msb_bit_result(high_byte,low_byte,scale);
			}
			
			
			sns_data_json["EndingByte : "] = std::to_string(ptr_to_msg[5]) + std::to_string(ptr_to_msg[6]);
			
			std::cout << "**************************************************** " << std::endl;
			std::cout << "MQTTRequestResponseManager::BroadsensMessagesProcess  topic : " << broadsens_msg_queue.front().mqtt_topic 
			<< "sns_data_json :" << sns_data_json << std::endl;
			std::cout << "**************************************************** " << std::endl;
		}
        break; 
       }
       case 4:
       { 
         if(brsns_msg.mqtt_data)
		{
			{
				uint8_t high_byte = ptr_to_msg[1];
				uint8_t low_byte = ptr_to_msg[2];
			
				double snsid  = (high_byte<<8|low_byte);
				sns_data_json["SensorId : "] = snsid;
			}
			
			{
				uint8_t high_byte = ptr_to_msg[3];
				uint8_t low_byte = ptr_to_msg[4];
				
				//Yet to check
				//Please note that one needs to convert the value 
				//to negative number if the highest digit of D4 is 1 (two’s complement
				
				double value = Msb_bit_result(high_byte,low_byte,scale);
				sns_data_json["BatteryVoltage : "] = value+0.27;
			}
			
			
			sns_data_json["Mac address : "] = std::to_string(ptr_to_msg[5]) +
											  std::to_string(ptr_to_msg[6]) +
											  std::to_string(ptr_to_msg[7]) +
											  std::to_string(ptr_to_msg[8]) +
											  std::to_string(ptr_to_msg[9]) + 
											  std::to_string(ptr_to_msg[10]);
			
			sns_data_json["RSSI level in dBm : "] = "-" + std::to_string(ptr_to_msg[11]);
			
			sns_data_json["Version number : "] = std::to_string(ptr_to_msg[12]/10);
			
			sns_data_json["GrpNumber : "] = std::to_string(ptr_to_msg[13]);
			
			sns_data_json["General Vibration Temperature : "] = std::to_string((ptr_to_msg[14] << 8 ) | ptr_to_msg[15]);
			
			sns_data_json["Ending Byte : "] = std::to_string(ptr_to_msg[16])  + std::to_string(ptr_to_msg[17]);
			
			std::cout << "**************************************************** " << std::endl;
			std::cout << "MQTTRequestResponseManager::BroadsensMessagesProcess  topic : " << broadsens_msg_queue.front().mqtt_topic 
			<< "sns_data_json :" << sns_data_json << std::endl;
			std::cout << "**************************************************** " << std::endl;
		}
	
         break;
       }
       case 5: 
       {
        //sns_data_json["BROADSENS_FFT : "]=json.parse(brsns_msg.mqtt_data); 
         break;
       }
       case 6:
        {
            break;
        }
        case 7:
        {
            break;
        }
    }
	
			
	return sns_data_json;
}


//return structure
ResponseStruct MQTTRequestResponseManager::ExecuteCommand( char *payload, std::uint32_t payloadLen, char *topic)
{
	std::stringstream logg;
	ResponseStruct responseStructObj;
	try
	{
		
		logg.str("");
		
		std::uint8_t temp = *payload;
		printf("MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand()  : data[0] %x\n", payload[0]);
		printf("MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand()  : data[1] %x\n", payload[1]);
		printf("MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand()  : data[2] %x\n", payload[2]);
		printf("MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand()  : data[3] %x\n", payload[3]);
		printf("MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand()  : data[4] %x\n", payload[4]);
		printf("MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand()  : data[5] %x\n", payload[5]);
		printf("MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand()  : data[6] %x\n", payload[6]);
		printf("MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand()  : data[7] %x\n", payload[7]);
		
		printf("MQTT_Broadsens::Subscribe topic : %s, payload : %s\n", topic, payload);
	
		//printf("MQTT_Broadsens::MQTTCommunicationWrapper::setData payload : %s\n", msg);
		//callback
		
		BROADSENS_MQTT_MESSAGE broadsens_msg_buff;
		
		//Process the data from the queue
		
		broadsens_msg_buff.mqtt_data = new char [payloadLen];
		
		std::memcpy(broadsens_msg_buff.mqtt_data, payload, payloadLen);
		
		m_exceptionLoggerObj->LogInfo( logg.str() );
		
		//Queue the payload
		
		//std::strcpy(&broadsens_msg_buff.mqtt_data[0], ptr_to_payload);
		broadsens_msg_buff.mqtt_topic = topic;
		std::cout << "*********************payloadLen******************************* "<<payloadLen<< std::endl;
		
		broadsens_msg_queue.push_back(broadsens_msg_buff);
		std::cout << "**************************************************** " << std::endl;
		std::cout << "MQTTRequestResponseManager::ExecuteCommand broadsens_msg_buff.mqtt_data :" << 
		broadsens_msg_buff.mqtt_data << std::endl;
		std::cout << "**************************************************** " << std::endl;
		
		broadsens_msg_queue.push_back(broadsens_msg_buff);
		
		//Process the message
		nlohmann::json br_sns_msg_json;
		
		br_sns_msg_json = BroadsensMessagesProcess( broadsens_msg_queue.front(), payloadLen );
		
		std::cout << "**************************************************** " << std::endl;
		std::cout << "MQTTRequestResponseManager::ExecuteCommand  topic : " << broadsens_msg_buff.mqtt_topic 
		<< "br_sns_msg_json :" << br_sns_msg_json << std::endl;
		std::cout << "**************************************************** " << std::endl;
		
		//Format the payload
		
		nlohmann::json BrPayloadJson;
		std::string publishNotification = PUBLISH_PREFIX + m_gatewayId + BROADSENS_TO_CLOUD_PREFIX;
		BrPayloadJson[TYPE] = "BROADSENS_SENSOR_DATA";
		BrPayloadJson[MESSAGE] = br_sns_msg_json;
		BrPayloadJson[EVENT] = "Broadsens sensor data tranfer";
		BrPayloadJson[TIMESTAMP] = GetTimeStamp();
		BrPayloadJson[COMMAND_INFO][COMMAND_TYPE] = "live data";
		std::string publishMsg = BrPayloadJson.dump();
		
		
		std::cout << "**************************************************** " << std::endl;
		std::cout << "MQTTRequestResponseManager::ExecuteCommand  publishMsg : " << publishMsg << std::endl;
		std::cout << "**************************************************** " << std::endl;
		
		// Send to cloud
		if( m_dataCB )
		{
			m_dataCB( publishMsg, publishNotification );
			responseStructObj.status = SUCCESS;
			responseStructObj.responseMetaInformation = "Publish Request successfully.";
			//return responseStructObj;
		}
		
		//Remove the processed data from the queue
		//TODO 
		broadsens_msg_queue.erase(broadsens_msg_queue.begin());
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "MQTT_Broadsens::MQTTRequestResponseManager::ExecuteCommand  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
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
						logg << "MQTT_Broadsens::MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : 'appid' key not found in respected json. JSON : " << dataJson; 
						m_exceptionLoggerObj->LogError( logg.str() );
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'appid' key not found in respected json";
						return responseStructObj;
					}
					
					if( dataJson[COMMAND_INFO][APP_NAME].is_null() )
					{
						logg.str( std::string() );
						logg << "MQTT_Broadsens::MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : 'app_name' key not found in respected json. JSON : " << dataJson; 
						m_exceptionLoggerObj->LogError( logg.str() );
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'app_name' key not found in respected json";
						return responseStructObj;
					}
					logg.str( std::string() );
					logg << "MQTT_Broadsens::MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : Validate Register request Successfully. "; 
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
						logg << "MQTT_Broadsens::MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : 'appid' key not found in respected json. JSON : " << dataJson; 
						m_exceptionLoggerObj->LogError( logg.str() );
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = "'appid' key not found in respected json";
						return responseStructObj;
					}
					
					if( dataJson[COMMAND_INFO][APP_NAME].is_null() )
					{
						logg.str( std::string() );
						logg << "MQTT_Broadsens::MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : 'app_name' key not found in respected json. JSON : " << dataJson; 
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
					logg << "MQTT_Broadsens::MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : Respected Application Not registerd. AppName : " << dataJson[COMMAND_INFO][APP_NAME]; 
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
                        logg << "MQTT_Broadsens::MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : 'sub_job_id' key not found in respected json. JSON : " << dataJson; 
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
								logg << "MQTT_Broadsens::MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : 'app_name' key not found in respected json. JSON : " << dataJson; 
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
									logg << "MQTT_Broadsens::MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : 'devices' key not found in respected json. JSON : " << dataJson; 
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
									logg << "MQTT_Broadsens::MQTTRequestResponseManager::ValidateData()  GatewayID : " << m_gatewayId << ",  Message : Request Validate Successfully."; 
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
		logg << "MQTT_Broadsens::MQTTRequestResponseManager::RequestHandler  requestJson Rule Engine : " << requestJson ;
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
			logg << "MQTT_Broadsens::MQTTRequestResponseManager::MaintainPersistency  GatewayID : " << m_gatewayId << ",  Message : Received empty persistency json from file";
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