#include "ReadProperties.h"

ReadProperties::ReadProperties(std::string deviceId, std::string slaveId, std::string processName, nlohmann::json connectionJson)
    : m_deviceId(deviceId),
      m_ctx(nullptr),
      m_slave_Id(slaveId),
      m_blockNumber(0),
      m_totalNumberOfProperties(0),
      m_noOfRegInOneBlock(45),
      m_pollingFreq(5000),
      m_turboTimeout(5000),
      m_deviceMode(""),
      m_changeValueState(false),
      m_ThreadRunning(true),
      m_connectionStatus(COMMFAIL),
      m_connectionJson(connectionJson),
      m_connectionNotificationFlag(false),
      m_processName(processName),
      m_numberOfRequests(1),
      m_firstTimeAppStartFlag(true),
      m_lastAlertVal(0)
{
    // Initialize group structures with default values
    m_grpStructObj1 = new GROUP_STRUCT{5000, 5000, 10000, 60000};
    m_grpStructObj2 = new GROUP_STRUCT{5000, 5000, 10000, 60000};
    m_grpStructObj3 = new GROUP_STRUCT{5000, 5000, 10000, 60000};

    // Connect to external device
    ConnectToDevice();

    // Retrieve current process ID
    long pid = GetProcessIdByName(processName);

    // Set file paths and initial JSON structures
    m_configFilePath = "./config/" + m_processName + "/";
    m_commandJson[APP_NAME] = processName;
    m_commandJson[APP_ID] = pid;
    m_commandJson[COMMAND_TYPE] = RESPONSE;

    m_alertJsonArray[COMMAND_INFO] = m_commandJson;
    m_alertJsonArray[COMMAND_INFO][COMMAND_SCHEMA] = "alert";
    m_alertJsonArray[TYPE] = MULTI_ALERT;
    m_alertJsonArray[DEVICE_ID] = m_deviceId;

    m_telemetryModeJson[COMMAND_INFO] = m_commandJson;
    m_ruleEnginePropertyJson[COMMAND_INFO] = m_commandJson;
    m_ruleEnginePropertyJson[TYPE] = "rule_engine";
    m_ruleEnginePropertyJson[COMMAND] = "rule_device_data";
    m_ruleEnginePropertyJson[DEVICE_ID] = deviceId;

    // Read persisted alert configuration
    m_alertPercistancyFileName = "./config/" + processName + "/" + deviceId + "_alert_persistency.json";
    m_lastAlertPersistancyJson = ReadAndSetConfiguration(m_alertPercistancyFileName);

    // Read persisted rule engine configuration
    m_ruleEnginePropertiesFileName = "./config/" + processName + "/" + deviceId + "_rule_engine_parsistancy.json";
    nlohmann::json ruleEnginePersistancyJson = ReadAndSetConfiguration(m_ruleEnginePropertiesFileName);

    // Populate rule engine properties
    for (const auto& x : ruleEnginePersistancyJson["properties"])
    {
        std::cout << "x : " << x << "\n";
        m_ruleEnginePropertyMap.insert(x);
    }
}

ReadProperties::~ReadProperties()
{
    // Proper cleanup: close connection and stop threads
    CloseConnection();
    m_ThreadRunning = false;

    // Clean up dynamically allocated ManageGroupData objects
    for (auto& [groupName, manageGrpObj] : m_commandGroupStructMap)
    {
        delete manageGrpObj;
    }
    m_commandGroupStructMap.clear();

    // Clean up group structures
    delete m_grpStructObj1;
    delete m_grpStructObj2;
    delete m_grpStructObj3;

    // Wait for the thread to finish execution
    m_threadObj.join();
}

void ReadProperties::CreateCommandStruct()
{
    // Clear existing command structures before rebuilding
    m_commandStructList.clear();

    // Iterate through each property in the map
    for (const auto& [propertyName, property] : m_propertiesMap)
    {
        std::string groupName = property->grpName;
        bool isAlert = property->isAlarm;
        long startAddress = property->startAddress;
        long secondaryDatatype = property->secondaryDatatype;
        char datatype = property->datatype;
        short functionCode = property->functionCode;
        short wordQuantity = 1;
        CMD_STRUCT* CommandAttr;

        // Determine number of registers to read based on data types
        if ((secondaryDatatype == DATA_BYTE || secondaryDatatype == DATA_UNSIGNED_16 || 
             secondaryDatatype == DATA_SIGNED_16 || secondaryDatatype == DATA_DECIMAL_FLOAT) &&
             (datatype == ANALOG || datatype == DIGITAL))
        {
            wordQuantity = 1;
        }
        else if (secondaryDatatype == DATA_UNSIGNED_16 && datatype == STRING)
        {
            wordQuantity = property->lastAddress;  // For string data across multiple registers
        }
        else
        {
            wordQuantity = 2;  // Default to 2 registers for other types
        }

        // Ensure the address is valid
        if (startAddress >= 0 && startAddress < MAXADDR)
        {
            bool found = false;

            // If it's a non-alert property, initialize group manager if not already present
            if (!isAlert)
            {
                if (m_commandGroupStructMap.find(groupName) == m_commandGroupStructMap.end())
                {
                    ManageGroupData* manageGrpObj = new ManageGroupData(groupName, m_deviceId, m_processName);
                    manageGrpObj->RegisterPropertiesCB(std::bind(&ReadProperties::PropertiesReceiver, this, std::placeholders::_1));
                    manageGrpObj->StartDeviceStateGetterThread();

                    // Assign group-specific structure
                    if (groupName == "g1") manageGrpObj->SetGroupDetails(m_grpStructObj1);
                    if (groupName == "g2") manageGrpObj->SetGroupDetails(m_grpStructObj2);
                    if (groupName == "g3") manageGrpObj->SetGroupDetails(m_grpStructObj3);

                    m_commandGroupStructMap[groupName] = manageGrpObj;
                }
            }

            // If no existing command block matches, create a new one
            if (!found)
            {
                CommandAttr = new CMD_STRUCT;
                CommandAttr->wordQuantity = wordQuantity;
                CommandAttr->prevWordQuantity = wordQuantity;
                CommandAttr->functionCode = functionCode;
                CommandAttr->dataType = datatype;
                CommandAttr->blockNo = ++m_blockNumber;
                CommandAttr->strtAddr = startAddress;
                CommandAttr->lastAddr = startAddress;

                // Associate property with this command block
                property->blockNumber = CommandAttr->blockNo;

                // Add command structure to the list
                m_commandStructList.emplace_back(CommandAttr);
            }
        }
    }
}

