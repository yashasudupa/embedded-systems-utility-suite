#include "FileUploadWrapper.h"

FileUploadWrapper::FileUploadWrapper( std::string appName, std::string gatewayId ):
	m_cloudAppName( appName ),
	m_gatewayId( gatewayId )
{
	m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
	m_deviceId = "";
	no_of_files = 0;
}

FileUploadWrapper::~FileUploadWrapper()
{
	
}

void FileUploadWrapper::no_of_cached_files(std::string jsonpath)
{
	std::stringstream logg;
	try
	{
		//std::lock_guard<std::mutex> lock(mtx);
		std::string filename = jsonpath + "fmgnmt.txt";
		std::string cmd = "ls " + jsonpath + " > " + filename;
		system(cmd.c_str());

		std::ifstream input_file(filename);
		std::string line;

		while(std::getline(input_file, line, '\n'))
		{
			no_of_files++;
		}
		no_of_files--;
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "FileUploadWrapper::no_of_cached_files() " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "FileUploadWrapper::no_of_cached_files() " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

void FileUploadWrapper::files_to_dynamic_array(std::string jsonpath, std::vector<CAL_DB> &dynamic_db_list)
{
	std::stringstream logg;
	try
	{
		//std::lock_guard<std::mutex> lock(mtx);
		std::string filename = jsonpath + "fmgnmt.txt";
		std::string cmd = "ls " + jsonpath + " > " + filename;
		system(cmd.c_str());

		std::ifstream input_file(filename);
		char *ch;
		std::string line;

		std::regex r_e("_[0-9.]+");
		std::smatch m;
		
		time_t curr_time;

		while(std::getline(input_file, line, '\n'))
		{
			std::cout << "line : " << line << std::endl;
			regex_search(line, m, r_e);

			std::string time_epoch = m.str();

			std::cout << "m.str() : " << time_epoch << std::endl;
			if(time_epoch.empty())
			{
				std::cout << "time_epoch.empty()" << std::endl;
				continue;
			}

			if(asset.empty() && line != "fmgnmt.txt")
			{
				//Legacy_Asset_Rel11_1661498878.json
				std::regex r_a("^(.*?)_[0-9]+");
				regex_search(line, m, r_a);
				asset = m.str();
				std::cout << "asset m.str() : " << asset << std::endl;
			}

			std::string trimmed_time_epoch =  time_epoch.substr(1, time_epoch.find(".") - 1);
			std::cout << "trimmed_time_epoch : " << trimmed_time_epoch.c_str() << std::endl;
			curr_time = std::stoi(trimmed_time_epoch, nullptr, 10);

			tm *tm_gmt = gmtime(&curr_time);

			CAL_DB dynamic_db;
			dynamic_db.calendar_epoch = trimmed_time_epoch;
			dynamic_db.calendar_tm = *tm_gmt;

			dynamic_db_list.push_back(dynamic_db);
		}
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "DataCacherManager::first_json_from_files() " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "DataCacherManager::first_json_from_files() " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief GetAndUploadFiles() :	This Method will get the files name from predefined path and call the 
 * 								FormatAndUploadFile() method.
 * 
 * @param std::string srcDirectory  : Directory path.
 */	
short FileUploadWrapper::GetAndUploadFiles( std::string srcDirectory, nlohmann::json datajson)
{	
	std::stringstream logg;
	try
	{
	
		//counter_t counter = {.hour = 10, .min = 30, .sec = 47};
		struct tm start_time = 
				{.tm_sec = 30,
				 .tm_min = 21,
				 .tm_hour = 17,
				 .tm_mday = 24,
				 .tm_mon = 7,
				 .tm_year = 192};
			
			
		time_t t_of_day_start = mktime(&start_time);
		
		struct tm end_time = 
				{.tm_sec = 25,
				 .tm_min = 51,
				 .tm_hour = 14,
				 .tm_mday = 5,
				 .tm_mon = 8,
				 .tm_year = 192,};
			
			
		time_t t_of_day_end = mktime(&end_time);

	
		if (start_time.tm_mday <= end_time.tm_mday ||
			start_time.tm_mon <= end_time.tm_mon||
			start_time.tm_year <= end_time.tm_year)
			{
				return -2;
			}
			
		no_of_cached_files(srcDirectory);
		
		std::string first_input;

		std::string second_input;
		
		//TODO
		files_to_dynamic_array(srcDirectory, cal_epoch_tm);
		
		std::vector<int> segments_and_range =
							{0, (no_of_files/4),
							no_of_files/4, (no_of_files/3),
							no_of_files/3, (no_of_files/2),
							no_of_files/2, no_of_files};
		
		std::vector<int>::iterator it;
		
		for (it = segments_and_range.begin(); it != segments_and_range.end(); ++it)
			{
				
			if(!(cal_epoch_tm[*it].calendar_tm.tm_mon <= start_time.tm_mon <= cal_epoch_tm[*(it+1)].calendar_tm.tm_mon
				&& (cal_epoch_tm[*it].calendar_tm.tm_mday <= start_time.tm_mday <= cal_epoch_tm[*(it+1)].calendar_tm.tm_mday)
				&& (cal_epoch_tm[*it].calendar_tm.tm_year <= start_time.tm_year <= cal_epoch_tm[*(it+1)].calendar_tm.tm_year)))
				{
					break;
				}
			}
		
		for(int i=*it; i<*(it+1); ++i)
		{

			// Find pos of
			// the start date 
			// in the array of the string
			
			if((cal_epoch_tm[i].calendar_tm.tm_mon >= start_time.tm_mon) 
				&& (cal_epoch_tm[i].calendar_tm.tm_mday >= start_time.tm_mday)
				&& (cal_epoch_tm[i].calendar_tm.tm_year >= start_time.tm_year))
			{
				first_input = cal_epoch_tm[i-1].calendar_epoch;
				break;
			} 
		}
		
		//If position does not
		//exist, report it
		//does not exist
		if(first_input.empty())
		{
			return -2;
		}
		
		
		for (;it != segments_and_range.end(); ++it)
			{
				
			if((cal_epoch_tm[*it].calendar_tm.tm_mon <= end_time.tm_mon <= cal_epoch_tm[*(it+1)].calendar_tm.tm_mon)
				&& (cal_epoch_tm[*it].calendar_tm.tm_mday <= end_time.tm_mday <= cal_epoch_tm[*(it+1)].calendar_tm.tm_mday)
				&& (cal_epoch_tm[*it].calendar_tm.tm_year <= end_time.tm_year <= cal_epoch_tm[*(it+1)].calendar_tm.tm_year))
				{
					break;
				}
			}
		
		for(int i=*it; i<*(it+1); ++i)
		{

			// Find pos of
			// end date in the array
			// of string
			if((cal_epoch_tm[i].calendar_tm.tm_mon >= end_time.tm_mon) 
				&& (cal_epoch_tm[i].calendar_tm.tm_mday >= end_time.tm_mday)
				&& (cal_epoch_tm[i].calendar_tm.tm_year >= end_time.tm_year))
			{
				second_input = cal_epoch_tm[i-1].calendar_epoch;
				break;
			}
			it = it+2;
		}
		
		//If position does not
		//exist, report it
		//does not exist
				
		if(second_input.empty())
		{
			return -2;
		}
		
		cal_epoch_tm.clear();
		
		std::string line;

		std::regex r_e("_[0-9.]+");
		std::smatch m;
		
		std::string first_input_check;
		std::string second_input_check;
		
		std::ifstream input_file(srcDirectory);
		
		while(std::getline(input_file, line))
		{
			std::cout << "line : " << line << std::endl;
			regex_search(line, m, r_e);

			std::string time_epoch = m.str();

			std::cout << "m.str() : " << time_epoch << std::endl;
			if(time_epoch.empty())
			{
				std::cout << "time_epoch.empty()" << std::endl;
				continue;
			}

			if(asset.empty() && line != "fmgnmt.txt")
			{
				//Legacy_Asset_Rel11_1661498878.json
				std::regex r_a("^(.*?)_[0-9]+");
				regex_search(line, m, r_a);
				asset = m.str();
				std::cout << "asset m.str() : " << asset << std::endl;
			}
			
			std::string trimmed_time_epoch =  time_epoch.substr(1, time_epoch.find(".") - 1);
			
			if(trimmed_time_epoch == first_input)
			{
				first_input_check = "Done";
			}
			
			if(!first_input_check.empty())
			{
				std::string filename = srcDirectory + asset + "_" +
										trimmed_time_epoch + ".json";
				
				std::string commandSchema = datajson[COMMAND_INFO][COMMAND_SCHEMA];
				
				std::string backup_msg = "backup/" + commandSchema;
				FormatAndUploadJson( filename, backup_msg );
				std::cout << "backup_msg :" << backup_msg << std::endl;
			}
			
			if(trimmed_time_epoch == second_input)
			{
				break;
			}
			
			//std::cout << "trimmed_time_epoch : " << trimmed_time_epoch.c_str() << std::endl;
			//curr_time = std::stoi(trimmed_time_epoch, nullptr, 10);
		}
	}
	catch( ... )
	{
		logg.str("");
		logg << "FileUploadWrapper::ValidateFileContent()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	//return status;
}

/**
 * @brief UploadFiles() :	This thread will get the files from predefined path and upload the  
 * 							Cached telemetry and alert files.
 * 
 */	
short FileUploadWrapper::UploadFiles(std::string dataJson, std::string dataPath)
{
	std::stringstream logg;
	try
	{
		std::thread uploadThread = std::thread([this, dataPath, dataJson]()
		{
			//get list of files and upload Cached Telemetry
			GetAndUploadFiles( dataPath, dataJson);
		});
		
		uploadThread.detach();
	}
	catch( ... )
	{
		logg.str("");
		logg << "FileUploadWrapper::UploadFiles()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}


/**
 * @brief UploadFiles() :	This thread will get the files from predefined path and upload the  
 * 							Cached telemetry and alert files.
 * 
 */	
void FileUploadWrapper::UploadFiles()
{
	std::stringstream logg;
	try
	{
		std::thread uploadThread = std::thread([this] ()
		{
			//get list of files and upload Cached Telemetry
			GetAndUploadFiles( CACHED_TELEMERTY_PATH );
			GetAndUploadFiles( CACHED_ALERT_PATH );
		});
		
		uploadThread.detach();
	}
	catch( ... )
	{
		logg.str("");
		logg << "FileUploadWrapper::UploadFiles()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

void FileUploadWrapper::process_cached_files(std::string path, std::string srcDirectory, std::string filename)
{
	char filename_str[50];
	char *p, *q;

	std::string LSCMD = "ls " + path + " >> " + path + "cached_" + filename + "_names.txt";
	system( LSCMD.c_str() );

	//LSCMD = "ls /opt/IoT_Gateway/CachedData/Alert >> alert_file_names.txt";
	//system( LSCMD.c_str() );

	std::string readFile = path + "cached_" + filename + "_names.txt";
	
	std::cout << "FileUploadWrapper::process_cached_files - readFile :" << readFile << std::endl;
	
	std::string fileContent = ReadAndSetConfigurationInStr( readFile );

	std::string rmCMD = "rm " + readFile;
	system( rmCMD.c_str() );

	int lnt = fileContent.length();
	char char_array[500] = {0};

	strcpy(char_array, fileContent.c_str());
	p = q = char_array;
	while (p = strstr(p,".json" ))
	{
		p = p + strlen(".json\n");
		memset(filename_str,0,sizeof(filename_str));
		memcpy(filename_str,q,p-q-1);
		filename_str[p-q-1] = '\0';
		q = p;
		
		std::string filepath = srcDirectory + filename_str;
		
		FormatAndUploadJson( filepath, filename_str );
	}
}


/**
 * @brief GetAndUploadFiles() :	This Method will get the files name from predefined path and call the 
 * 								FormatAndUploadFile() method.
 * 
 * @param std::string srcDirectory  : Directory path.
 */	
bool FileUploadWrapper::GetAndUploadFiles( std::string srcDirectory )
{
	std::stringstream logg;
	std::list<std::string> lstFileNames;
	bool status = false;
	
	try
	{
		struct dirent **hFile;
		int n,i=0;
		
		#ifdef TRB_GATEWAY
			process_cached_files(CACHED_TELEMERTY_PATH, srcDirectory, "Telemetry");
			process_cached_files(CACHED_ALERT_PATH, srcDirectory, "Alert");

		#else

		n = scandir( srcDirectory.c_str(), &hFile, 0, alphasort );
	   
		if (n < 0)
		{
		   perror("scandir");
		}
		else 
		{
			while ( (i < n) )
			{
				if (!strcmp( hFile[i]->d_name, "." ) || !strcmp( hFile[i]->d_name, ".." )) 
				{
					i++;
					continue;
				}
				
				if ((strstr(hFile[i]->d_name,".json")))
				{
					char name[200];
					sprintf(name,"%s",(hFile[i]->d_name));
					std::string filePath = srcDirectory + name; 
					FormatAndUploadJson( filePath, name );
					status = true;
					free( hFile[i] );
				}
				i++;
				usleep(100);
			}
		}
		free(hFile);
		#endif
	}
	catch( ... )
	{
		logg.str("");
		logg << "FileUploadWrapper::ValidateFileContent()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
		lstFileNames.empty();
	}
	return status;
}

/**
 * @brief GetAndUploadFiles() :	This Method will read the files and add the required json content,
 * 								publish the json to CommunicationApp and delete the file.
 * 
 * @param std::string fileNameWithPath  : full path with filename.
 * @param std::string fileName          : only filename.
 */	
void FileUploadWrapper::FormatAndUploadJson( std::string fileNameWithPath, std::string fileName )
{
	std::stringstream logg;
	try
	{
		nlohmann::json jsonObj1;
		jsonObj1 = ReadAndSetConfiguration( fileNameWithPath );
		if( !jsonObj1.is_null() )
		{
            std::string destFilePath = "telemetry/" ;
			destFilePath += jsonObj1[DEVICE_ID];
			destFilePath += GetCurrentDate() + fileName ;
			jsonObj1[COMMAND_INFO][COMMAND_TYPE] = CACHED_DATA;
			jsonObj1[COMMAND_INFO][FILE_NAME] = destFilePath;
			
			#ifdef TRB_GATEWAY
				
			std::string remove_logfile = "rm " + fileNameWithPath;
			if( system( remove_logfile.c_str()) == 0)
			{
				logg.str("");
				logg << "FileUploadWrapper::FormatAndUploadJson - " << fileNameWithPath << "' file deleted successfully.";
				m_exceptionLoggerObj->LogInfo( logg.str() );
			}
			else
			{
				logg.str("");
				logg << "FileUploadWrapper::FormatAndUploadJson - " << fileNameWithPath << "' file deleted failed.";
				m_exceptionLoggerObj->LogError( logg.str() );
			}
			
			#else
			if( unlink( fileNameWithPath.c_str() ) != 0 )
			{
				logg.str("");
				logg << "FileUploadWrapper::FormatAndUploadJson  GatewayID : " << m_gatewayId << ",  Message : File can not be deleted. File Name : " << fileName ;
				m_exceptionLoggerObj->LogError( logg.str() );
			}
			
			#endif
			
			m_cachedDataCB( jsonObj1 );
		}
		else
		{
			if( unlink( fileNameWithPath.c_str() ) == 0 )
			{
				logg.str("");
				logg << "FileUploadWrapper::FormatAndUploadJson  GatewayID : " << m_gatewayId << ",  Message : Delete empty file. File Name : " << fileName ;
				m_exceptionLoggerObj->LogInfo( logg.str() );
			}
		}
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "FileUploadWrapper::FormatAndUploadJson  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "FileUploadWrapper::FormatAndUploadJson  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

void FileUploadWrapper::RegisterCachedDataCB( std::function<void(nlohmann::json)> cb )
{
	m_cachedDataCB = cb;
}