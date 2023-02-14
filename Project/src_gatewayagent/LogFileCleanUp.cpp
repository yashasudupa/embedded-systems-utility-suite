#include "LogFileCleanUp.h"

LogFileCleanUp::LogFileCleanUp( std::string gatewayId ):
        m_ThreadRunning( true ),
        m_logCleanUpThreadFlag( true ),
        m_logCleanUpTimeCount( 3600 ),
        m_cleanUpCount( 3600 ),
        m_gatewayId( gatewayId )
{
    m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
    m_sendMsgWrapperObj = m_sendMsgWrapperObj->GetInstance( NULL );
    StartLogCleanUpThread();
}

LogFileCleanUp::~LogFileCleanUp()
{
    m_ThreadRunning = false;
    m_logCleanUpThreadFlag = false;
}

void LogFileCleanUp::StartLogCleanUpThread()
{
        m_threadObj = std::thread([this](){
		while( m_ThreadRunning )
		{
			if( m_logCleanUpThreadFlag )
			{
                m_currentDate = GenerateCurrentDateStr();
                if( m_currentDate != m_previousDate )
                {
                    m_previousDate = m_currentDate; 
                    CheckAndCleanLogFiles();
                }
			}
            
            while( m_logCleanUpTimeCount >= m_cleanUpCount )
            {
                m_cleanUpCount++;
                sleep(1);
            }
			m_cleanUpCount = 0;
		}
	});
}

void LogFileCleanUp::CheckAndCleanLogFiles()
{
    std::list<std::string> lstFileNames = getListOfFiles();
    auto itr =lstFileNames.begin();
    std::stringstream logg;
    while( itr != lstFileNames.end() )
    {
        std::string fileName = *itr;
        StringParser strParsr;
        strParsr.init( fileName,"_",ESCSEQ );
        std::string t1,t2,t3;
        strParsr.getNext3Tokens(t1,t2,t3,true);
        size_t found = t3.find(".");
        if ( found != std::string::npos )
        {
            std::cout << "First occurrence is " << found << std::endl;
            std::string fileDate  = t3.substr( 0,found );
            std::cout << "t3 : " << fileDate << "\n\n";
            
            struct tm tm1;
            time_t t1;
            
			//std::cout << "fileDate before erase : " << fileDate << std::endl;
			
            fileDate.erase(std::remove(fileDate.begin(),fileDate.end(),'-'),fileDate.end());
			
			//std::cout << "fileDate after erase : " << fileDate << std::endl;
			
            sscanf(fileDate.c_str(),"%2d%2d%4d",&tm1.tm_mday,&tm1.tm_mon,&tm1.tm_year);
			
			//std::cout << "fileDate today : " << fileDate << std::endl;
			
            //get no. of days between current date & date of file
            tm1 = make_tm(tm1.tm_year,tm1.tm_mon,tm1.tm_mday);
            t1 = mktime(&tm1);
            
            struct tm tm2;
            time_t t2;
            std::string subString = GenerateCurrentDateStr();
			
			//std::cout << "Current date subString : " << subString << std::endl;
			
            subString.erase(std::remove(subString.begin(),subString.end(),'-'),subString.end());
            sscanf(subString.c_str(),"%2d%2d%4d",&tm2.tm_mday,&tm2.tm_mon,&tm2.tm_year);
            //get no. of days between current date & date of file
            tm2 = make_tm(tm2.tm_year,tm2.tm_mon,tm2.tm_mday);
            t2 = mktime(&tm2);
            
            double diff = difftime(t2, t1) / (60 * 60 * 24);
            std::string readFilepath = LOG_FILE_PATH;
			
			
			
            readFilepath += "/" + fileName;
         
			//std::cout << "diff : " << diff << std::endl;
			//std::cout << "readFilepath : " << readFilepath << std::endl;
   
            if( diff >= NUMBER_OF_DAYS_TO_DELETE )
            {
                if( m_sendMsgWrapperObj )
                {
                    std::string fileContent = ReadAndSetConfigurationInStr( readFilepath );
					
					
					logg.str("");
					logg << "LogFileCleanUp::CheckAndCleanLogFiles() - fileContent :'" << fileContent;
					std::cout << "-----LogFileCleanUp::CheckAndCleanLogFiles() - fileContent :'" << fileContent << std::endl;
					m_exceptionLoggerObj->LogInfo( logg.str() );
					
					
                    //Example Path : logs/2021/09/16/CachingAgent_Log_15-09-2021.log
                    std::string destFilePath =  "logs" + GetCurrentDate() + fileName;
					
					
					logg.str("");
					logg << "LogFileCleanUp::CheckAndCleanLogFiles() - destFilePath :'" << destFilePath;
					std::cout << "-----LogFileCleanUp::CheckAndCleanLogFiles() - destFilePath :'" << destFilePath << std::endl;
					m_exceptionLoggerObj->LogInfo( logg.str() );
					
					
					
					logg.str("");
					logg << "LogFileCleanUp::CheckAndCleanLogFiles() - '" << readFilepath << "Uploading to cloud";
					std::cout << "-----LogFileCleanUp::CheckAndCleanLogFiles() - readFilepath :'" << readFilepath << std::endl;
					m_exceptionLoggerObj->LogInfo( logg.str() );
					
                    m_sendMsgWrapperObj->UploadFilesToCloud( fileContent, destFilePath );
                }
                
				#ifdef TRB_GATEWAY
				
				std::string remove_logfile = "rm " + readFilepath;
				if( system( remove_logfile.c_str()) == 0)
				{
					logg.str("");
					logg << "LogFileCleanUp::CheckAndCleanLogFiles() 1 st- '" << readFilepath << "' file deleted successfully.";
					m_exceptionLoggerObj->LogInfo( logg.str() );
                    
					
				}
				else
				{
					logg.str("");
					logg << "LogFileCleanUp::CheckAndCleanLogFiles() - '" << readFilepath << "' file deleted failed.";
					m_exceptionLoggerObj->LogError( logg.str() );
				}
				
				#else
                if( unlink( readFilepath.c_str() ) == 0 )
                {
                    logg.str("");
                    logg << "LogFileCleanUp::CheckAndCleanLogFiles() - last'" << readFilepath << "' file deleted successfully.";
                    m_exceptionLoggerObj->LogInfo( logg.str() );
                }
				#endif
            } 
        }
        itr++;
        sleep(5);
    }
}