void ReadProperties::CreatePropertiesStruct()
{
	try
	{
        std::cout << " ReadProperties::CreatePropertiesStruct() : " << m_deviceProperties <<"\n\n"; 
		m_propertiesMap.clear(); 
		if( !m_deviceProperties["measured_properties"].is_null() )
		{
			nlohmann::json propertiesJson;
			propertiesJson["properties"] = m_deviceProperties["measured_properties"];
			AddPropertiesAndAlertInMap( propertiesJson, false );
		}
		
		if( !m_deviceProperties["alerts"].is_null() )
		{
			nlohmann::json alertsJson;
			alertsJson["properties"] = m_deviceProperties["alerts"];
			AddPropertiesAndAlertInMap( alertsJson, true );
		}
		
		if( !m_deviceProperties[DERIVED_PROPERTIES].is_null() )
		{
			m_derivedPropertiesJson[DERIVED_PROPERTIES] = m_deviceProperties[DERIVED_PROPERTIES];
		}
	}
	catch( nlohmann::json::exception &e )
	{
		std::cout <<"CreatePropertiesStruct() : " << e.id << " : " << e.what() << std::endl;
	}
}

void ReadProperties::AddPropertiesAndAlertInMap( nlohmann::json alertPropertiesJson, bool alertFlag  )
{
	try
	{
		for ( auto& x : alertPropertiesJson["properties"].items() )
		{
			PROPERTIES_STRUCT *property = new PROPERTIES_STRUCT;
			VALUE_TYPE *vt = new VALUE_TYPE;
			property->vt = vt;
			nlohmann::json jsonObj;
			jsonObj = x.value();
			nlohmann::json mapKeyJsonObj;
			mapKeyJsonObj["key"] = x.key();
			std::string mapKey = mapKeyJsonObj["key"];
			std::string slaveId = jsonObj["slave_id"];
            
			if( slaveId == m_slave_Id )
            {
                property->isAlarm = alertFlag;
                if( !jsonObj[DATA_TYPE].is_null() )
                {
                    std::cout << "jsonObj[DATA_TYPE] : " << jsonObj[DATA_TYPE] << "\n";
                    std::string value = jsonObj[DATA_TYPE];
                    property->datatype = value[0];
                    if( property->datatype == DIGITAL )
                    {
                        property->vt->digitalValue = false;
                    }
                }
                
                if( !jsonObj[SECONDARY_DATA_TYPE].is_null() )
                {
                    std::cout << "jsonObj[SECONDARY_DATA_TYPE] : " << jsonObj[SECONDARY_DATA_TYPE] << "\n";
                    property->secondaryDatatype = jsonObj[SECONDARY_DATA_TYPE];
                }
                
                if( !jsonObj[GROUP_NAME].is_null() )
                {
                    std::cout << "jsonObj[GROUP_NAME] : " << jsonObj[GROUP_NAME] << "\n";
                    property->grpName = jsonObj[GROUP_NAME];
                }
                
                
                if( !jsonObj[START_ADDRESS].is_null() )
                {
                    std::cout << "jsonObj[START_ADDRESS] : " << jsonObj[START_ADDRESS] << "\n";
                    property->startAddress = jsonObj[START_ADDRESS];
                }
                
                if( !jsonObj[PRECISION].is_null() )
                {
                    std::cout << "jsonObj[PRECISION] : " << jsonObj[PRECISION] << "\n";
                    property->precision = jsonObj[PRECISION];
                }
                
                if( !jsonObj[LAST_ADDRESS].is_null() )
                {
                    std::cout << "jsonObj[LAST_ADDRESS] : " << jsonObj[LAST_ADDRESS] << "\n";
                    property->lastAddress = jsonObj[LAST_ADDRESS];
                }
                
                if( !jsonObj[FUNCTION_CODE].is_null() )
                {
                    std::cout << "jsonObj[FUNCTION_CODE] : " << jsonObj[FUNCTION_CODE] << "\n";
                    property->functionCode = jsonObj[FUNCTION_CODE];
                }
                
                if( !jsonObj[BIT_NUMBER].is_null() )
                {
                    std::cout << "jsonObj[BIT_NUMBER] : " << jsonObj[BIT_NUMBER] << "\n";
                    property->bitNumber = jsonObj[BIT_NUMBER];
                }
                
                if( mapKey != "" )
                {
                    m_propertiesMap[mapKey] = property;
                }
                 
                m_totalNumberOfProperties = m_propertiesMap.size();
                std::cout << "*** m_totalNumberOfProperties : " << m_totalNumberOfProperties << "\n\n";
            }
           
        }
	}
	catch( nlohmann::json::exception &e )
	{
		std::cout <<"CreatePropertiesStruct() : " << e.id << " : " << e.what() << std::endl;
	}
}


void ReadProperties::AddDesiredPropertiesInTelemetry()
{
    for ( auto& x : m_derivedPropertiesJson[DERIVED_PROPERTIES].items() )
    {
        try
        {
            std::string propertyName = x.key();
            nlohmann::json valueJson = x.value();
            
            std::string condition = valueJson["condition"];//derived property condition;
            int precision = 2;//valueJson["p"];
            boost::format derivedFormat = boost::format( condition );
            
            for ( auto& x1 : valueJson["props"].items() )
            {
                std::string variableName = x1.value();
                if( !m_localTelemetryJson.contains( x1.value() ) )
                {
                    std::cout << "ReadProperties::AddDesiredPropertiesInTelemetry : Respected dependent property not registered : " << "\n\n";
                    return;
                }
                double value = m_localTelemetryJson[variableName];
                derivedFormat.operator %( value );
            }
            
            mu::Parser valueParser;
            std::string str = derivedFormat.str();
            valueParser.SetExpr( str.c_str() );
            std::cout << "Expression : " << str.c_str() << "\n\n";
            
            double calculatedValue = valueParser.Eval();
            char str1[100];
            sprintf ( str1, "%.*f", precision, calculatedValue );
            calculatedValue = atof( str1 );
            
            auto it = m_ruleEnginePropertyMap.find( propertyName );
            if( it != m_ruleEnginePropertyMap.end() )
            {
                m_ruleEnginePropertyJson[PROPERTIES][*it] = calculatedValue;
            }

            m_cmmonJson[m_deviceId]["g1"][DERIVED_PROPERTY_TYPE][propertyName] = calculatedValue;
            std::cout << "Evaluation : "<< calculatedValue << std::endl;
        }
        catch( nlohmann::json::exception &e )
        {
            std::cout << e.id << " : " << e.what() << std::endl;
        }
        catch( ... )
        {
            std::cout << "ReadProperties::AddDesiredPropertiesInTelemetry() - Unknown exception occured." << std::endl;
        }
    }
}

