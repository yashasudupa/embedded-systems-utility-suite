#ifndef ExceptionLogger_h
#define ExceptionLogger_h 1

#include <iostream>
#include <string>
#include <list>
#include <time.h>
#include <sys/stat.h>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <mutex>

#define LOG_DIRECTORY "./logs/"
#define MESSAGE_STRING_LENGTH			1000
#define LOGFILE_LENGTH					50

class ExceptionLogger 
{

  public:
    
    ~ExceptionLogger();
    
    
	void LogException (std::string Info = "");
    void LogError (std::string Info = "");
    void LogInfo (std::string Info = "");
    void LogDebug (std::string Info = "");
	
	
    void Init ( std::string ExceptionLogFileName, std::string appName );
	std::string GenerateCurrentDateStr();
	static ExceptionLogger *GetInstance();

  private:
	ExceptionLogger();
	bool CheckDateChange ();
	void OpenFile();
	void LogMessage (std::string Info = "");

  private: 
    std::fstream m_logFile;
    std::string m_logFileDate ;
    std::string m_logFileName ;
    std::string m_appName ;
	std::mutex m_mutexObj;
	static ExceptionLogger *m_exceptionLoggerInstance;
};

#endif
