#include "AppHealthMonitoring.h"

/**
 * @brief Create an AppHealthMonitoring	:	register current process.
 */
AppHealthMonitoring::AppHealthMonitoring( std::string processName ):
	m_prviousDate ( "" ),
	m_appHealthMoniThreadFlag( true )
{
	m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
	m_sendMsgToCloudObj = m_sendMsgToCloudObj->GetInstance( NULL );
	std::string currentProcessName = GetLastToken( processName, "/" );
	RegisterApp( currentProcessName );
}

/**
 * @brief Create an AppHealthMonitoring	:	register current process.
 */
AppHealthMonitoring::~AppHealthMonitoring()
{
	m_appHealthMoniThreadFlag = false;
	m_threadObj.join();
}

void AppHealthMonitoring::StartHealthInfoGetterThread()
{
	m_threadObj = std::thread([this](){
		while( m_appHealthMoniThreadFlag )
		{
			if( m_appHealthMap.empty() )
			{
				sleep(120);
				continue;
			}
			 nlohmann::json jsonObj;
			 for( auto it : m_appHealthMap )
			 {
				it.second->app_ram_usage = GetRamUsage( it.first );
				it.second->app_cpu_usage = GetCpuUsage( it.first );
                std::string ramUsageStr = it.first + " RAM Usage";
                std::string cpuUsageStr = it.first + " CPU Usage";
				jsonObj[CONFIGURATION][ramUsageStr] = it.second->app_ram_usage;
				jsonObj[CONFIGURATION][cpuUsageStr] = it.second->app_cpu_usage;
			 }
			 
			 if( !jsonObj.is_null() )
			 {
				char memoryStr[100];
				std::stringstream logg;
				sprintf(memoryStr, "%d%%", GetDiskFreeSpace());
				jsonObj[CONFIGURATION][MEMORY_AVAILABLE] = memoryStr ;
				jsonObj[TYPE] = "notification";
				jsonObj[MESSAGE] = "Gateway has reported heartbeat";
				jsonObj[EVENT] = "heartbeat";
				jsonObj[TIMESTAMP] = GetTimeStamp();
				 
				std::string notificationStr = jsonObj.dump();
				 
				logg.str("");
				logg << "AppHealthMonitoring::StartHealthInfoGetterThread()  Message : App Health Json : " << notificationStr;
				m_exceptionLoggerObj->LogInfo( logg.str() );
				m_sendMsgToCloudObj->SendMessageToCloud( notificationStr, "notification" );
			 }
			 
			for(int i=0;i<30;i++)
			{
				sleep(120);
			}
		}});
}

std::string AppHealthMonitoring::GetRamUsage( std::string processname )
{
    char path[1035];
    try
    {
        FILE *fp;
        //char path[1035];
        
        if( processname.length() > 15 )
        {
            processname = processname.substr(0,15);
        }
        
        std::string name = "";
        #ifdef KEMSYS_GATEWAY
            name = "top -b -n2 -d0.1 | grep \"" + processname + "\" | grep -v grep | tail -n 1 | awk '{print $6}'";
        #else
            name = "ps -o pmem -C " + processname;
        #endif
        
        fp = popen(name.c_str(), "r");
        if (fp == NULL) 
        {
            return "";
        }
        
        while (fgets(path, sizeof(path), fp) != NULL)
        {
            sleep(1);
            
            #ifdef KEMSYS_GATEWAY
                std::string result = path;
                result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
                return result;
            #else
                if(strstr(path,"%MEM"))
                {
                    continue;
                } 
                double tempRamUsage = atof( path );
                sprintf(path,"%.2f%%",tempRamUsage );
                return path;
            #endif
        }
        pclose(fp);
    }
    catch ( ... )
    {
        std::cout << "AppHealthMonitoring::GetRamUsage() Exception occured\n\n";
    }
	return "";
}

std::string AppHealthMonitoring::GetCpuUsage( std::string processname )
{
    std::string path1 = "";
    try
    {
        FILE *fp;
        char path[1035];
        std::string cpuUsage = "";
        if( processname.length() > 15 )
        {
            processname = processname.substr(0,15);
        }
        
        std::string name = "";
        #ifdef KEMSYS_GATEWAY
            name = "top -b -n2 -d0.1 | grep \"" + processname + "\" | grep -v grep | tail -n 1 | awk '{print $7}'";
        #else
            name = "ps -o pcpu -C " + processname;
        #endif
        
        fp = popen( name.c_str(), "r" );
        if (fp == NULL) 
        {
            return "";
        }
        
        while (fgets(path, sizeof(path), fp) != NULL)
        {
            sleep(1);
            #ifdef KEMSYS_GATEWAY
                std::string result = path;
                result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
                return result;
            #else
                if(strstr(path,"%CPU"))
                {
                    continue;
                }
                
                double tempCpuUsage = atof( path );
                sprintf(path,"%.2f%%",tempCpuUsage );
                return path;
            #endif
        }
        pclose(fp);
        
    }
    catch ( ... )
    {
        std::cout << "AppHealthMonitoring::GetCpuUsage() Exception occured\n\n";
    }
	return "";
}

//function calculates the available disk free space in percentage
int AppHealthMonitoring::GetDiskFreeSpace()
{
	int freeSpaceInPer = -1;
	struct statvfs buf;
	statvfs ( SYSTEM_PATH, &buf );
	char disklog[500];
	unsigned long blksize, blocks, freeblks, disk_size, used, free;
	 
	blksize = buf.f_bsize;
	blocks = buf.f_blocks;
	freeblks = buf.f_bfree;
	 
	disk_size = (blocks * blksize)/1024;
	free = (buf.f_bavail* blksize)/1024;
	used = disk_size - free;

	freeSpaceInPer = (int)((free*100)/disk_size);
	
	return freeSpaceInPer;
}


void AppHealthMonitoring::RegisterApp( std::string processname )
{
	auto it = m_appHealthMap.find( processname );
	if (it == m_appHealthMap.end())
	{
		APPHEALTH *appHealthObj = new APPHEALTH;
		m_appHealthMap[processname] = appHealthObj;
	}
}

void AppHealthMonitoring::DeRegisterApp( std::string processname )
{
	auto it = m_appHealthMap.find( processname );
	if (it != m_appHealthMap.end())
	{
		m_appHealthMap.erase (it);
	}
}