void ReadProperties::CloseConnection()
{
	if ( m_ctx != NULL )
	{
		modbus_flush(m_ctx);
		modbus_close(m_ctx);
		modbus_free(m_ctx);
	}
}

bool ReadProperties::ConnectToDevice()
{
    bool retStatus = false;
    std::stringstream logg;
	
    try
    {
        CloseConnection();
        long slaveId = m_connectionJson["slave_id"];
        
        long portNumber = m_connectionJson[PORT_NUMBER];
        std::string hostAddress = m_connectionJson[HOST_ADDRESS];
      
		std::cout << "\n portNumber : " << portNumber << std::endl;
		std::cout << "\n slaveId : " << slaveId << std::endl;
		std::cout << "\n hostAddress : " << hostAddress.c_str() << std::endl;
	  
        m_ctx = modbus_new_tcp( hostAddress.c_str(), portNumber );

		//int slave;
       /* Socket or file descriptor */
       //int s;
       //int debug;
       //int error_recovery;
       //struct timeval response_timeout;
       //struct timeval byte_timeout;
       //const modbus_backend_t *backend;
       //void *backend_data;
	
		//std::cout << "\n m_ctx->slave : " << m_ctx->slave << std::endl;
		//std::cout << "\n m_ctx->s : " << m_ctx.s << std::endl;
		//std::cout << "\n m_ctx->debug : " << m_ctx->debug << std::endl;
		//std::cout << "\n m_ctx->error_recovery : " << m_ctx->error_recovery << std::endl;
		//std::cout << "\n m_ctx->backend_data : " << (int8_t)m_ctx->backend_data << std::endl;

        if( modbus_connect( m_ctx ) == -1) 
        {
            std::cout << "\n Connection failed: : HostAddress :" << hostAddress.c_str() << ", PortNumber : " << portNumber;
            std::cout << "Error :" << modbus_strerror(errno) << std::endl;
            fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
            m_connectionStatus = COMMFAIL;
        }
        else
        {
            modbus_set_slave( m_ctx, slaveId );
            modbus_set_debug( m_ctx, 1 );
            retStatus = true;
            std::cout << "\n Connection successful : HostAddress :" << hostAddress.c_str() << "PortNumber : " << portNumber << "SlaveId : " << slaveId << std::endl;
            m_connectionStatus = CONNECTED;
        }
    }
    catch( nlohmann::json::exception &e )
    {
        std::cout << e.id << " : " << e.what() << std::endl;
    }
    catch( ... )
    {
        std::cout << "ReadProperties::ConnectToDevice() - Unknown exception occured." << std::endl;
    }
    
	return retStatus;
}

short ReadProperties::GetStates()
{
	std::stringstream logg;	
	
	
	CMD_STRUCT* commandStructObj;
	char Logg[2000];
	short NoOfVariablesRead = 0;
	for (auto i = m_commandStructList.begin(); i != m_commandStructList.end(); i++)		// 4 function codes
	{
		short wError = -1;
		commandStructObj = *i;
		uint16_t arrValues[255] = {0};
		uint8_t coilValues[255] = {0};

		//At a time modbus can not read more than 125 registers. Thus if word count in > 125 then log message and 
		//Ignore processing of those no of registers.
		if( commandStructObj->wordQuantity > 125 )
		{			
			std::cout << "Block size out of range.. wordQuantity : " << commandStructObj->wordQuantity << std::endl; 
			continue;
		}
		else
		{
            int retryCount = 0;
            
            while( retryCount < 3 )
            {
                retryCount++;
                //******************************* Send Query & Get Response *****************************************/
                wError = ReadRegisters( commandStructObj->functionCode, commandStructObj->strtAddr, 
                        commandStructObj->wordQuantity, commandStructObj->dataType, arrValues, coilValues);

                if( wError != -1 )
                {
                    NoOfVariablesRead += ParseValues(commandStructObj->functionCode,commandStructObj->strtAddr,
                                            commandStructObj->wordQuantity, commandStructObj->blockNo, arrValues, coilValues );
                    break;
                }
                else
                {
                    std::cout << "Error in reading properties " << wError << std::endl;
                }
                usleep(10);
            }
		}
	}
	
	if(NoOfVariablesRead > 0)
	{
        m_connectionNotificationFlag = false;
		sprintf(Logg,"ModbusMaster::readVariables() - No of Variables : %d read Successfully out of : %d\n\n",
				NoOfVariablesRead,m_totalNumberOfProperties);
		std::cout << Logg;
		
		std::string Logg_string = Logg;
		
		//std::string print_log = "echo " + Logg_string + " > log.txt"; 
			
		//system(print_log.c_str());
	}
	else
	{
        m_connectionNotificationFlag = true;
		m_connectionStatus = COMMFAIL;
		std::cout << "ERROR : Zero properties read out of : " << m_totalNumberOfProperties << std::endl;
		//std::string print_log = "echo ERROR : Zero properties read out of :  > log_error.txt"; 
		
		//system(print_log.c_str());
	}
	return NoOfVariablesRead;
}

short ReadProperties::ReadRegisters( int functionCode, long startAddress, short wordQuant, int dataType, uint16_t wordValues[], uint8_t coilValues[]) 
{
    short nError = -1;	
	std::stringstream logg;
	
	int flag;
	std::cout <<  "functionCode : " << functionCode << "\tstartAddress : " << startAddress;
	std::cout <<  "\twordQuant : " << wordQuant << "\tdataType : " << dataType;
	//std::cout <<  "\twordQuant : " << wordQuant << "\tdataType : " << dataType;
	 
	switch ( functionCode )
	{
		case 1: //READ_COIL_STATUS: //01
			coilValues[wordQuant];
			nError = modbus_read_bits(m_ctx, startAddress, wordQuant, coilValues ) ;
			flag = 1;
			break;
				
		case 2: //READ_INPUT_STATUS: //02
			coilValues[wordQuant];
			nError = modbus_read_input_bits(m_ctx, startAddress, wordQuant, coilValues );
			break;	
		
		case 3: //READ_HOLDING_REGISTERS:	//03
			wordValues[wordQuant];
			nError = modbus_read_registers(m_ctx, startAddress, wordQuant, wordValues );
			break;

		case 4: //READ_INPUT_REGISTERS: //04
			wordValues[wordQuant];
			nError = modbus_read_input_registers(m_ctx, startAddress, wordQuant, wordValues );
			break;
	}
	
	if ( nError == -1 )
	{
		std::cout << "ERROR : Read Properties failed.. StartAddress : " << startAddress << " "<< modbus_strerror( nError ) << std::endl;
	}
	
	return nError;
}

