#include "GlobalOperations.h"

std::string GetLastToken ( std::string value, std::string seprator )
{
    // /opt/IoT_Gateway/GatewayAgent/GatewayAgent
	std::cout << " GlobalOperations:: GetLastToken - file: " << value << std::endl;
	std::size_t found = value.find_last_of( seprator );
	std::cout << " GlobalOperations:: GetLastToken - file: " << found << std::endl;
	if( found != std::string::npos )
	{
		value = value.substr( found+1 );
	}
	std::cout << " GlobalOperations:: GetLastToken - file: " << value << std::endl;
	return value;
}

// /opt/IoT_Gateway/GatewayAgent/config/persistency_config/gateway_persistency_config.json
nlohmann::json ReadAndSetConfiguration (std::string fileName )
{
    nlohmann::json configurationJson;
    try
    {
        std::ifstream file( fileName.c_str() );
        if ( file )
        {
            std::string fileContentString((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
            
            std::cout << "file Content String configuration : " << fileContentString << std::endl;
            try
            {
                configurationJson = nlohmann::json::parse( fileContentString.c_str() );
                std::cout << "Json configuration file content : " << configurationJson << std::endl;
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
    }
    catch( ... )
	{
		std::cout <<"ReadProperties::ReadAndSetConfiguration : Unknown Exception occured." << std::endl;
	}
    
	return configurationJson;
}

std::string ReadAndSetConfigurationInStr( std::string fileName )
{
    try
    {
        std::ifstream file(fileName.c_str());
        if ( file )
        {
            std::string fileContentString((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
            //std::cout << "configuration file content : " << fileContentString << std::endl;
            file.close();
            return fileContentString;
        }
        else
        {
            std::cout << "File Not found : " << fileName << std::endl;
        }
    }
    catch( ... )
	{
		std::cout <<"ReadProperties::ReadAndSetConfigurationInStr : Unknown Exception occured." << std::endl;
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
        // Allow for rounding up to nearest second
        millisec -=1000;
        tv.tv_sec++;
    }
    
    tm_info = localtime(&tv.tv_sec);
    strftime(buf, 26, "%Y-%m-%dT%H:%M:%S", tm_info);
    sprintf(buf1,"%s.%03dZ", buf, millisec);
    return buf1;
}

std::string GetCurrentDateStr()
{
	time_t CurrTime;
	time( &CurrTime );
	struct tm *TimeStruct = localtime( &CurrTime );

	char DateStr[12];
	// create string
	strftime( DateStr, 11, "%d-%m-%Y", TimeStruct );
	std::string str = DateStr;
	return str;
}

std::string GetCurrentDate()
{
	char strDate[80];
	time_t t = time(0);
    strftime(strDate, 80, "/%Y/%m/%d/", localtime(&t));
	return strDate;
}

tm make_tm(int year, int month, int day)
{
    tm tm = {0};
    tm.tm_year = year - 1900; // years count from 1900
    tm.tm_mon = month - 1;    // months count from January=0
    tm.tm_mday = day;         // days count from 1
    return tm;
}


std::string GenerateCurrentDateStr()
{
	time_t CurrTime;
	time( &CurrTime );
	struct tm *TimeStruct = localtime( &CurrTime );

	char DateStr[12];
	// create string
	strftime( DateStr, 11, "%d-%m-%Y", TimeStruct );
	std::string str = DateStr;
	return str;
}