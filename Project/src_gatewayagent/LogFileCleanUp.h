#ifndef LOGFILECLEANUP_h
#define LOGFILECLEANUP_h 1
#include <thread>
#include <mutex>
#include <dirent.h>
#include <algorithm>
#include <sstream>

#include "Common.h"
#include "GlobalOperations.h"
#include "SendMessageToCloudWrapper.h"

#define NUMBER_OF_DAYS_TO_DELETE            1
#define LOG_FILE_PATH                       "/opt/IoT_Gateway/GatewayAgent/logs"

class LogFileCleanUp
{
public:
    LogFileCleanUp( std::string gatewayId );
    ~LogFileCleanUp();
    
private:
    void StartLogCleanUpThread();
    void SetLogCleanUpTime( long cleanupTime );
    void StartThread( bool startFlag );
    std::list<std::string> getListOfFiles();
    void CheckAndCleanLogFiles();
    
private:
    
    bool m_logCleanUpThreadFlag;
    bool m_ThreadRunning;
    
    long m_logCleanUpTimeCount;
    long m_cleanUpCount;
    
    ExceptionLogger *m_exceptionLoggerObj;
    SendMessageToCloudWrapper *m_sendMsgWrapperObj;
    std::mutex m_mutex;
    std::thread m_threadObj;
    
    std::string m_currentDate;
    std::string m_previousDate;
    std::string m_gatewayId;
    
};

#endif