short ReadProperties::ParseValues( int functionCode, long strtAddr, short wordQuant, short blockNumber,
                                    uint16_t wordValues[], uint8_t coilValues[] )
{
	short NoOfVariablesRead = 0;
    std::stringstream logg;
    
    logg.str("");
    try
    {
        for ( auto it : m_propertiesMap )
        { 
            short propertyFunctionCode = it.second->functionCode;
            int bn = it.second->blockNumber;
            if (( propertyFunctionCode != functionCode ) ||  ( bn != blockNumber ))
            {
                continue;
            }
            
            std::string propertyName = it.first;
            std::string grpName = it.second->grpName;
            long startAddress = it.second->startAddress;
            long secondaryDatatype = it.second->secondaryDatatype;
            char datatype = it.second->datatype;
            short bitNumber = it.second->bitNumber;
            VALUE_TYPE *newValueType = new VALUE_TYPE;
            
            switch( datatype )
            {
                case ANALOG:
                {
                    switch ( secondaryDatatype )
                    {
                        case DATA_BYTE:
                        {								
                            newValueType->analogValue = MODBUS_GET_HIGH_BYTE( wordValues[(startAddress)-strtAddr] );
                        }
                        break;
                        
                        case DATA_SIGNED_16:
                        {
                            newValueType->analogValue = wordValues[(startAddress)-strtAddr];
                        }
                        break;
                            
                        case DATA_UNSIGNED_16:
                        {
                            double tempData = (unsigned short)wordValues[(startAddress)-strtAddr];
                            newValueType->analogValue = tempData;
                        }
                        break;	
                            
                        case DATA_LONG:
                        {								
                            newValueType->analogValue = MODBUS_GET_INT32_FROM_INT16( wordValues, (startAddress)-strtAddr );
                        }
                        break;

                        case DATA_LONG_REVERSE:
                        {		
                            uint16_t arr[10];
                            arr[0] = wordValues[(startAddress-strtAddr)+1];
                            arr[1] = wordValues[(startAddress-strtAddr)];
                            newValueType->analogValue = MODBUS_GET_INT32_FROM_INT16(arr, (startAddress)-strtAddr );
                        }
                        break;
                            
                        case DATA_FLOAT_REVERSE :
                        {
                            newValueType->analogValue = modbus_get_float_dcba(&wordValues[(startAddress-strtAddr)]); 
                            char str[100];
                            sprintf (str, "%.*f", it.second->precision, newValueType->analogValue );
                            newValueType->analogValue = atof( str );
                        }
                        break;
						
						case DATA_FLOAT_REVERSE_ABCD :
                        {
                            newValueType->analogValue = modbus_get_float_abcd(&wordValues[(startAddress-strtAddr)]); 
                            char str[100];
                            sprintf (str, "%.*f", it.second->precision, newValueType->analogValue );
                            newValueType->analogValue = atof( str );
                        }
                        break;
						
						case DATA_FLOAT_REVERSE_BACD :
                        {
                            newValueType->analogValue = modbus_get_float_badc(&wordValues[(startAddress-strtAddr)]); 
                            char str[100];
                            sprintf (str, "%.*f", it.second->precision, newValueType->analogValue );
                            newValueType->analogValue = atof( str );
                        }
                        break;
						
						case DATA_FLOAT_REVERSE_CDAB :
                        {
                            newValueType->analogValue = modbus_get_float_cdab(&wordValues[(startAddress-strtAddr)]); 
                            char str[100];
                            sprintf (str, "%.*f", it.second->precision, newValueType->analogValue );
                            newValueType->analogValue = atof( str );
                        }
                        break;

                        case DATA_FLOAT :
                        {
                            newValueType->analogValue = modbus_get_float(&wordValues[(startAddress-strtAddr)]);
                            char str[100];
                            sprintf (str, "%.*f", it.second->precision, newValueType->analogValue );
                            newValueType->analogValue = atof( str );
                        }
                        break;	
                        
                        case DATA_DECIMAL_FLOAT:
                        {
                            double tempData = (unsigned short)wordValues[(startAddress)-strtAddr];
                            newValueType->analogValue = tempData / 10;
                        }
                        break;
                    }
                }
                break;
                case DIGITAL:
                {
                    if( bitNumber == -1 )
                    {
                        newValueType->digitalValue = coilValues[(startAddress-strtAddr)];;
                    }
                    else
                    {
                        unsigned short unsData = (unsigned short )wordValues[startAddress-strtAddr];
                        int maskVal = pow( 2,(bitNumber) );		
                        newValueType->digitalValue = ( unsData & maskVal ) != 0x00;
                    }
                }
                break;
                case STRING:
                {
                    std::string resString = "";
                    int registerNo = it.second->lastAddress;
                    int i = startAddress-strtAddr; // for reading multiple string in same or diff blocks.
                    while( (registerNo > 0)  && (i <= wordQuant ))
                    {
                        char firstc = MODBUS_GET_HIGH_BYTE(wordValues[i]);	
                        char secondc = MODBUS_GET_LOW_BYTE(wordValues[i]);
                        resString+= secondc;
                        resString+= firstc;
                        i++;
                        registerNo--;
                    }
                }
                break;
            }
            
            if( it.second->isAlarm )
            {
                CreateTelemetryAndAlertJson( it.second->isAlarm, it.second->vt, newValueType, datatype, propertyName );
            }
            else
            {
                CreateTelemetryJson( newValueType, datatype, propertyName, grpName );
            }
            
            if( newValueType )
            {
                delete newValueType;
            }
            
            NoOfVariablesRead++;
        }
    
    }
    catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "FileUploadWrapper::FormatAndUploadJson,  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		std::cout << logg.str() << "\n\n";
	}
	catch( ... )
	{
		logg.str("");
		logg << "FileUploadWrapper::FormatAndUploadJson,  Message : Unknown exception occured.";
        std::cout << logg.str() << "\n\n";
	}
	return NoOfVariablesRead;
}



