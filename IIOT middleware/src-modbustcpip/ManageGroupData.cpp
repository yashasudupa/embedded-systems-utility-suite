#include "ManageGroupData.h"

// Constructor initializes member variables and populates initial JSON schema for telemetry
ManageGroupData::ManageGroupData(std::string grpName, std::string deviceId, std::string processName) :
    m_grpName(grpName),
    m_threadStartStaus(true),
    m_ThreadRunning(true),
    m_uploadCountConstant(1),
    m_tempuploadCountConstant(1),
    m_deviceId(deviceId),
    m_turboTimeoutCountConst(0),
    m_turboTimeoutCount(0),
    m_measurementFreq1(5000),
    m_uploadCount(0)
{
    // Prepare base command JSON structure
    m_commandJson[APP_NAME] = processName;
    m_commandJson[APP_ID] = GetProcessIdByName(processName);
    m_commandJson[COMMAND_TYPE] = RESPONSE;

    // Embed command structure into telemetry JSON
    m_telemetryJsonArray[COMMAND_INFO] = m_commandJson;
    m_telemetryJsonArray[COMMAND_INFO][COMMAND_SCHEMA] = "telemetry";
    m_telemetryJsonArray[DEVICE_ID] = m_deviceId; 
    m_telemetryJsonArray["group_name"] = grpName; 
    m_telemetryJsonArray[TYPE] = MULTI_TELEMETRY;
}

// Destructor stops thread execution
ManageGroupData::~ManageGroupData()
{
    m_ThreadRunning = false;
    m_threadStartStaus = false;
}

// Sets group-specific configuration values
void ManageGroupData::SetGroupDetails(GROUP_STRUCT* groupStructObj)
{
    try
    {
        m_groupStructObj = groupStructObj;
        m_measurementFreq = groupStructObj->measuredFrequency;
        m_measurementFreq1 = m_measurementFreq * 1000; // Convert seconds to milliseconds
        m_uploadFreq = groupStructObj->uploadFrequency;

        std::cout << "SetGroupDetails m_uploadFreq " << m_uploadFreq << "\n\n"; 
        std::cout << "SetGroupDetails m_measurementFreq " << m_measurementFreq << "\n\n"; 
        std::cout << "SetGroupDetails m_uploadCountConstant " << m_uploadCountConstant << "\n\n"; 

        // Determine how many measurements should be taken before an upload
        if (m_uploadFreq >= m_measurementFreq)
        {
            m_uploadCountConstant = m_uploadFreq / m_measurementFreq;
            m_tempuploadCountConstant = m_uploadCountConstant;
        }
    }
    catch (nlohmann::json::exception& e)
    {
        std::cout << "ManageGroupData::SetGroupDetails() : " << e.id << " : " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "ReadProperties::SetGroupDetails : Unknown Exception occurred." << std::endl;
    }
}

// Sets the sensor data to be processed in the thread loop
void ManageGroupData::SetDataJson(nlohmann::json dataJson)
{
    m_dataJson = dataJson;
}

// Enables or disables turbo mode by adjusting upload frequency
void ManageGroupData::SetTurboMode(long turboTimeOut, bool setMode)
{
    if (setMode == true)
    {
        m_uploadCountConstant = 1; // Upload after every measurement in turbo mode
        m_turboTimeoutCountConst = turboTimeOut / m_measurementFreq;
    }
    else
    {
        m_uploadCountConstant = m_tempuploadCountConstant; // Restore normal mode
    }
}

// Starts a thread to periodically fetch and send device state
void ManageGroupData::StartDeviceStateGetterThread()
{
    std::cout << "start StartDeviceStateGetterThread : " << "\n\n";
    m_threadObj = std::thread([this]() {
        while (m_ThreadRunning)
        {
            try
            {
                if (m_threadStartStaus)
                {
                    // Copy the latest data (assumed to be externally updated)
                    nlohmann::json latestData = m_dataJson;
                    std::cout << "common JSON : " << latestData << "\n\n";

                    nlohmann::json telemetryJson;
                    bool dataSendFlag = false;

                    // Extract group-specific data if available
                    if (latestData[m_deviceId].contains(m_grpName))
                    {
                        telemetryJson = latestData[m_deviceId][m_grpName];
                        telemetryJson["ts"] = latestData[m_deviceId]["ts"];
                        m_dataJson.clear();
                    }

                    // Proceed only if data is valid
                    if (!telemetryJson[MEASURED_PROPERTY_TYPE].is_null())
                    {
                        m_uploadCount++;

                        // If turbo mode is active, count elapsed time
                        if (m_turboTimeoutCountConst > 0)
                        {
                            m_turboTimeoutCount++;
                        }

                        // Send or buffer the telemetry data based on frequency count
                        if (!telemetryJson.is_null() && m_uploadCount >= m_uploadCountConstant)
                        {
                            m_telemetryJsonArray[LATEST] = telemetryJson;

                            // Send via registered callback if available
                            if (m_propertiesCB)
                            {
                                m_propertiesCB(m_telemetryJsonArray);
                            }

                            // Clear old data after upload
                            m_telemetryJsonArray[DATA].clear();
                            m_telemetryJsonArray[LATEST].clear();
                            m_uploadCount = 0;
                        }
                        else
                        {
                            // Buffer the data to send later
                            m_telemetryJsonArray[DATA].push_back(telemetryJson);
                            dataSendFlag = true;
                        }

                        // Exit turbo mode if timeout is reached
                        if (m_turboTimeoutCount > m_turboTimeoutCountConst)
                        {
                            m_turboTimeoutCountConst = 0;
                            m_turboTimeoutCount = 0;
                            m_uploadCountConstant = m_tempuploadCountConstant;
                        }
                    }
                }

                // Sleep for measurement interval
                usleep(m_measurementFreq1);
            }
            catch (nlohmann::json::exception& e)
            {
                std::cout << "StartDeviceStateGetterThread::StartDeviceStateGetterThread() : " << e.id << " : " << e.what() << std::endl;
            }
            catch (...)
            {
                std::cout << "ReadProperties::StartDeviceStateGetterThread : Unknown Exception occurred." << std::endl;
            }
        }
    });
}

// Registers a callback to be used for sending telemetry data externally
void ManageGroupData::RegisterPropertiesCB(std::function<void(nlohmann::json)> cb)
{
    m_propertiesCB = cb;
}
