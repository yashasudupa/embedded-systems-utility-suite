#include "ExceptionLogger.h"

#define MESSAGE_STRING_LENGTH			1000
#define LOGFILE_LENGTH					50

ExceptionLogger* ExceptionLogger::m_exceptionLoggerInstance = nullptr;;

ExceptionLogger* ExceptionLogger::GetInstance()
{
	 if (!m_exceptionLoggerInstance)
	 {
		m_exceptionLoggerInstance = new ExceptionLogger();
	 }
	 return m_exceptionLoggerInstance;
}

ExceptionLogger::ExceptionLogger()
{
	
}

ExceptionLogger::~ExceptionLogger()
{
	m_logFile.close();

}

//Log exception in Exception File
void ExceptionLogger::LogMessage ( std::string info  )
{
	try
	{
		if ( m_logFile.fail() )	// previous operation failed, close file and open again
		{
			OpenFile();
		}
		
		if( m_logFile )
		{
			// get date and time 
			char timeStr[MESSAGE_STRING_LENGTH];
			struct tm *newTime;
			time_t longTime;
			time( &longTime );                
			newTime = localtime( &longTime );

			// create date time string
			strftime( timeStr, MESSAGE_STRING_LENGTH, "[%d-%m-%Y %H:%M:%S]  ", newTime );

			//check date change for generating new file everyday
			CheckDateChange();
			// write to file
			std::string exceptionLog = timeStr;
			exceptionLog += "[" + m_appName + "]  " + info;
			m_logFile << exceptionLog << std::endl;
			std::cout << exceptionLog << std::endl;
			m_logFile.flush();
		}
	}
	catch( ... )
	{
		std::cout << "ExceptionLogger::LogMessage()  Exception occured while logging.\n";
	}
}

void ExceptionLogger::LogError ( std::string info  )
{
	std::string message = "[ERROR]  " + info;
	LogMessage( message );
}

void ExceptionLogger::LogException ( std::string info  )
{
	std::string message = "[EXCEPTION]  " + info;
	LogMessage( message );
}

void ExceptionLogger::LogInfo ( std::string info  )
{
	std::string message = "[INFO]  " + info;
	LogMessage( message );
}

void ExceptionLogger::LogDebug ( std::string info )
{
	std::string message = "[DEBUG]  " + info;
	LogMessage( message );
}

// generate date string for current date. 
// string format is dd-mm-yyyy 
std::string ExceptionLogger::GenerateCurrentDateStr()
{
	time_t currTime;
	time( &currTime );
	struct tm *timeStruct = localtime( &currTime );

	char dateStr[12];
	// create string
	strftime( dateStr, 11, "%d-%m-%Y", timeStruct );
	std::string str = dateStr;
	return str;
}

bool ExceptionLogger::CheckDateChange ()
{
	//check for time to restart the computer	
	if ( m_logFileDate == GenerateCurrentDateStr() )
	{
		return false;
	}
	m_logFileDate = GenerateCurrentDateStr() ;
	OpenFile();
	return true;
}

void ExceptionLogger::Init ( std::string exceptionLogFileName, std::string appName )
{
	m_logFileName = LOG_DIRECTORY + exceptionLogFileName ;
	m_logFileDate = GenerateCurrentDateStr() ;
	m_appName = appName;

	mkdir ( LOG_DIRECTORY, S_IRWXU | S_IRWXG | S_IRWXO );
	chmod( LOG_DIRECTORY, S_IRWXU | S_IRWXG | S_IRWXO );
	OpenFile();
	chmod( exceptionLogFileName.c_str(), S_IRWXU | S_IRWXG | S_IRWXO );
}


void ExceptionLogger::OpenFile()
{
	//log file name "PLSLogFile%d-%m-%Y.txt"
	std::string fileName = m_logFileName + "_";
	fileName += m_logFileDate;
	fileName += ".log";
	//unexpected Files are created 
	if(fileName.length() < 16)
		return;
	
	if ( m_logFile.is_open() )
	{
		m_logFile.close();
	}
	
	m_logFile.open( fileName.c_str(), std::ios::out | std::ios::app );
	if ( m_logFile.fail() )
	{
		printf("Could not open exception log file. No exceptions will be logged  : filename : %s!!!\n",fileName.c_str()); 
	}
}