void ReadProperties::CreateTelemetryJson( VALUE_TYPE *newValueType, char datatype, 
                                          std::string propertyName, std::string grpName  )
{
    switch( datatype )
    {
        case ANALOG:
        {
            m_cmmonJson[m_deviceId][grpName]["m"][propertyName] = newValueType->analogValue;
            m_localTelemetryJson[propertyName] = newValueType->analogValue;
            
            auto it = m_ruleEnginePropertyMap.find( propertyName );
            if( it != m_ruleEnginePropertyMap.end() )
            {
                m_ruleEnginePropertyJson[PROPERTIES][*it] = newValueType->analogValue;
            }
        }
        break;
        case DIGITAL:
        {
            m_cmmonJson[m_deviceId][grpName]["m"][propertyName] = newValueType->digitalValue;
            m_localTelemetryJson[propertyName] = newValueType->digitalValue;
            
            auto it = m_ruleEnginePropertyMap.find( propertyName );
            if( it != m_ruleEnginePropertyMap.end() )
            {
                m_ruleEnginePropertyJson[PROPERTIES][*it] = newValueType->digitalValue;
            }
            
        }
        break;
        case STRING:
        {
            m_cmmonJson[m_deviceId][grpName]["m"][propertyName] = newValueType->stringValue;
            m_localTelemetryJson[propertyName] = newValueType->digitalValue;
            
            auto it = m_ruleEnginePropertyMap.find( propertyName );
            if( it != m_ruleEnginePropertyMap.end() )
            {
                m_ruleEnginePropertyJson[PROPERTIES][*it] = newValueType->stringValue;
            }
        }
        break;
    }
}

void ReadProperties::CreateTelemetryAndAlertJson( bool isAlertFlag, VALUE_TYPE *oldValueType,
									VALUE_TYPE *newValueType, char datatype, std::string propertyName )
{
    std::stringstream logg;
    try
    {
        nlohmann::json alertJson;
        std::string alertTimeStamp = GetTimeStamp();
        std::time_t result = std::time(nullptr);
        std::string messageId = m_deviceId + "_" + std::to_string(result);
        
        alertJson[TIMESTAMP] = alertTimeStamp;
        alertJson[CODE] = propertyName;
        
        switch( datatype )
        {
            case ANALOG:
            {	 
                if( isAlertFlag )
                {
                    int alrtCode = atoi(propertyName.c_str() ); //11
                    int receivedvalue = (int)newValueType->analogValue; //11
                    bool checkFlag = false;
                    
                    if( !m_lastAlertPersistancyJson[ALERT][propertyName][STATUS].is_null() && m_firstTimeAppStartFlag )
                    {
                        m_lastAlertVal = m_lastAlertPersistancyJson[ALERT][propertyName][STATUS];
                        if( m_lastAlertVal == alrtCode )
                        {
                            m_firstTimeAppStartFlag = false;
                            oldValueType->analogValue = m_lastAlertVal;
                        }
                    }

                    
                    if( m_lastAlertVal == alrtCode )
                    {
                        if( alrtCode != receivedvalue )
                        {
                            if( oldValueType->analogValue != newValueType->analogValue  && oldValueType->analogValue != -1 )
                            {
                                oldValueType->analogValue = -1;
                                if( !m_lastAlertPersistancyJson[ALERT][propertyName][ALERT_START_EVENT].is_null() )
                                {
                                    alertJson[ALERT_START_EVENT] = m_lastAlertPersistancyJson[ALERT][propertyName][ALERT_START_EVENT];
                                }
                                if( !m_lastAlertPersistancyJson[ALERT][propertyName]["message_id"].is_null() )
                                {
                                    alertJson["message_id"] = m_lastAlertPersistancyJson[ALERT][propertyName]["message_id"];
                                }
                                 
                                alertJson[TYPE] = "alertendevent";
                                alertJson[ALERT_MSG_DATA] = m_lastAlertPersistancyJson[ALERT][propertyName][ALERT_MSG_DATA];
                                m_lastAlertPersistancyJson[ALERT][propertyName] = nullptr;
                                checkFlag = true;
                            }
                        }
                    }
                    
                    if( alrtCode == receivedvalue )
                    {
                        if( oldValueType->analogValue != newValueType->analogValue )
                        {
                            oldValueType->analogValue = newValueType->analogValue;
                            m_lastAlertVal = receivedvalue;
                            m_lastAlertPersistancyJson[ALERT][propertyName][STATUS] =  receivedvalue;
                            m_lastAlertPersistancyJson[ALERT][propertyName]["message_id"] =  messageId;
                            m_lastAlertPersistancyJson[ALERT][propertyName][ALERT_START_EVENT] =  alertTimeStamp;
                            m_lastAlertPersistancyJson[ALERT][propertyName][ALERT_MSG_DATA] =  alertTimeStamp;
                            
                            alertJson[TYPE] = "alert";//11
                            alertJson["message_id"] = messageId;
                            alertJson[ALERT_MSG_DATA] = alertTimeStamp;
                            checkFlag = true;
                        }
                    }
                    
                    if( checkFlag )
                    {
                        WriteConfiguration( m_alertPercistancyFileName, m_lastAlertPersistancyJson );
                        m_alertJsonArray[DATA].push_back(alertJson);
                        checkFlag = false;
                    }
                }
                else
                {
                    if( m_changeValueState )
                    {
                        if( oldValueType->analogValue != newValueType->analogValue )
                        {
                            std::cout << " oldValueType->analogValue : " << oldValueType->analogValue << "\n";
                            std::cout << " newValueType->analogValue : " << newValueType->analogValue << "\n\n";
                            oldValueType->analogValue = newValueType->analogValue;
                            m_telemetryJson[MEASURED_PROPERTY_TYPE][propertyName] = newValueType->analogValue;
                        }
                    }
                    else
                    {
                        m_telemetryJson[MEASURED_PROPERTY_TYPE][propertyName] = newValueType->analogValue;
                        std::cout << " newValueType->analogValue : " << newValueType->analogValue << "\n\n";
                    }
                        
                    auto it = m_ruleEnginePropertyMap.find( propertyName );
                    if( it != m_ruleEnginePropertyMap.end() )
                    {
                        m_ruleEnginePropertyJson[PROPERTIES][*it] = newValueType->analogValue;
                    }
                }
            }
            break;
            case DIGITAL:
            {
                if( isAlertFlag )
                {
                    if( !m_lastAlertPersistancyJson[ALERT][propertyName][STATUS].is_null() )
                    {
                        oldValueType->digitalValue = m_lastAlertPersistancyJson[ALERT][propertyName][STATUS];
                    }
                    
                    if( oldValueType->digitalValue != newValueType->digitalValue )
                    {
                        oldValueType->digitalValue = newValueType->digitalValue;
                        if( newValueType->digitalValue )
                        {
                            m_lastAlertPersistancyJson[ALERT][propertyName][STATUS] =  newValueType->digitalValue;
                            m_lastAlertPersistancyJson[ALERT][propertyName]["message_id"] =  messageId;
                            m_lastAlertPersistancyJson[ALERT][propertyName][ALERT_MSG_DATA] =  alertTimeStamp;
                            
                            alertJson[TYPE] = "alert";
                            alertJson["message_id"] = messageId;
                            alertJson[ALERT_MSG_DATA] = alertTimeStamp;
                        }
                        else
                        {
                             if( !m_lastAlertPersistancyJson[ALERT][propertyName]["message_id"].is_null() )
                             {
                                alertJson["message_id"] = m_lastAlertPersistancyJson[ALERT][propertyName]["message_id"];
                             }
                             
                            alertJson[TYPE] = "alertendevent";
                            alertJson[ALERT_MSG_DATA] = m_lastAlertPersistancyJson[ALERT][propertyName][ALERT_MSG_DATA];
                            m_lastAlertPersistancyJson[ALERT][propertyName] = nullptr;
                        }
                        
                        WriteConfiguration( m_alertPercistancyFileName, m_lastAlertPersistancyJson );
                        m_alertJsonArray[DATA].push_back(alertJson);
                    }
                }
                else
                {
                    if ( m_changeValueState )
                    {
                        if( oldValueType->digitalValue != newValueType->digitalValue )
                        {
                            oldValueType->digitalValue = newValueType->digitalValue;
                            m_telemetryJson[MEASURED_PROPERTY_TYPE][propertyName] = newValueType->digitalValue;
                        }
                    }
                    else
                    {
                        m_telemetryJson[MEASURED_PROPERTY_TYPE][propertyName] = newValueType->digitalValue;
                    }
                }
            
                auto it = m_ruleEnginePropertyMap.find( propertyName );
                if( it != m_ruleEnginePropertyMap.end() )
                {
                    m_ruleEnginePropertyJson[PROPERTIES][*it] = newValueType->digitalValue;
                }
            }
            break;
            case STRING:
            {
                if ( m_changeValueState )
                {
                    if( oldValueType->stringValue != newValueType->stringValue )
                    {
                        strcpy( oldValueType->stringValue, newValueType->stringValue );
                        //oldValueType->stringValue = newValueType->stringValue;
                        m_telemetryJson[MEASURED_PROPERTY_TYPE][propertyName] = newValueType->stringValue;
                    }
                }
                else
                {
                    m_telemetryJson[MEASURED_PROPERTY_TYPE][propertyName] = newValueType->stringValue;
                }

                auto it = m_ruleEnginePropertyMap.find( propertyName );
                if( it != m_ruleEnginePropertyMap.end() )
                {
                    m_ruleEnginePropertyJson[PROPERTIES][*it] = newValueType->stringValue;
                }
            }
            break;
        }
    }
    catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "FileUploadWrapper::FormatAndUploadJson,  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		std::cout << logg.str() << "\n\n";
	}
	catch( ... )
	{
		logg.str("");
		logg << "FileUploadWrapper::FormatAndUploadJson,  Message : Unknown exception occured.";
        std::cout << logg.str() << "\n\n";
	}
}

