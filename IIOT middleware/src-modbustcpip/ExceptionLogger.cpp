#include "ExceptionLogger.h"

#define MESSAGE_STRING_LENGTH 1000
#define LOGFILE_LENGTH 50

// Initialize static member to hold singleton instance
ExceptionLogger* ExceptionLogger::m_exceptionLoggerInstance = nullptr;

// Singleton pattern: returns the single instance of ExceptionLogger
ExceptionLogger* ExceptionLogger::GetInstance()
{
	if (!m_exceptionLoggerInstance)
	{
		m_exceptionLoggerInstance = new ExceptionLogger();
	}
	return m_exceptionLoggerInstance;
}

// Constructor
ExceptionLogger::ExceptionLogger()
{
	// Nothing to initialize here for now
}

// Destructor: closes the log file if open
ExceptionLogger::~ExceptionLogger()
{
	m_logFile.close();
}

// Logs a general message to the log file with timestamp and app name
void ExceptionLogger::LogMessage(std::string info)
{
	try
	{
		// If previous file operation failed, reopen the file
		if (m_logFile.fail())
		{
			OpenFile();
		}

		// Proceed only if file is open and ready
		if (m_logFile)
		{
			char timeStr[MESSAGE_STRING_LENGTH];
			struct tm* newTime;
			time_t longTime;

			// Get current local time
			time(&longTime);
			newTime = localtime(&longTime);

			// Format time string
			strftime(timeStr, MESSAGE_STRING_LENGTH, "[%d-%m-%Y %H:%M:%S]  ", newTime);

			// Check if date has changed and switch log file if necessary
			CheckDateChange();

			// Construct full log message
			std::string exceptionLog = timeStr;
			exceptionLog += "[" + m_appName + "]  " + info;

			// Log to file and console
			m_logFile << exceptionLog << std::endl;
			std::cout << exceptionLog << "\n\n";
			m_logFile.flush();
		}
	}
	catch (...)
	{
		std::cout << "ExceptionLogger::LogMessage()  Exception occurred while logging.\n";
	}
}

// Logs an error message
void ExceptionLogger::LogError(std::string info)
{
	std::string message = "[ERROR]  " + info;
	LogMessage(message);
}

// Logs an exception
void ExceptionLogger::LogException(std::string info)
{
	std::string message = "[EXCEPTION]  " + info;
	LogMessage(message);
}

// Logs general info
void ExceptionLogger::LogInfo(std::string info)
{
	std::string message = "[INFO]  " + info;
	LogMessage(message);
}

// Logs debug information
void ExceptionLogger::LogDebug(std::string info)
{
	std::string message = "[DEBUG]  " + info;
	LogMessage(message);
}

// Generates the current date string in dd-mm-yyyy format
std::string ExceptionLogger::GenerateCurrentDateStr()
{
	time_t currTime;
	time(&currTime);
	struct tm* timeStruct = localtime(&currTime);

	char dateStr[12];
	strftime(dateStr, 11, "%d-%m-%Y", timeStruct);
	std::string str = dateStr;
	return str;
}

// Checks if the date has changed, and opens a new log file accordingly
bool ExceptionLogger::CheckDateChange()
{
	if (m_logFileDate == GenerateCurrentDateStr())
	{
		return false;
	}
	m_logFileDate = GenerateCurrentDateStr();
	OpenFile();
	return true;
}

// Initializes the logger with a log file name and application name
void ExceptionLogger::Init(std::string exceptionLogFileName, std::string appName)
{
	m_logFileName = LOG_DIRECTORY + exceptionLogFileName;
	m_logFileDate = GenerateCurrentDateStr();
	m_appName = appName;

	// Create log directory with full permissions
	mkdir(LOG_DIRECTORY, S_IRWXU | S_IRWXG | S_IRWXO);
	chmod(LOG_DIRECTORY, S_IRWXU | S_IRWXG | S_IRWXO);

	// Open the log file
	OpenFile();

	// Set permissions on the log file
	chmod(exceptionLogFileName.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
}

// Opens or reopens the log file using the current date
void ExceptionLogger::OpenFile()
{
	// Construct log file name using base name + date
	std::string fileName = m_logFileName + "_";
	fileName += m_logFileDate;
	fileName += ".log";

	// Safety check: avoid opening invalid file names
	if (fileName.length() < 16)
		return;

	// Close previously opened file
	if (m_logFile.is_open())
	{
		m_logFile.close();
	}

	// Open new log file in append mode
	m_logFile.open(fileName.c_str(), std::ios::out | std::ios::app);

	// If opening fails, log the issue to console
	if (m_logFile.fail())
	{
		printf("Could not open exception log file. No exceptions will be logged  : filename : %s!!!\n", fileName.c_str());
	}
}
