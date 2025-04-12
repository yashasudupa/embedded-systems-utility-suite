#ifndef ManageGroupData_h
#define ManageGroupData_h 1
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>
#include "Common.h"
#include "GlobalOperations.h"
#include "ExceptionLogger.h"
#include <thread>

class ManageGroupData
{
public:
    ManageGroupData( std::string grpName, std::string deviceId, std::string processName );
    ~ManageGroupData();
    void StartDeviceStateGetterThread();
    void SetGroupDetails( GROUP_STRUCT *groupStructObj );
    void RegisterPropertiesCB( std::function<void(nlohmann::json)> cb );
    void SetDataJson( nlohmann::json dataJson );
    void SetTurboMode( long turboTimeOut, bool setMode = false  );
    
private:

    
private:
    GROUP_STRUCT *m_groupStructObj;
    
    std::string m_grpName;
    std::string m_deviceId;
    
    bool m_ThreadRunning;
    bool m_threadStartStaus;
    std::thread m_threadObj;
    std::function<void(nlohmann::json)> m_propertiesCB;
    
    long m_measurementFreq;
    long m_measurementFreq1;
    long m_uploadFreq;
    long m_uploadCount;
    long m_uploadCountConstant;
    long m_tempuploadCountConstant;
    long m_turboTimeoutCountConst;
    long m_turboTimeoutCount;
    nlohmann::json m_telemetryJsonArray;
    nlohmann::json m_telemetryJson;
    nlohmann::json m_commandJson;
    nlohmann::json m_dataJson;
    ExceptionLogger *m_exceptionLoggerObj;
};

#endif