void ReadProperties::SetThreadStatus( bool threadStartStaus )
{
	m_threadStartStaus = threadStartStaus;
}

void ReadProperties::SetDeviceProperties( nlohmann::json deviceProperties )
{
	if( !deviceProperties.is_null() )
	{
		m_deviceProperties = deviceProperties;
	}
}

void ReadProperties::StartDeviceStateGetterThread()
{
	std::stringstream logg;
	m_threadObj = std::thread([this](){
		while( m_ThreadRunning )
		{
			if( m_threadStartStaus )
			{
				if( m_connectionStatus )
				{
					/*
					std::stringstream logg;
					logg.str("");
					logg << "ReadProperties::StartDeviceStateGetterThread() : m_connectionStatus success";
					m_exceptionLoggerObj->LogDebug( logg.str() );
					*/

					short noOfPropertiesRead = GetStates();

					if( noOfPropertiesRead > 0 )
					{
                        if( m_cmmonJson.is_null() )
                        {
                            sleep( m_pollingFreq/1000 );
                            continue;
                        }
                        
						if( !m_derivedPropertiesJson.is_null() )
						{
							AddDesiredPropertiesInTelemetry();
						} 
                        
                        m_cmmonJson[m_deviceId]["ts"] = GetTimeStamp();
                        
                        for( auto itr2 = m_commandGroupStructMap.begin(); itr2 != m_commandGroupStructMap.end();itr2++)
                        {
                            ManageGroupData *obj = itr2->second;
                            obj->SetDataJson( m_cmmonJson ); 
                        }
                        
                        //senddata to common class
                        //m_commonJsonInstance->SetSensorData( m_cmmonJson );
                        
                        //clear m_cmmonJson
                        m_cmmonJson.clear();
						
						if( !m_alertJsonArray[DATA].empty() )
						{
							m_propertiesCB( m_alertJsonArray );
							m_alertJsonArray["data"].clear();
						}
						
						if( !m_ruleEnginePropertyJson[PROPERTIES].is_null() )
						{
                            std::stringstream logg;
							std::cout << "m_ruleEnginePropertyJson Send to RuleEngine from ModbusTCP : " << m_ruleEnginePropertyJson << std::endl; // H1
							m_propertiesCB( m_ruleEnginePropertyJson ); 
							m_ruleEnginePropertyJson[PROPERTIES].clear();
						}
					}
				}
                else
                {
                    if( m_connectionNotificationFlag )
                    {
                        m_connectionNotificationFlag = false;
						
						std::cout << "ReadProperties::StartDeviceStateGetterThread() - SendsNotificationToCloud(1) \n";
                        
						SendNotificationToCloud(1);
                    }
                    
                    ConnectToDevice();
                }
			}
            long pollinfreq = m_pollingFreq;
			usleep( pollinfreq*1000 );
		}
		
	});	
}