void LogFileCleanUp::SetLogCleanUpTime( long cleanupTime )
{
    m_logCleanUpTimeCount = cleanupTime;
}

void LogFileCleanUp::StartThread( bool startFlag )
{
    m_logCleanUpThreadFlag = startFlag;
}

//get list of files from Directory
std::list<std::string> LogFileCleanUp::getListOfFiles()
{
	std::list<std::string> lstFileNames;
	std::string Directory="";

    struct dirent **hFile;
    int n,i=0;
	
	#ifdef TRB_GATEWAY
    
	char filename_str[50];
	char *p, *q;

	std::string LSCMD = "ls /opt/IoT_Gateway/GatewayAgent/logs >> /opt/IoT_Gateway/GatewayAgent/logs.txt";
	system( LSCMD.c_str() );

	std::string readFile = "/opt/IoT_Gateway/GatewayAgent/logs.txt";
	std::string fileContent = ReadAndSetConfigurationInStr( readFile );

	std::string rmCMD = "rm /opt/IoT_Gateway/GatewayAgent/logs.txt";
	system( rmCMD.c_str() );

	int lnt = fileContent.length();
	char char_array[500] = {0};

	strcpy(char_array, fileContent.c_str());
	//std::cout << "char_array : \n" << char_array << std::endl;
	p = q = char_array;
	//std::cout << "filename_str : \n" ;
	while (p = strstr(p,".log" ))
	{
		p = p + strlen(".log\n");
		memset(filename_str,0,sizeof(filename_str));
		memcpy(filename_str,q,p-q-1);
		//std::cout << "p-q : " << p-q-1 << std::endl;
		filename_str[p-q-1] = '\0';
		q = p;
		std::cout << filename_str << std::endl;
		lstFileNames.push_back(filename_str);
	}
 
	#else
    //get no of files in Directory
    n = scandir( LOG_FILE_PATH, &hFile, 0, alphasort );
       
    if (n < 0)
    {
      perror("scandir");
    }
    else 
    {
        // we send 50 files at a time throught FTP so we get only 50 files from such directory
        // and add to the list;
       while (n--) 
        {
            if( i < 50 )
            {
                if (!strcmp(hFile[i]->d_name, ".") || !strcmp(hFile[i]->d_name, "..")) 
                {
                    free(hFile[i]);
                    i++;
                    continue;
                }
                
                if ( strstr(hFile[i]->d_name,".log" ) )
                {
                    char name[50];
                    sprintf(name,"%s",(hFile[i]->d_name));
                    lstFileNames.push_back(name);
                    
                    std::cout << "add in list FileName : " << name << "\n\n";
                }
            }
            free(hFile[i]);
            i++;
        }
        
        free(hFile);
    }
	#endif
	return lstFileNames;
}