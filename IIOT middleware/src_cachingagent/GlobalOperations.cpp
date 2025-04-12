#include "GlobalOperations.h"

std::string GetLastToken ( std::string value, std::string seprator )
{
	std::size_t found = value.find_last_of( seprator );
	std::cout << " file: " << value.substr( found+1 ) << '\n';
	return value.substr( found+1 );
}

std::string ReadAndSetConfigurationInStr (std::string fileName )
{
	std::ifstream file(fileName.c_str());
	if ( file )
	{
		std::string fileContentString((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());
		std::cout << "configuration file content : " << fileContentString << std::endl;
		return fileContentString;
	}
	else
	{
		std::cout << "File Not found : " << fileName << std::endl;
	}
	return "";
}

bool WriteConfiguration( std::string fileName, nlohmann::json contentObj )
{
	std::ofstream outdata; 
	
	outdata.open( fileName.c_str() ); // opens the file
	if( !outdata )
	{ // file couldn't be opened
		std::cout  << "Error: file could not be opened" << std::endl;
		return false;
	}
	
	outdata << contentObj;
	outdata.close();
	return true;
}

nlohmann::json ReadAndSetConfiguration (std::string fileName )
{
	std::ifstream file(fileName.c_str());
	nlohmann::json configurationJson;
	if ( file )
	{
		std::string fileContentString((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());
		
		std::cout << "configuration file content : " << fileContentString << std::endl;
		try
		{
			configurationJson = nlohmann::json::parse(fileContentString.c_str());
			std::cout << "configuration file content : " << configurationJson << std::endl;
		}
		catch(nlohmann::json::exception &e)
		{
			std::cout << e.id << " : " << e.what() << std::endl;
		}
	}
	else
	{
		std::cout << "File Not found : " << fileName << std::endl;
	}
	return configurationJson;
}

long GetProcessIdByName( std::string processName )
{
	char line[100];
    std::string pidCommand = "pidof ";
    
    #ifdef KEMSYS_GATEWAY
        pidCommand += processName;
    #else
        pidCommand += "-s " + processName;
    #endif
    
    pid_t pid = 0;
	FILE *cmd = popen(pidCommand.c_str(), "r");
    if( cmd != NULL )
    {
        fgets(line, 100, cmd);
        pid = strtoul(line, NULL, 10);
        pclose(cmd);
    }
    return pid;
}

std::string GetTimeStamp()
{
    char buf[100];
    char buf1[100];
    int millisec;
    struct tm* tm_info;
    struct timeval tv;

    gettimeofday(&tv, NULL);

    millisec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
    if (millisec>=1000) 
    { 
        millisec -=1000;
        tv.tv_sec++;
    }
    
    tm_info = localtime(&tv.tv_sec);
    strftime(buf, 26, "%Y-%m-%dT%H:%M:%S", tm_info);
    sprintf(buf1,"%s.%03dZ", buf, millisec);
    return buf1;
}

std::string GetTimeStampInEpoch()
{
	struct timeval tp;
    gettimeofday(&tp, NULL);
    long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    
    char dt[100];
    sprintf(dt,"%ld",ms);
    std::string dateTime1 = dt;
	
	return dateTime1;
}

std::string GetCurrentDate()
{
	char strDate[80];
	time_t t = time(0);
    strftime(strDate, 80, "/%Y/%m/%d/", localtime(&t));
	return strDate;
}