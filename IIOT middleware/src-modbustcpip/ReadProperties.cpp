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

// Function to parse the device JSON and populate internal property structures
void ReadProperties::CreatePropertiesStruct()
{
	try
	{
        std::cout << " ReadProperties::CreatePropertiesStruct() : " << m_deviceProperties <<"\n\n"; 
        
        // Clear the existing map to avoid duplicates or stale entries
		m_propertiesMap.clear(); 
		
        // Check and add measured properties from JSON
		if( !m_deviceProperties["measured_properties"].is_null() )
		{
			nlohmann::json propertiesJson;
			propertiesJson["properties"] = m_deviceProperties["measured_properties"];
			AddPropertiesAndAlertInMap(propertiesJson, false); // alertFlag = false
		}
		
        // Check and add alerts from JSON
		if( !m_deviceProperties["alerts"].is_null() )
		{
			nlohmann::json alertsJson;
			alertsJson["properties"] = m_deviceProperties["alerts"];
			AddPropertiesAndAlertInMap(alertsJson, true); // alertFlag = true
		}
		
        // Check and save derived properties JSON
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

// Function to create property structures and store them in a map for measured or alert data
void ReadProperties::AddPropertiesAndAlertInMap(nlohmann::json alertPropertiesJson, bool alertFlag)
{
	try
	{
        // Iterate through all JSON key-value pairs
		for ( auto& x : alertPropertiesJson["properties"].items() )
		{
			// Allocate memory for property structure and its value holder
			PROPERTIES_STRUCT *property = new PROPERTIES_STRUCT;
			VALUE_TYPE *vt = new VALUE_TYPE;
			property->vt = vt;
			
			nlohmann::json jsonObj = x.value();
			nlohmann::json mapKeyJsonObj;
			mapKeyJsonObj["key"] = x.key();
			
			std::string mapKey = mapKeyJsonObj["key"];
			std::string slaveId = jsonObj["slave_id"];
            
            // Only process properties for matching slave ID
			if( slaveId == m_slave_Id )
            {
                property->isAlarm = alertFlag; // Set alert flag

                // Parse datatype and initialize default value
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

                // Parse optional fields safely
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

                // Add the property struct to map if map key is valid
                if( mapKey != "" )
                {
                    m_propertiesMap[mapKey] = property;
                }

                // Update the count of total properties
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

// Function to compute and insert derived (calculated) telemetry values into telemetry JSON
void ReadProperties::AddDesiredPropertiesInTelemetry()
{
    for ( auto& x : m_derivedPropertiesJson[DERIVED_PROPERTIES].items() )
    {
        try
        {
            std::string propertyName = x.key();          // Name of the derived property
            nlohmann::json valueJson = x.value();        // Associated JSON object

            std::string condition = valueJson["condition"]; // Expression string with placeholders
            int precision = 2; // Future enhancement: can be parsed from JSON

            // Format string with dependent property values
            boost::format derivedFormat = boost::format(condition);

            for ( auto& x1 : valueJson["props"].items() )
            {
                std::string variableName = x1.value();

                // Check if the dependent property exists
                if( !m_localTelemetryJson.contains(variableName) )
                {
                    std::cout << "ReadProperties::AddDesiredPropertiesInTelemetry : Respected dependent property not registered : " << "\n\n";
                    return;
                }

                double value = m_localTelemetryJson[variableName];
                derivedFormat.operator %(value); // Insert value into format string
            }

            // Use mathematical expression parser to evaluate derived expression
            mu::Parser valueParser;
            std::string str = derivedFormat.str();
            valueParser.SetExpr(str.c_str());

            std::cout << "Expression : " << str << "\n\n";

            double calculatedValue = valueParser.Eval(); // Evaluate expression

            // Round to desired precision
            char str1[100];
            sprintf(str1, "%.*f", precision, calculatedValue);
            calculatedValue = atof(str1);

            // Update rule engine property map if present
            auto it = m_ruleEnginePropertyMap.find(propertyName);
            if( it != m_ruleEnginePropertyMap.end() )
            {
                m_ruleEnginePropertyJson[PROPERTIES][*it] = calculatedValue;
            }

            // Add to telemetry JSON under derived property section
            m_cmmonJson[m_deviceId]["g1"][DERIVED_PROPERTY_TYPE][propertyName] = calculatedValue;
            std::cout << "Evaluation : "<< calculatedValue << std::endl;
        }
        catch( nlohmann::json::exception &e )
        {
            std::cout << e.id << " : " << e.what() << std::endl;
        }
        catch( ... )
        {
            std::cout << "ReadProperties::AddDesiredPropertiesInTelemetry() - Unknown exception occurred." << std::endl;
        }
    }
}

// Function to close and free Modbus connection
void ReadProperties::CloseConnection()
{
	if ( m_ctx != NULL )
	{
		modbus_flush(m_ctx);   // Ensure all data has been transmitted
		modbus_close(m_ctx);   // Close the connection
		modbus_free(m_ctx);    // Free the context memory
	}
}

// Establishes a Modbus TCP connection to the device
bool ReadProperties::ConnectToDevice()
{
    bool retStatus = false;

    try
    {
        CloseConnection();  // Ensure any previous connection is closed

        // Extract connection parameters from JSON
        long slaveId = m_connectionJson["slave_id"];
        long portNumber = m_connectionJson[PORT_NUMBER];
        std::string hostAddress = m_connectionJson[HOST_ADDRESS];

        // Debug prints for connection parameters
        std::cout << "\nPort Number: " << portNumber;
        std::cout << "\nSlave ID: " << slaveId;
        std::cout << "\nHost Address: " << hostAddress << std::endl;

        // Create new Modbus TCP context
        m_ctx = modbus_new_tcp(hostAddress.c_str(), portNumber);

        // Attempt to connect
        if (modbus_connect(m_ctx) == -1)
        {
            std::cout << "Connection failed: " << modbus_strerror(errno) << std::endl;
            m_connectionStatus = COMMFAIL;
        }
        else
        {
            modbus_set_slave(m_ctx, slaveId);
            modbus_set_debug(m_ctx, 1);
            m_connectionStatus = CONNECTED;
            retStatus = true;
            std::cout << "Connection successful\n";
        }
    }
    catch (nlohmann::json::exception &e)
    {
        std::cout << e.id << " : " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "ReadProperties::ConnectToDevice() - Unknown exception occurred." << std::endl;
    }

    return retStatus;
}

// Reads all Modbus properties defined in m_commandStructList
short ReadProperties::GetStates()
{
    CMD_STRUCT* commandStructObj;
    short NoOfVariablesRead = 0;

    for (auto i = m_commandStructList.begin(); i != m_commandStructList.end(); i++)
    {
        short wError = -1;
        commandStructObj = *i;
        uint16_t arrValues[255] = {0};
        uint8_t coilValues[255] = {0};

        // Skip reading if word count exceeds Modbus limit
        if (commandStructObj->wordQuantity > 125)
        {
            std::cout << "Block size out of range. Word Quantity: " << commandStructObj->wordQuantity << std::endl;
            continue;
        }

        // Retry mechanism for reading registers
        for (int retryCount = 0; retryCount < 3; ++retryCount)
        {
            wError = ReadRegisters(commandStructObj->functionCode, commandStructObj->strtAddr,
                                   commandStructObj->wordQuantity, commandStructObj->dataType, arrValues, coilValues);

            if (wError != -1)
            {
                NoOfVariablesRead += ParseValues(commandStructObj->functionCode, commandStructObj->strtAddr,
                                                 commandStructObj->wordQuantity, commandStructObj->blockNo,
                                                 arrValues, coilValues);
                break;
            }
            else
            {
                std::cout << "Error reading properties, attempt: " << retryCount + 1 << std::endl;
                usleep(10);
            }
        }
    }

    // Logging
    if (NoOfVariablesRead > 0)
    {
        m_connectionNotificationFlag = false;
        std::cout << "ModbusMaster::readVariables() - Variables read successfully: " 
                  << NoOfVariablesRead << " out of " << m_totalNumberOfProperties << std::endl;
    }
    else
    {
        m_connectionNotificationFlag = true;
        m_connectionStatus = COMMFAIL;
        std::cout << "ERROR: Zero properties read out of " << m_totalNumberOfProperties << std::endl;
    }

    return NoOfVariablesRead;
}

// Executes the actual Modbus read operation based on the function code
short ReadProperties::ReadRegisters(int functionCode, long startAddress, short wordQuant, int dataType,
                                    uint16_t wordValues[], uint8_t coilValues[])
{
    short nError = -1;

    std::cout << "Function Code: " << functionCode << "\tStart Address: " << startAddress
              << "\tWord Quantity: " << wordQuant << "\tData Type: " << dataType << std::endl;

    switch (functionCode)
    {
        case 1: // Read Coils
            nError = modbus_read_bits(m_ctx, startAddress, wordQuant, coilValues);
            break;

        case 2: // Read Discrete Inputs
            nError = modbus_read_input_bits(m_ctx, startAddress, wordQuant, coilValues);
            break;

        case 3: // Read Holding Registers
            nError = modbus_read_registers(m_ctx, startAddress, wordQuant, wordValues);
            break;

        case 4: // Read Input Registers
            nError = modbus_read_input_registers(m_ctx, startAddress, wordQuant, wordValues);
            break;
    }

    if (nError == -1)
    {
        std::cout << "ERROR: Read failed at address " << startAddress << " - " << modbus_strerror(errno) << std::endl;
    }

    return nError;
}

// Parses raw register/coil values into meaningful properties
short ReadProperties::ParseValues(int functionCode, long strtAddr, short wordQuant, short blockNumber,
                                  uint16_t wordValues[], uint8_t coilValues[])
{
    short NoOfVariablesRead = 0;

    try
    {
        for (auto& it : m_propertiesMap)
        {
            if (it.second->functionCode != functionCode || it.second->blockNumber != blockNumber)
                continue;

            std::string propertyName = it.first;
            std::string grpName = it.second->grpName;
            long startAddress = it.second->startAddress;
            long secondaryDatatype = it.second->secondaryDatatype;
            char datatype = it.second->datatype;
            short bitNumber = it.second->bitNumber;

            VALUE_TYPE* newValueType = new VALUE_TYPE;

            switch (datatype)
            {
                case ANALOG:
                    switch (secondaryDatatype)
                    {
                        case DATA_BYTE:
                            newValueType->analogValue = MODBUS_GET_HIGH_BYTE(wordValues[startAddress - strtAddr]);
                            break;

                        case DATA_SIGNED_16:
                        case DATA_UNSIGNED_16:
                            newValueType->analogValue = static_cast<unsigned short>(wordValues[startAddress - strtAddr]);
                            break;

                        case DATA_LONG:
                            newValueType->analogValue = MODBUS_GET_INT32_FROM_INT16(wordValues, startAddress - strtAddr);
                            break;

                        case DATA_LONG_REVERSE:
                        {
                            uint16_t arr[] = {
                                wordValues[startAddress - strtAddr + 1],
                                wordValues[startAddress - strtAddr]
                            };
                            newValueType->analogValue = MODBUS_GET_INT32_FROM_INT16(arr, 0);
                            break;
                        }

                        case DATA_FLOAT:
                        case DATA_FLOAT_REVERSE:
                        case DATA_FLOAT_REVERSE_ABCD:
                        case DATA_FLOAT_REVERSE_BACD:
                        case DATA_FLOAT_REVERSE_CDAB:
                        {
                            float val = 0.0;
                            if (secondaryDatatype == DATA_FLOAT)
                                val = modbus_get_float(&wordValues[startAddress - strtAddr]);
                            else if (secondaryDatatype == DATA_FLOAT_REVERSE)
                                val = modbus_get_float_dcba(&wordValues[startAddress - strtAddr]);
                            else if (secondaryDatatype == DATA_FLOAT_REVERSE_ABCD)
                                val = modbus_get_float_abcd(&wordValues[startAddress - strtAddr]);
                            else if (secondaryDatatype == DATA_FLOAT_REVERSE_BACD)
                                val = modbus_get_float_badc(&wordValues[startAddress - strtAddr]);
                            else if (secondaryDatatype == DATA_FLOAT_REVERSE_CDAB)
                                val = modbus_get_float_cdab(&wordValues[startAddress - strtAddr]);

                            char str[100];
                            sprintf(str, "%.*f", it.second->precision, val);
                            newValueType->analogValue = atof(str);
                            break;
                        }

                        case DATA_DECIMAL_FLOAT:
                            newValueType->analogValue = static_cast<double>(wordValues[startAddress - strtAddr]) / 10;
                            break;
                    }
                    break;

                case DIGITAL:
                    if (bitNumber == -1)
                    {
                        newValueType->digitalValue = coilValues[startAddress - strtAddr];
                    }
                    else
                    {
                        unsigned short val = wordValues[startAddress - strtAddr];
                        newValueType->digitalValue = (val & (1 << bitNumber)) != 0;
                    }
                    break;

                case STRING:
                {
                    std::string resString;
                    int i = startAddress - strtAddr;
                    int registerNo = it.second->lastAddress;

                    while ((registerNo > 0) && (i <= wordQuant))
                    {
                        resString += MODBUS_GET_LOW_BYTE(wordValues[i]);
                        resString += MODBUS_GET_HIGH_BYTE(wordValues[i]);
                        i++;
                        registerNo--;
                    }
                    break;
                }
            }

            // Push data to telemetry
            if (it.second->isAlarm)
                CreateTelemetryAndAlertJson(it.second->isAlarm, it.second->vt, newValueType, datatype, propertyName);
            else
                CreateTelemetryJson(newValueType, datatype, propertyName, grpName);

            delete newValueType;
            NoOfVariablesRead++;
        }
    }
    catch (nlohmann::json::exception &e)
    {
        std::cout << "ParseValues JSON error " << e.id << ": " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "ParseValues unknown exception occurred." << std::endl;
    }

    return NoOfVariablesRead;
}

// Function to create JSON entries for telemetry data based on value type (analog/digital/string)
void ReadProperties::CreateTelemetryJson(VALUE_TYPE *newValueType, char datatype, 
                                         std::string propertyName, std::string grpName)
{
    switch (datatype)
    {
        case ANALOG:
        {
            // Populate JSON for analog telemetry data
            m_cmmonJson[m_deviceId][grpName]["m"][propertyName] = newValueType->analogValue;
            m_localTelemetryJson[propertyName] = newValueType->analogValue;

            // If property exists in rule engine map, update rule engine JSON
            auto it = m_ruleEnginePropertyMap.find(propertyName);
            if (it != m_ruleEnginePropertyMap.end())
            {
                m_ruleEnginePropertyJson[PROPERTIES][*it] = newValueType->analogValue;
            }
        }
        break;

        case DIGITAL:
        {
            // Populate JSON for digital telemetry data
            m_cmmonJson[m_deviceId][grpName]["m"][propertyName] = newValueType->digitalValue;
            m_localTelemetryJson[propertyName] = newValueType->digitalValue;

            auto it = m_ruleEnginePropertyMap.find(propertyName);
            if (it != m_ruleEnginePropertyMap.end())
            {
                m_ruleEnginePropertyJson[PROPERTIES][*it] = newValueType->digitalValue;
            }
        }
        break;

        case STRING:
        {
            // Populate JSON for string telemetry data
            m_cmmonJson[m_deviceId][grpName]["m"][propertyName] = newValueType->stringValue;
            m_localTelemetryJson[propertyName] = newValueType->digitalValue; // Possibly a bug? Should be stringValue?

            auto it = m_ruleEnginePropertyMap.find(propertyName);
            if (it != m_ruleEnginePropertyMap.end())
            {
                m_ruleEnginePropertyJson[PROPERTIES][*it] = newValueType->stringValue;
            }
        }
        break;
    }
}

// Function to create telemetry and alert JSON based on current and old values
void ReadProperties::CreateTelemetryAndAlertJson(bool isAlertFlag, VALUE_TYPE *oldValueType,
                                                 VALUE_TYPE *newValueType, char datatype, std::string propertyName)
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

        switch (datatype)
        {
            case ANALOG:
            {
                if (isAlertFlag)
                {
                    int alrtCode = atoi(propertyName.c_str());
                    int receivedvalue = (int)newValueType->analogValue;
                    bool checkFlag = false;

                    // On first app start, get persisted alert state
                    if (!m_lastAlertPersistancyJson[ALERT][propertyName][STATUS].is_null() && m_firstTimeAppStartFlag)
                    {
                        m_lastAlertVal = m_lastAlertPersistancyJson[ALERT][propertyName][STATUS];
                        if (m_lastAlertVal == alrtCode)
                        {
                            m_firstTimeAppStartFlag = false;
                            oldValueType->analogValue = m_lastAlertVal;
                        }
                    }

                    // Check if existing alert is ending
                    if (m_lastAlertVal == alrtCode && alrtCode != receivedvalue)
                    {
                        if (oldValueType->analogValue != newValueType->analogValue && oldValueType->analogValue != -1)
                        {
                            oldValueType->analogValue = -1;

                            if (!m_lastAlertPersistancyJson[ALERT][propertyName][ALERT_START_EVENT].is_null())
                                alertJson[ALERT_START_EVENT] = m_lastAlertPersistancyJson[ALERT][propertyName][ALERT_START_EVENT];

                            if (!m_lastAlertPersistancyJson[ALERT][propertyName]["message_id"].is_null())
                                alertJson["message_id"] = m_lastAlertPersistancyJson[ALERT][propertyName]["message_id"];

                            alertJson[TYPE] = "alertendevent";
                            alertJson[ALERT_MSG_DATA] = m_lastAlertPersistancyJson[ALERT][propertyName][ALERT_MSG_DATA];

                            m_lastAlertPersistancyJson[ALERT][propertyName] = nullptr;
                            checkFlag = true;
                        }
                    }

                    // Check if a new alert is triggered
                    if (alrtCode == receivedvalue && oldValueType->analogValue != newValueType->analogValue)
                    {
                        oldValueType->analogValue = newValueType->analogValue;
                        m_lastAlertVal = receivedvalue;
                        m_lastAlertPersistancyJson[ALERT][propertyName][STATUS] = receivedvalue;
                        m_lastAlertPersistancyJson[ALERT][propertyName]["message_id"] = messageId;
                        m_lastAlertPersistancyJson[ALERT][propertyName][ALERT_START_EVENT] = alertTimeStamp;
                        m_lastAlertPersistancyJson[ALERT][propertyName][ALERT_MSG_DATA] = alertTimeStamp;

                        alertJson[TYPE] = "alert";
                        alertJson["message_id"] = messageId;
                        alertJson[ALERT_MSG_DATA] = alertTimeStamp;
                        checkFlag = true;
                    }

                    // If alert was triggered/ended, persist and push to alert array
                    if (checkFlag)
                    {
                        WriteConfiguration(m_alertPercistancyFileName, m_lastAlertPersistancyJson);
                        m_alertJsonArray[DATA].push_back(alertJson);
                        checkFlag = false;
                    }
                }
                else
                {
                    // Normal telemetry collection
                    if (m_changeValueState && oldValueType->analogValue != newValueType->analogValue)
                    {
                        oldValueType->analogValue = newValueType->analogValue;
                        m_telemetryJson[MEASURED_PROPERTY_TYPE][propertyName] = newValueType->analogValue;
                    }
                    else
                    {
                        m_telemetryJson[MEASURED_PROPERTY_TYPE][propertyName] = newValueType->analogValue;
                    }

                    auto it = m_ruleEnginePropertyMap.find(propertyName);
                    if (it != m_ruleEnginePropertyMap.end())
                    {
                        m_ruleEnginePropertyJson[PROPERTIES][*it] = newValueType->analogValue;
                    }
                }
            }
            break;

            case DIGITAL:
            {
                if (isAlertFlag)
                {
                    // Restore old alert state if any
                    if (!m_lastAlertPersistancyJson[ALERT][propertyName][STATUS].is_null())
                        oldValueType->digitalValue = m_lastAlertPersistancyJson[ALERT][propertyName][STATUS];

                    // Compare old and new to detect alert change
                    if (oldValueType->digitalValue != newValueType->digitalValue)
                    {
                        oldValueType->digitalValue = newValueType->digitalValue;

                        if (newValueType->digitalValue)
                        {
                            m_lastAlertPersistancyJson[ALERT][propertyName][STATUS] = newValueType->digitalValue;
                            m_lastAlertPersistancyJson[ALERT][propertyName]["message_id"] = messageId;
                            m_lastAlertPersistancyJson[ALERT][propertyName][ALERT_MSG_DATA] = alertTimeStamp;

                            alertJson[TYPE] = "alert";
                            alertJson["message_id"] = messageId;
                            alertJson[ALERT_MSG_DATA] = alertTimeStamp;
                        }
                        else
                        {
                            if (!m_lastAlertPersistancyJson[ALERT][propertyName]["message_id"].is_null())
                                alertJson["message_id"] = m_lastAlertPersistancyJson[ALERT][propertyName]["message_id"];

                            alertJson[TYPE] = "alertendevent";
                            alertJson[ALERT_MSG_DATA] = m_lastAlertPersistancyJson[ALERT][propertyName][ALERT_MSG_DATA];

                            m_lastAlertPersistancyJson[ALERT][propertyName] = nullptr;
                        }

                        WriteConfiguration(m_alertPercistancyFileName, m_lastAlertPersistancyJson);
                        m_alertJsonArray[DATA].push_back(alertJson);
                    }
                }
                else
                {
                    // Digital telemetry update
                    if (m_changeValueState && oldValueType->digitalValue != newValueType->digitalValue)
                    {
                        oldValueType->digitalValue = newValueType->digitalValue;
                        m_telemetryJson[MEASURED_PROPERTY_TYPE][propertyName] = newValueType->digitalValue;
                    }
                    else
                    {
                        m_telemetryJson[MEASURED_PROPERTY_TYPE][propertyName] = newValueType->digitalValue;
                    }
                }

                auto it = m_ruleEnginePropertyMap.find(propertyName);
                if (it != m_ruleEnginePropertyMap.end())
                {
                    m_ruleEnginePropertyJson[PROPERTIES][*it] = newValueType->digitalValue;
                }
            }
            break;

            case STRING:
            {
                // Telemetry for string values
                if (m_changeValueState && strcmp(oldValueType->stringValue, newValueType->stringValue) != 0)
                {
                    strcpy(oldValueType->stringValue, newValueType->stringValue);
                    m_telemetryJson[MEASURED_PROPERTY_TYPE][propertyName] = newValueType->stringValue;
                }
                else
                {
                    m_telemetryJson[MEASURED_PROPERTY_TYPE][propertyName] = newValueType->stringValue;
                }

                auto it = m_ruleEnginePropertyMap.find(propertyName);
                if (it != m_ruleEnginePropertyMap.end())
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
    catch(...)
    {
        logg.str("");
        logg << "FileUploadWrapper::FormatAndUploadJson,  Message : Unknown exception occured.";
        std::cout << logg.str() << "\n\n";
    }
}

// Set flag indicating if data fetch thread should run
void ReadProperties::SetThreadStatus(bool threadStartStaus)
{
    m_threadStartStaus = threadStartStaus;
}

// Set device configuration properties
void ReadProperties::SetDeviceProperties(nlohmann::json deviceProperties)
{
    if (!deviceProperties.is_null())
    {
        m_deviceProperties = deviceProperties;
    }
}

// Start the thread that polls device states and generates telemetry/alert JSON
void ReadProperties::StartDeviceStateGetterThread()
{
    std::stringstream logg;
    m_threadObj = std::thread([this]()
    {
        while (m_ThreadRunning)
        {
            if (m_threadStartStaus && m_connectionStatus)
            {
                short noOfPropertiesRead = GetStates(); // Fetch states from device

                if (noOfPropertiesRead > 0)
                {
                    if (m_cmmonJson.is_null())
                    {
                        sleep(m_pollingFreq / 1000);
                        continue;
                    }

                    if (!m_derivedPropertiesJson.is_null())
                    {
                        AddDesiredPropertiesInTelemetry();
                    }

                    m_cmmonJson[m_deviceId]["ts"] = GetTimeStamp(); // Add timestamp

                    // Update all group objects with common data
                    for (auto itr2 = m_commandGroupStructMap.begin(); itr2 != m_commandGroupStructMap.end(); itr2++)
                    {
                        ManageGroupData *obj = itr2->second;
                        obj->SetDataJson(m_cmmonJson);
                    }

                    // Clear after sending to avoid duplicate data
                    m_cmmonJson.clear();

                    // Send any pending alert JSONs
                    if (!m_alertJsonArray[DATA].empty())
                    {
                        m_propertiesCB(m_alertJsonArray);
                        m_alertJsonArray["data"].clear();
                    }

                    // Send rule engine properties if available
                    if (!m_ruleEnginePropertyJson[PROPERTIES].is_null())
                    {
                        std::cout << "m_ruleEnginePropertyJson Send to RuleEngine from ModbusTCP : " << m_ruleEnginePropertyJson << std::endl;
                        m_propertiesCB(m_ruleEnginePropertyJson);
                        m_ruleEnginePropertyJson[PROPERTIES].clear();
                    }
                }
            }
            else if (!m_connectionStatus)
            {
                if (m_connectionNotificationFlag)
                {
                    m_connectionNotificationFlag = false;
                    std::cout << "ReadProperties::StartDeviceStateGetterThread() - SendsNotificationToCloud(1) \n";
                    SendNotificationToCloud(1);
                }

                ConnectToDevice(); // Try reconnecting
            }

            usleep(m_pollingFreq * 1000); // Delay based on polling frequency
        }
    });
}

void ReadProperties::SetConfig(nlohmann::json config)
{
    try
    {
        // Extract and convert command string to lowercase for uniformity
        std::string configJson = config[COMMAND];
        transform(configJson.begin(), configJson.end(), configJson.begin(), ::tolower);

        // Handle device mode change
        if (configJson == CHANGE_DEVICE_MODE)
        {
            std::string deviceMode = config[TELEMETRY_MODE];
            bool enableTurbo = (deviceMode == "turbo");

            // Iterate over each group and set turbo mode
            for (auto& [groupName, groupObj] : m_commandGroupStructMap)
            {
                groupObj->SetTurboMode(m_turboTimeout, enableTurbo);
            }
        }
        // Register properties for rule engine persistence
        else if (configJson == "register_rule_device_properties")
        {
            std::cout << "\nModbusTCP ReadProperties::SetConfig() config - " << config << std::endl;
            nlohmann::json ruleEnginePersistancyJson = ReadAndSetConfiguration(m_ruleEnginePropertiesFileName);

            // Extract properties and update local map
            for (auto& item : config["properties"].items())
            {
                std::string propertyName = item.value()["property"];
                m_ruleEnginePropertyMap.insert(propertyName);
                ruleEnginePersistancyJson["properties"].push_back(propertyName);
            }

            // Write back updated rule engine configuration
            if (!ruleEnginePersistancyJson.is_null())
            {
                WriteConfiguration(m_ruleEnginePropertiesFileName, ruleEnginePersistancyJson);
            }
        }
        // Set state for value changes
        else if (configJson == SET_VALUE_CANAGE)
        {
            m_changeValueState = config["scv"];
        }
    }
    catch (nlohmann::json::exception& e)
    {
        std::cout << "SetConfig() [ERROR]: " << e.id << " : " << e.what() << std::endl;
        std::cout << "SetConfig() [ERROR]: frequency_in_sec : " << config << std::endl;
    }
}

void ReadProperties::RegisterPropertiesCB( std::function<void(nlohmann::json)> cb )
{
	m_propertiesCB = cb;
}

void ReadProperties::SendNotificationToCloud(int caseId, nlohmann::json config)
{
    nlohmann::json connectionFailJson;

    switch (caseId)
    {
        case 1: // PLC/Slave connection failure
            connectionFailJson = {
                {TYPE, "notification"},
                {DEVICE_ID, m_deviceId},
                {MESSAGE, "PLC or Slave connection Failed"},
                {EVENT, "plc_connection_fail"},
                {TIMESTAMP, GetTimeStamp()},
                {COMMAND_INFO, m_commandJson}
            };
            break;

        case 2: // Successfully configured to normal mode
            connectionFailJson[CONFIGURATION].clear();
            connectionFailJson[SUB_JOB_ID] = config.is_null() ? m_telemetryModeSubJobId : config[SUB_JOB_ID];
            m_telemetryModeSubJobId = "";
            connectionFailJson.update({
                {TYPE, "notification"},
                {MESSAGE, "Successfully configured to normal mode"},
                {CONFIGURATION, {{TELEMETRY_MODE, "normal"}}},
                {TIMESTAMP, GetTimeStamp()},
                {DEVICE_ID, m_deviceId},
                {COMMAND_INFO, m_commandJson}
            });
            break;

        case 5: // Already in requested mode
            connectionFailJson = {
                {SUB_JOB_ID, config[SUB_JOB_ID]},
                {COMMAND_INFO, m_commandJson},
                {TYPE, "error"},
                {MESSAGE, "Already configured " + config[TELEMETRY_MODE].get<std::string>() + " mode."},
                {DEVICE_ID, m_deviceId},
                {TIMESTAMP, GetTimeStamp()}
            };
            break;
    }

    // Send notification via callback if valid
    if (!connectionFailJson.is_null())
    {
        m_propertiesCB(connectionFailJson);
    }
}

void ReadProperties::UpdateConfigFile()
{
    try
    {
        std::string configFilePath = m_configFilePath + DEVICE_CONFIGURATION_JSON;
        nlohmann::json config = ReadAndSetConfiguration(configFilePath);

        // If file is empty or unreadable, initialize defaults
        if (config.is_null())
        {
            nlohmann::json defaultConfig;
            defaultConfig["assets"][m_deviceId] = {
                {"g1_ingestion_frequency_in_ms", 10000},
                {"g1_measurement_frequency_in_ms", 5000},
                {"g1_turbo_mode_frequency_in_ms", 5000},
                {"g2_ingestion_frequency_in_ms", 20000},
                {"g2_measurement_frequency_in_ms", 10000},
                {"g2_turbo_mode_frequency_in_ms", 10000},
                {"g3_ingestion_frequency_in_ms", 30000},
                {"g3_measurement_frequency_in_ms", 15000},
                {"g3_turbo_mode_frequency_in_ms", 15000},
                {"ingestion_settings_type", "all_props_at_fixed_interval"},
                {"telemetry_mode", "normal"},
                {"turbo_mode_timeout_in_milli_sec", 120000},
                {"turbo_mode_timeout_in_ms", 60000}
            };

            WriteConfiguration(configFilePath, defaultConfig);
            config = defaultConfig;
        }

        // Prepare reported twin payload
        nlohmann::json twinReportedJson = {
            {COMMAND_INFO, m_commandJson},
            {TYPE, "reported_twin"},
            {"apps", {
                {m_processName, {
                    {STATUS, "Running"}
                }}
            }}
        };

        // Set measurement and ingestion frequencies for G1, G2, G3
        auto& asset = config["assets"][m_deviceId];
        if (asset.contains(G1_MEASUREMENT_FREQUENCY))
        {
            m_grpStructObj1->measuredFrequency = asset[G1_MEASUREMENT_FREQUENCY];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][G1_MEASUREMENT_FREQUENCY] = m_grpStructObj1->measuredFrequency;
        }
        if (asset.contains(G2_MEASUREMENT_FREQUENCY))
        {
            m_grpStructObj2->measuredFrequency = asset[G2_MEASUREMENT_FREQUENCY];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][G2_MEASUREMENT_FREQUENCY] = m_grpStructObj2->measuredFrequency;
        }
        if (asset.contains(G3_MEASUREMENT_FREQUENCY))
        {
            m_grpStructObj3->measuredFrequency = asset[G3_MEASUREMENT_FREQUENCY];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][G3_MEASUREMENT_FREQUENCY] = m_grpStructObj3->measuredFrequency;
        }

        // Set ingestion frequency
        if (asset.contains(G1_INGESTION_FREQUENCY))
        {
            m_grpStructObj1->uploadFrequency = asset[G1_INGESTION_FREQUENCY];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][G1_INGESTION_FREQUENCY] = m_grpStructObj1->uploadFrequency;
        }
        if (asset.contains(G2_INGESTION_FREQUENCY))
        {
            m_grpStructObj2->uploadFrequency = asset[G2_INGESTION_FREQUENCY];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][G2_INGESTION_FREQUENCY] = m_grpStructObj2->uploadFrequency;
        }
        if (asset.contains(G3_INGESTION_FREQUENCY))
        {
            m_grpStructObj3->uploadFrequency = asset[G3_INGESTION_FREQUENCY];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId][G3_INGESTION_FREQUENCY] = m_grpStructObj3->uploadFrequency;
        }

        // Turbo mode frequency
        if (asset.contains(G1_TURBO_MODE_FREQUENCY))
            m_grpStructObj1->turboModeFrequency = asset[G1_TURBO_MODE_FREQUENCY];
        if (asset.contains(G2_TURBO_MODE_FREQUENCY))
            m_grpStructObj2->turboModeFrequency = asset[G2_TURBO_MODE_FREQUENCY];
        if (asset.contains(G3_TURBO_MODE_FREQUENCY))
            m_grpStructObj3->turboModeFrequency = asset[G3_TURBO_MODE_FREQUENCY];

        // Turbo mode timeout
        if (asset.contains("turbo_mode_timeout_in_ms"))
        {
            m_turboTimeout = asset["turbo_mode_timeout_in_ms"];
            twinReportedJson["apps"][m_processName]["asset_configuration"][m_deviceId]["turbo_mode_timeout_in_ms"] = m_turboTimeout;
        }

        // Ingestion setting type
        if (asset.contains(INGESTION_SETTINGS_TYPE))
        {
            std::string type = asset[INGESTION_SETTINGS_TYPE];
            m_changeValueState = (type != ALL_PROPERTIES);
        }

        // Determine lowest measurement frequency for polling
        m_pollingFreq = std::min({
            m_grpStructObj1->measuredFrequency,
            m_grpStructObj2->measuredFrequency,
            m_grpStructObj3->measuredFrequency
        });

        // Apply group settings
        for (auto& [group, grpObj] : m_commandGroupStructMap)
        {
            // logic to call grpObj->SetGroupDetails(...) based on group name
        }

        // Optional: Report twin to cloud, log config, etc.
    }
    catch (const std::exception& e)
    {
        std::cerr << "UpdateConfigFile() [ERROR]: " << e.what() << std::endl;
    }
}

void ReadProperties::PropertiesReceiver( nlohmann::json jsonObj )
{
	m_propertiesCB( jsonObj );
}