void ReadProperties::SetConfig( nlohmann::json config )
{
	try
	{
		std::string configJson = config[COMMAND];
		transform(configJson.begin(), configJson.end(), configJson.begin(), ::tolower);
		if( configJson == CHANGE_DEVICE_MODE )
		{
			nlohmann::json errorTelemetryModeJson;
			std::string deviceMode = config[TELEMETRY_MODE];
            
            for( auto itr2 = m_commandGroupStructMap.begin(); itr2 != m_commandGroupStructMap.end();itr2++)
            {
                ManageGroupData *obj = itr2->second;
                bool flag = false;
                if( deviceMode == "turbo" )
                {
                    flag = true;
                }
                obj->SetTurboMode( m_turboTimeout, flag ); 
            }
		}
		else if( configJson == "register_rule_device_properties" )
		{
            std::cout << "\nModbusTCP ReadProperties::SetConfig() config - " << config << std::endl;
            nlohmann::json ruleEnginePersistancyJson;
			for( auto& x : config["properties"].items() )
			{
                nlohmann::json t1Json = x.value();
				std::string propertyName = t1Json["property"];
				m_ruleEnginePropertyMap.insert( propertyName );

                ruleEnginePersistancyJson = ReadAndSetConfiguration( m_ruleEnginePropertiesFileName );
                nlohmann::json jsonarr;
                ruleEnginePersistancyJson["properties"].push_back(propertyName);
			}
            if( !ruleEnginePersistancyJson.is_null() )
            {
                WriteConfiguration( m_ruleEnginePropertiesFileName,ruleEnginePersistancyJson );
            }
		}
		else if( configJson == SET_VALUE_CANAGE )
		{
			m_changeValueState = config["scv"];
		}
	}
	catch( nlohmann::json::exception &e )
	{
		std::cout <<"SetConfig() [ERROR]: " << e.id << " : " << e.what() << std::endl;
		std::cout <<"SetConfig() [ERROR]: frequency_in_sec : " << config << std::endl;
	}
}

void ReadProperties::RegisterPropertiesCB( std::function<void(nlohmann::json)> cb )
{
	m_propertiesCB = cb;
}


void ReadProperties::SendNotificationToCloud( int caseId, nlohmann::json config )
{
    nlohmann::json connectionFailJson;
    
    switch( caseId )
    {
        case 1:
        {
            connectionFailJson[TYPE] = "notification";
            connectionFailJson[DEVICE_ID] = m_deviceId;
            connectionFailJson[MESSAGE] = "PLC or Slave connection Failed";
            connectionFailJson[EVENT] = "plc_connection_fail";
            connectionFailJson[TIMESTAMP] = GetTimeStamp();
            connectionFailJson[COMMAND_INFO] = m_commandJson;
        }
        break;
        
        case 2:
        {
            connectionFailJson[CONFIGURATION].clear();
            if( config.is_null() )
            {
                connectionFailJson[SUB_JOB_ID] = m_telemetryModeSubJobId;
                m_telemetryModeSubJobId = "";
            }
            else
            {
                connectionFailJson[SUB_JOB_ID] = config[SUB_JOB_ID];
            }
            connectionFailJson[TYPE] = "notification";
            connectionFailJson[MESSAGE] = "Successfully configured to normal mode";
            connectionFailJson[CONFIGURATION][TELEMETRY_MODE] = "normal";
            connectionFailJson[TIMESTAMP] = GetTimeStamp();
            connectionFailJson[DEVICE_ID] = m_deviceId;
            connectionFailJson[COMMAND_INFO] = m_commandJson;
        }
        break;
        
        /*case 3:
        {
            connectionFailJson[SUB_JOB_ID] = config[SUB_JOB_ID];
            connectionFailJson[TYPE] = "notification";
            connectionFailJson[MESSAGE] = "Successfully configured to turbo mode";
            connectionFailJson[CONFIGURATION][TELEMETRY_MODE] = config[TELEMETRY_MODE];
            connectionFailJson[CONFIGURATION][TURBO_MODE_FREQUENCY] = m_turboModeFrequencyMiliSec/1000;
            connectionFailJson[CONFIGURATION][TURBO_MODE_TIMEOUT] = m_turboModeTimeoutInMiliSec/1000;
            connectionFailJson[DEVICE_ID] = m_deviceId;
            connectionFailJson[TIMESTAMP] = GetTimeStamp();
            connectionFailJson[COMMAND_INFO] = m_commandJson;
        }
        break;
        case 4:
        {
            std::stringstream tempValue;
            long pollingFreq = m_pollingFrequency/1000;
            tempValue << pollingFreq;
            connectionFailJson[SUB_JOB_ID] = config[SUB_JOB_ID];
            connectionFailJson[COMMAND_INFO] = m_commandJson;
            connectionFailJson[TYPE] = "error";
            connectionFailJson[MESSAGE] = "Turbo mode configuration failed. Please set turbo_mode_frequency_in_sec is greater than or equal to " + tempValue.str() + " sec";
            connectionFailJson[DEVICE_ID] = m_deviceId;
            connectionFailJson[TIMESTAMP] = GetTimeStamp();
            connectionFailJson[COMMAND_INFO] = m_commandJson;
        }
        break;
        */
        
        case 5:
        {
            std::string deviceMode = config[TELEMETRY_MODE];
            connectionFailJson[SUB_JOB_ID] = config[SUB_JOB_ID];
            connectionFailJson[COMMAND_INFO] = m_commandJson;
            connectionFailJson[TYPE] = "error";
            connectionFailJson[MESSAGE] = "Already configured " + deviceMode + " mode.";
            connectionFailJson[DEVICE_ID] = m_deviceId;
            connectionFailJson[TIMESTAMP] = GetTimeStamp();
        }
        break;
    }
    
    if( !connectionFailJson.is_null() )
    {
        m_propertiesCB( connectionFailJson );
    }
	
}


void ReadProperties::UpdateConfigFile()
{
    try
    {
        std::string configFilePath = m_configFilePath + DEVICE_CONFIGURATION_JSON;
        nlohmann::json config = ReadAndSetConfiguration( configFilePath );
        
        std::cout << "$$$$$$$$$$$$$$$$$$$$$   **********  \n" <<config << "\n***********************\n\n";
        
        if( config.is_null() )
        {
            nlohmann::json deviceconfig;

            deviceconfig["assets"][m_deviceId]["g1_ingestion_frequency_in_ms"] = 10000;
            deviceconfig["assets"][m_deviceId]["g1_measurement_frequency_in_ms"] = 5000;
            deviceconfig["assets"][m_deviceId]["g1_turbo_mode_frequency_in_ms"] = 5000;
            deviceconfig["assets"][m_deviceId]["g2_ingestion_frequency_in_ms"] = 20000;
            deviceconfig["assets"][m_deviceId]["g2_measurement_frequency_in_ms"] = 10000;
            deviceconfig["assets"][m_deviceId]["g2_turbo_mode_frequency_in_ms"] = 10000;
            deviceconfig["assets"][m_deviceId]["g3_ingestion_frequency_in_ms"] = 30000;
            deviceconfig["assets"][m_deviceId]["g3_measurement_frequency_in_ms"] = 15000;
            deviceconfig["assets"][m_deviceId]["g3_turbo_mode_frequency_in_ms"] = 15000;
            deviceconfig["assets"][m_deviceId]["ingestion_settings_type"] = "all_props_at_fixed_interval";
            deviceconfig["assets"][m_deviceId]["telemetry_mode"] = "normal";
            deviceconfig["assets"][m_deviceId]["turbo_mode_timeout_in_milli_sec"] = 120000;
            deviceconfig["assets"][m_deviceId]["turbo_mode_timeout_in_ms"] = 60000;
            
            WriteConfiguration( configFilePath ,deviceconfig);
            config = deviceconfig;
        }
        
        
        
        nlohmann::json twinReportedJson;
        twinReportedJson[COMMAND_INFO] = m_commandJson;
        twinReportedJson[TYPE] = "reported_twin";
        twinReportedJson["apps"][m_processName][STATUS] = "Running";
        
        //Set Measurment frequency
        if( config["assets"][m_deviceId].contains( G1_MEASUREMENT_FREQUENCY ) )
        {
            m_grpStructObj1->measuredFrequency = config["assets"][m_deviceId][G1_MEASUREMENT_FREQUENCY];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][G1_MEASUREMENT_FREQUENCY] = m_grpStructObj1->measuredFrequency;
        }
        
        if( config["assets"][m_deviceId].contains( G2_MEASUREMENT_FREQUENCY ) )
        {
            m_grpStructObj2->measuredFrequency = config["assets"][m_deviceId][G2_MEASUREMENT_FREQUENCY];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][G2_MEASUREMENT_FREQUENCY]  = m_grpStructObj2->measuredFrequency;
        }

        if( config["assets"][m_deviceId].contains( G3_MEASUREMENT_FREQUENCY ) )
        {
            m_grpStructObj3->measuredFrequency = config["assets"][m_deviceId][G3_MEASUREMENT_FREQUENCY];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][G3_MEASUREMENT_FREQUENCY]  = m_grpStructObj3->measuredFrequency;
        }
        
        //set ingation frequency
        if( config["assets"][m_deviceId].contains( G1_INGESTION_FREQUENCY ) )
        {
            m_grpStructObj1->uploadFrequency = config["assets"][m_deviceId][G1_INGESTION_FREQUENCY];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][G1_INGESTION_FREQUENCY] = m_grpStructObj1->uploadFrequency;
        }

        if( config["assets"][m_deviceId].contains( G2_INGESTION_FREQUENCY ) )
        {
            m_grpStructObj2->uploadFrequency = config["assets"][m_deviceId][G2_INGESTION_FREQUENCY];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][G2_INGESTION_FREQUENCY]  = m_grpStructObj2->uploadFrequency;
        }

        if( config["assets"][m_deviceId].contains( G3_INGESTION_FREQUENCY ) )
        {
            m_grpStructObj3->uploadFrequency = config["assets"][m_deviceId][G3_INGESTION_FREQUENCY];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][G3_INGESTION_FREQUENCY]  = m_grpStructObj3->uploadFrequency;
        }
        
        //set Turbo mode
        if( config["assets"][m_deviceId].contains( G1_TURBO_MODE_FREQUENCY ) )
        {
            m_grpStructObj1->turboModeFrequency = config["assets"][m_deviceId][G1_TURBO_MODE_FREQUENCY];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][G1_TURBO_MODE_FREQUENCY]  = m_grpStructObj1->turboModeFrequency;
        }

        if( config["assets"][m_deviceId].contains( G2_TURBO_MODE_FREQUENCY ) )
        {
            m_grpStructObj2->turboModeFrequency = config["assets"][m_deviceId][G2_TURBO_MODE_FREQUENCY];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][G2_TURBO_MODE_FREQUENCY]  = m_grpStructObj2->turboModeFrequency;
        }

        if( config["assets"][m_deviceId].contains( G3_TURBO_MODE_FREQUENCY ) )
        {
            m_grpStructObj3->turboModeFrequency = config["assets"][m_deviceId][G3_TURBO_MODE_FREQUENCY];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][G3_TURBO_MODE_FREQUENCY]  = m_grpStructObj3->turboModeFrequency;
        }
        
        if( config["assets"][m_deviceId].contains( "turbo_mode_timeout_in_ms" ) )
        {
            m_turboTimeout = config["assets"][m_deviceId]["turbo_mode_timeout_in_ms"];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId]["turbo_mode_timeout_in_ms"]  = m_turboTimeout;
        }
        
        if( config["assets"][m_deviceId].contains( INGESTION_SETTINGS_TYPE ) )
        {
            std::string strType = config["assets"][m_deviceId][INGESTION_SETTINGS_TYPE];
            if( strType == ALL_PROPERTIES )
            {
                m_changeValueState = false;
            }
            else
            {
                m_changeValueState = true;
            }
        }
        
         std::string type = ALL_PROPERTIES;
        if( m_changeValueState )
        {
            type = CHANGED_PROPERTIES;
        }
            
        if( m_grpStructObj1->measuredFrequency <= m_grpStructObj2->measuredFrequency && m_grpStructObj1->measuredFrequency <= m_grpStructObj3->measuredFrequency) 
        { 
            m_pollingFreq = m_grpStructObj1->measuredFrequency;
        }
        else if( m_grpStructObj2->measuredFrequency <= m_grpStructObj1->measuredFrequency && m_grpStructObj2->measuredFrequency <= m_grpStructObj3->measuredFrequency) 
        {
            m_pollingFreq = m_grpStructObj2->measuredFrequency;
        }
        else 
        {
            m_pollingFreq = m_grpStructObj3->measuredFrequency;
        }    
        
        for( auto itr = m_commandGroupStructMap.begin(); itr != m_commandGroupStructMap.end(); ++itr )
        {
            ManageGroupData *grpObj = itr->second;
            if( itr->first == "g1" )
            {
                grpObj->SetGroupDetails( m_grpStructObj1 );
            }
            
            if( itr->first == "g2" )
            {
                grpObj->SetGroupDetails( m_grpStructObj2 );
            }
            
            if( itr->first == "g3" )
            {
                grpObj->SetGroupDetails( m_grpStructObj3 );
            }
        }
        
        twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][TELEMETRY_MODE] = "normal";
        twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][INGESTION_SETTINGS_TYPE] = type;
        //m_propertiesCB( twinReportedJson );
    }
    catch( nlohmann::json::exception &e )
	{
		std::cout <<"ReadProperties::UpdateConfigFile() : " << e.id << " : " << e.what() << std::endl;
	}
    catch( ...)
	{
		std::cout <<"ReadProperties::UpdateConfigFile() : frequency_in_sec : " << std::endl;
	}
}

void ReadProperties::PropertiesReceiver( nlohmann::json jsonObj )
{
	m_propertiesCB( jsonObj );
}
