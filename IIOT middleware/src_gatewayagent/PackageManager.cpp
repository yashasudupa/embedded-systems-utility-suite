#include "PackageManager.h"

/**
 * @brief Create an PackageManager	:	It will call InitPackageManager Method.
 * 
 * @param std::string gatewayId 	:	it contains gateway id.
 * @param std::string cloudAppName 	:	it contains cloud app name which is defined in cloud.
 */
PackageManager::PackageManager( std::string gatewayId, std::string cloudAppName, std::string processName ):
	m_gatewayId( gatewayId ),
	m_cloudAppName( cloudAppName ),
	m_processName( processName ),
    m_appHealthMonitoringObj( NULL )
{
	m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
	InitPackageManager();
}

/**
 * @brief destroy an PackageManager	:	It will deinitilize the AppHealthMonitoring and PersistencyManager.
 * 										register call backs
 */
PackageManager::~PackageManager()
{
	if( m_appHealthMonitoringObj )
	{
		delete m_appHealthMonitoringObj;
	}
	
	if( m_persistencyManagerObj )
	{
		delete m_persistencyManagerObj;
	}
}

/**
 * @brief InitPackageManager	:	It will initilize the AppHealthMonitoring and PersistencyManager.
 * 									register call backs
 * 
 */
void PackageManager::InitPackageManager()
{
	std::stringstream logg;
	try
	{
		m_watchDogObj = m_watchDogObj->GetInstance();
		m_sendMsgWrapperObj = m_sendMsgWrapperObj->GetInstance( NULL );
		
		m_appHealthMonitoringObj = new AppHealthMonitoring( m_processName );
		if( m_appHealthMonitoringObj )
		{
			logg.str("");
			logg << "PackageManager::InitGatewayAgentManager()  GatewayID : " << m_gatewayId << ",  Message : AppHealthMonitoring object created Successfully.";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			m_appHealthMonitoringObj->StartHealthInfoGetterThread();
		}
		else
		{
			logg.str("");
			logg << "PackageManager::InitGatewayAgentManager()  GatewayID : " << m_gatewayId << ",  Message : AppHealthMonitoring object creation failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
		
		m_persistencyManagerObj = new PersistencyManager();
		if( m_persistencyManagerObj )
		{
			logg.str("");
			logg << "PackageManager::InitGatewayAgentManager()  GatewayID : " << m_gatewayId << ",  Message : PersistencyManager object created Successfully.";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			nlohmann::json persistentJson;
			persistentJson = m_persistencyManagerObj->MaintainPersistency(m_exceptionLoggerObj);
			
			logg.str("");
			logg << "PackageManager::InitGatewayAgentManager()  persistentJson : " << persistentJson << ", appjson check";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			
			ReceivePersistencyCommand( persistentJson );
		}
		else
		{
			logg.str("");
			logg << "PackageManager::InitGatewayAgentManager()  GatewayID : " << m_gatewayId << ",  Message : PersistencyManager object creation failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
	}
	catch( ... )
	{
		logg.str("");
		logg << "PackageManager::InitGatewayAgentManager()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief InstallPackage() 			:	This method will download the package from cloud and unzip  
 *										the package in predefined path.
 * 
 * @param nlohmann::json jsonObj	:	It install package information like url, token, app_name, version
 *
 * @return : It will return SUCCESS if package installed successfully otherwise return predefined errors codes.
 */
ResponseStruct PackageManager::InstallPackage ( nlohmann::json jsonObj )
{
	ResponseStruct responseStructObj;
	std::stringstream logg;
	

	try
	{
		if(jsonObj== NULL)
		{
			responseStructObj.status = EMPTY_JSON;
			return responseStructObj;
		}

		
		std::string appName = jsonObj[APP_NAME];
		m_sendMsgWrapperObj->SendReportedJsonToCloud( jsonObj, REPORT_DOWNLOADING );
		AddDataIntoMap( jsonObj, INSTALL );
		
		if( DownloadPackage( jsonObj ) == SUCCESS )
		{
			m_sendMsgWrapperObj->SendReportedJsonToCloud( jsonObj, REPORT_APPLYING );
			
			if( UnzipPackage( jsonObj ) == SUCCESS )  //existing installation overwritten here.
			{
				// validate, dowloaded and zipped, app
				
		         auto it = m_installedPackageInfoMap.find(appName); 
		
					std::string ValidateAppCmd = " ";
					m_persistentJson[appName]["packagename"] = it->second->packageName;
					std::string appPath = INSTALLPATH + appName + "/";
		
				      if ( it != m_installedPackageInfoMap.end() )
		              {		
			             std::string downloadDir = it->second->appDownloadPath;
			             downloadDir += it->second->packageName;
						 int appisValid = -1,applibValid = -1,appInstallValid = -1,pkgcfgvalid = -1;;
						if((appName == "CachingAgent")||(appName == "MQTTAgent")||(appName == "GatewayAgent"))
						{
							    //invalid operation, for system app's
								responseStructObj.status = FAILURE;
								responseStructObj.responseMetaInformation = appName + " System apps package install: invalid request";
								logg.str("");
								logg << "PackageManager::InstallPackage()  GatewayID : " << m_gatewayId << ",  Message : Invalid command " << appName << " package install failed.";
								m_exceptionLoggerObj->LogError( logg.str() );
								return responseStructObj;

		                   }else{
							   
							   //custom appp, check it directory present, if so, check install path for app and cleanit and cp app dir.
							   ValidateAppCmd = "ls " + downloadDir  + "/" + appName ; //devicemanger
							   
							   appisValid = system( ValidateAppCmd.c_str() );
							   
							   ValidateAppCmd = "ls " + downloadDir +  "/" + "libs/" + "lib" + appName + ".so" ; //library
							   
							   applibValid = system( ValidateAppCmd.c_str() );
							   
							   std::string InstallCmd = " ";
								   //cp director from, temp to install path ie /opt/IoT_Gateway
								  
							   InstallCmd = "cp -f -R " + downloadDir  + " " + INSTALLPATH;
							   
							   appInstallValid = system( InstallCmd.c_str() );
							   
							   std::string packageConfigFileName = downloadDir + "/"+ PACKAGE_CONFIG_JSON;
							   
							   std::string pkgcfgCmdvalid = "ls " + packageConfigFileName;
							   
							   pkgcfgvalid = system( pkgcfgCmdvalid.c_str() );
							   
							   std::cout<<"\napplibValid\n"<<applibValid;
							   std::cout<<"\nappisValid\n"<<appisValid;
							   std::cout<<"\naInstallCmd\n"<<InstallCmd<<"\n";
							   std::cout<<"\nappInstallValid\n"<<appInstallValid<<"\n";
							   
							   
							   //if((appisValid == 0) && (applibValid == 0)&& (appInstallValid == 0)&& (pkgcfgvalid == 0))
							   if((appisValid == 0) && (appInstallValid == 0))
							   {
								   //std::string packageConfigFileName = " ";
			                       nlohmann::json fileContentJson = " ";
								   std::string InstallCmd = " ";
								   //cp director from, temp to install path ie /opt/IoT_Gateway
								  
									//InstallCmd = "cp -f " + downloadDir + appName + " " + INSTALLPATH;
									packageConfigFileName = appPath + PACKAGE_CONFIG_JSON; //"package_config.json"
								    fileContentJson = ReadAndSetConfiguration( packageConfigFileName );
					   
      				                it->second->appJson = fileContentJson;
								    UpdateLibFiles( fileContentJson, appPath );
									
                                    m_persistentJson[appName]["app_install_path"] = appPath;
									m_persistentJson[appName]["state"] = INSTALLED;
									m_persistentJson[appName]["app_json"] = fileContentJson;
									m_persistencyManagerObj->SetConfiguration( m_persistentJson );
									m_sendMsgWrapperObj->SendReportedJsonToCloud( jsonObj, REPORT_INSTALLED );
									
									// remove unzipped packages
									std::string removezipcmd = "rm -f -r ";
									removezipcmd += downloadDir;
									if(0 != system(removezipcmd.c_str() ))
									std::cout<<"\nremoved unzip,fies:failed\n";
									
									
													   
							   }else
							   {
								   
								    // remove unzipped packages, failed installations files
									std::string removefailedinstallcmd = "rm -f -r ";
									removefailedinstallcmd += appPath;
									if(0 != system(removefailedinstallcmd.c_str() ))
									std::cout<<"\nremoved failed install,fies:failed\n";
									
								    m_installedPackageInfoMap.erase (it);
									m_persistentJson.erase(m_persistentJson.find(appName));
									m_persistencyManagerObj->SetConfiguration( m_persistentJson );
								    responseStructObj.status = FAILURE;
									responseStructObj.responseMetaInformation = appName + " package install failed: incorrect package format";
									logg.str("");
									logg << "PackageManager::InstallPackage()  GatewayID : " << m_gatewayId << ",  Message : Unzip " << appName << " package failed.";
									m_exceptionLoggerObj->LogError( logg.str() );
									// remove unzipped packages
									std::string removezipcmd = "rm -f -r ";
									removezipcmd += downloadDir;
									if(0 != system(removezipcmd.c_str() ))
									std::cout<<"\nremoved unzip,fies:failed\n";
									
									return responseStructObj;
								   
							   }
							   
						   }
					  }
				
				nlohmann::json installedAppInfoJson;
				installedAppInfoJson[INSTALLED_PACKAGES][appName] = jsonObj["version"];
                installedAppInfoJson[SUB_JOB_ID] = jsonObj[SUB_JOB_ID];
                installedAppInfoJson[CUSTOM_APPS][appName][STATUS] = "Not Running";
				m_sendMsgWrapperObj->SendReportedJsonToCloud( installedAppInfoJson, REPORT_GENERIC );
				responseStructObj.status = SUCCESS;
				responseStructObj.responseMetaInformation = appName + " package installed successfully.";
				
				return responseStructObj;
			}
			else
			{
				responseStructObj.status = FAILURE;
				responseStructObj.responseMetaInformation = appName + " package install failed.";
				logg.str("");
				logg << "PackageManager::InstallPackage()  GatewayID : " << m_gatewayId << ",  Message : Unzip " << appName << " package failed.";
				m_exceptionLoggerObj->LogError( logg.str() );
				return responseStructObj;
			}
		}
		else
		{
			responseStructObj.status = FAILURE;
			responseStructObj.responseMetaInformation = appName + " package download failed.";
			logg.str("");
			logg << "PackageManager::InstallPackage()  GatewayID : " << m_gatewayId << ",  Message : Download " << appName << " package failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
			return responseStructObj;
		}
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "PackageManager::InstallPackage()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "PackageManager::InstallPackage()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured. JSON : " << jsonObj;
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	responseStructObj.status = FAILURE;
	responseStructObj.responseMetaInformation = "Unknown Exception occured.";
	return responseStructObj;
}

/**
 * @brief UpgradePackage() 			:	This method will validate the received version with current version.
 * 										if both version are different then it will stop the application and 
 * 										delete the old installed package and download the updated package
 *										from cloud and unzip the package in predefined path.
 * 										
 *										
 * @param nlohmann::json jsonObj	:	It contains Upgrade package information like url, token, app_name, version
 *
 * @return : It will return SUCCESS if package Upgraded successfully otherwise return predefined errors codes.
 */
ResponseStruct PackageManager::UpgradePackage( nlohmann::json jsonObj )
{
	ResponseStruct responseStructObj;
	std::stringstream logg;
	try
	{
		bool retVal = false;
		std::string version = jsonObj["version"];
		std::string appName = jsonObj[APP_NAME];
		nlohmann::json startApplicationJson;
		
		if( appName == "GatewayAgent" )
		{
			//responseStructObj = UpdateGatewayAgent( jsonObj );
			//return responseStructObj;
			AddDataIntoMap( jsonObj, INSTALL );
		}
		
		auto it = m_installedPackageInfoMap.find(appName); 
		
		
		
		
		if (it != m_installedPackageInfoMap.end() )
		{
			if(appName == "GatewayAgent")
				version = VERSION_NUMBER;
				
			if( version != it->second->appVersion ) //ver must not be same
			{
				nlohmann::json tempJson;
				tempJson[APP_NAME] = appName;
				tempJson[COMMAND] = STOP_APP;
				tempJson[UPDATE_FLAG] = true;
				
				if( appName != "GatewayAgent" )
				StopApplication( tempJson );
				
				
				m_sendMsgWrapperObj->SendReportedJsonToCloud( jsonObj, REPORT_DOWNLOADING );
				AddDataIntoMap( jsonObj, UPGRADE );
				
				if( DownloadPackage( jsonObj ) == SUCCESS )
				{
					
					
					std::cout<<"@@@@@@@@@@@@DownloadPackage: sucesss****************!"<<"\n";
					if( UnzipPackage( jsonObj ) == SUCCESS )
					{
						std::string ValidateAppCmd = " ";
		                m_persistentJson[appName]["packagename"] = it->second->packageName;
		                std::string appPath = INSTALLPATH + appName + "/";
		
						
						std::cout<<"##############UnzipPackage: sucesss****************!"<<"\n";
						nlohmann::json installedAppInfoJson;
						installedAppInfoJson[INSTALLED_PACKAGES][appName] = jsonObj["version"];
                        installedAppInfoJson[SUB_JOB_ID] = jsonObj[SUB_JOB_ID];

							std::string downloadDir = it->second->appDownloadPath;
			                downloadDir += it->second->packageName;
						    int appisValid = -1,applibValid = -1,appInstallValid = -1, backupflag =-1,pkgcfgvalid = -1;
							
							if( appName != "GatewayAgent" )
							installedAppInfoJson[CUSTOM_APPS][appName][STATUS] = "Not Running";
							
							  
							   //custom appp, check it directory present, if so, check install path for app and cleanit and cp app dir.
							   ValidateAppCmd = "ls " + downloadDir  + "/" + appName ; //devicemanger
							   
							   appisValid = system( ValidateAppCmd.c_str() );
							   
							   ValidateAppCmd = "ls " + downloadDir +  "/" + "libs/" + "lib" + appName + ".so" ; //library
							   
							   applibValid = system( ValidateAppCmd.c_str() );
							   
							   // backup,current version of app
							   // 

							   std::string backupappcmd = "mv -f ";
							   
							   backupappcmd += INSTALLPATH + appName + "/" + appName + " ";
							   
							   backupappcmd += INSTALLPATH + appName + "/" + appName + "0" ;
							   
							   backupflag = system( backupappcmd.c_str() );
							   
							   if(backupflag != 0)
							   std::cout<<"\n$$$$$####failed to backup app:Error\n";
							   
							   std::string InstallCmd = " ";
							    //cp director from, temp to install path ie /opt/IoT_Gateway
								  
							   InstallCmd = "cp -f -R " + downloadDir + "/" + appName  + " " + INSTALLPATH + appName;
	
							   appInstallValid = system( InstallCmd.c_str() );
							   
							   std::string packageConfigFileName = downloadDir + "/"+ PACKAGE_CONFIG_JSON;
							   
							   std::string pkgcfgCmdvalid = "ls " + packageConfigFileName;
							   
							   pkgcfgvalid = system( pkgcfgCmdvalid.c_str() );
							   
							   
							   std::cout<<"\napplibValid\n"<<applibValid;
							   std::cout<<"\nappisValid\n"<<appisValid;
							   std::cout<<"\naInstallCmd\n"<<InstallCmd<<"\n";
							   std::cout<<"\nappInstallValid\n"<<appInstallValid<<"\n";
							   
							 
							   
							    
							  
							   
							   if((appisValid == 0) && (pkgcfgvalid == 0)&& (appInstallValid == 0)) //pkgcfgvalid
							   {
								  // if lib and dependancies, need to be updated, check if config file is present 
								
								
								nlohmann::json fileContentJson = ReadAndSetConfiguration( packageConfigFileName ); // not handled, error
								it->second->appJson = fileContentJson;
							   
							    m_persistentJson[appName]["app_install_path"] = INSTALLPATH + appName + "/";
								m_persistentJson[appName]["state"] = INSTALLED;
								m_persistentJson[appName]["app_json"] = fileContentJson;
								
								
								m_persistencyManagerObj->SetConfiguration( m_persistentJson );
								m_sendMsgWrapperObj->SendReportedJsonToCloud( jsonObj, REPORT_INSTALLED );
								
							
							   
								   
							
								
								if((applibValid == 0))
								UpdateLibFiles( fileContentJson, downloadDir ); // not handled 
								
								logg.str("");
								logg << "PackageManager::UnzipPackage()  GatewayID : " << m_gatewayId << ",  Message : Unzip package Successfully. AppName : " << appName;
								m_exceptionLoggerObj->LogInfo( logg.str() );
								
							    // re-start app, if fails, recover back app
								
								startApplicationJson[COMMAND] = START_APP;
								startApplicationJson[APP_NAME] = appName;
								startApplicationJson[SUB_JOB_ID] = jsonObj[SUB_JOB_ID];
								
								 if( appName != "GatewayAgent" )
								   responseStructObj = StartApplication(startApplicationJson);
								else
									{
										responseStructObj.status = SUCCESS;
										std::string chmodCmd = "chmod +x ";
                                         chmodCmd += INSTALLPATH + appName + "/" + appName ;                
										 system( chmodCmd.c_str() );
									}
								   
								if(responseStructObj.status == SUCCESS)	
								
							    {
									installedAppInfoJson[CUSTOM_APPS][appName][STATUS] = "Running";
								
									//installedAppInfoJson[CUSTOM_APPS][appName]["device_configuration"] = nullptr;// retain, last config.
									m_sendMsgWrapperObj->SendReportedJsonToCloud( installedAppInfoJson, REPORT_GENERIC );
									responseStructObj.status = SUCCESS;
									responseStructObj.responseMetaInformation = appName + " package updated successfully.";
									
									// remove unzipped packages
									std::string removezipcmd = "rm -f -r ";
									removezipcmd += downloadDir;
									if(0 != system(removezipcmd.c_str() ))
									std::cout<<"\nremoved unzip,fies:failed\n";
									
									return responseStructObj;
								
								}else
								{
									// recover and respond failure
									   backupappcmd = "mv -f ";
									   backupappcmd =  INSTALLPATH + appName + "/" + appName + "0"+ " " + INSTALLPATH + appName + "/" + appName;
									   
									   backupflag = system( backupappcmd.c_str() );
									   
									   if(backupflag != 0)
									   std::cout<<"\n$$$$$####failed to backup:recovery app:Error\n";
									   
									   
									// remove unzipped packages
									std::string removezipcmd = "rm -f -r ";
									removezipcmd += downloadDir;
									if(0 != system(removezipcmd.c_str() ))
									std::cout<<"\nremoved unzip,fies:failed\n";
									   
			
									logg.str("");
									logg << "PackageManager::UpgradePackage()  GatewayID : " << m_gatewayId << ",  Message : app restart failed " << appName << " package failed.";
									m_exceptionLoggerObj->LogError( logg.str() );
									responseStructObj.status = FAILURE;
									responseStructObj.responseMetaInformation = appName + " failed : start app:package update failed.";
								
								
								return responseStructObj;
								}
								
							   }else
							   {
								logg.str("");
								logg << "PackageManager::UpgradePackage()  GatewayID : " << m_gatewayId << ",  Message : Unzip " << appName << " package failed.";
								m_exceptionLoggerObj->LogError( logg.str() );
								responseStructObj.status = FAILURE;
								responseStructObj.responseMetaInformation = appName + " package update failed.";
							
								
								return responseStructObj;
										   
							   }
							
							
						
						
					}
					else
					{
						logg.str("");
						logg << "PackageManager::UpgradePackage()  GatewayID : " << m_gatewayId << ",  Message : Unzip " << appName << " package failed.";
						m_exceptionLoggerObj->LogError( logg.str() );
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = appName + " package update failed.";
					
						
						return responseStructObj;
					}
				}
				else
				{
					logg.str("");
					logg << "PackageManager::UpgradePackage()  GatewayID : " << m_gatewayId << ",  Message : Download " << appName << " package failed.";
					m_exceptionLoggerObj->LogError( logg.str() );
					responseStructObj.status = FAILURE;
					responseStructObj.responseMetaInformation = appName + " package downloading failed.";
					// revert back the older version of app

					return responseStructObj;
				}
			}
			else
			{
				logg.str("");
				logg << "PackageManager::UpgradePackage()  GatewayID : " << m_gatewayId << ",  Message : Version is already up to date.";
				m_exceptionLoggerObj->LogError( logg.str() );
				responseStructObj.status = FAILURE;
				responseStructObj.responseMetaInformation = appName + " package is already up to date.";
				return responseStructObj;
			}
		}
		else
		{
			//IOT or IoT
			if(appName == "IoT_Defender")
			{
				logg.str("");
				logg << "PackageManager::UpgradePackage() : *******jsonObj" << jsonObj;
				m_exceptionLoggerObj->LogInfo( logg.str() );
				
				std::string connection_string = jsonObj["connection_string"];
				nlohmann::json installedAppInfoJson;
				
				logg.str("");
				logg << "PackageManager::UpgradePackage() : *******connection_string" << connection_string;
				m_exceptionLoggerObj->LogInfo( logg.str() );
						
						
						
				m_sendMsgWrapperObj->SendReportedJsonToCloud( jsonObj, REPORT_DOWNLOADING );
				if( DownloadPackage( jsonObj ) == SUCCESS )
				{
					std::cout<<"@@@@@@@@@@@@DownloadPackage: sucesss****************!"<<"\n";
					std::string version = jsonObj[VERSION];
					
					std::string downloadDir ;
					int appInstallValid;
					
					logg.str("");
					logg << "PackageManager::UpgradePackage()  entered" ;
					m_exceptionLoggerObj->LogInfo( logg.str() );
					
					if( UnzipPackage( jsonObj ) == SUCCESS )
					{
						//removing defender
						std::string removeDefender;
						
						
						removeDefender = "sudo apt-get remove -y defender-iot-micro-agent";
						std::cout<<"Removing the defender-iot-micro-agent "<<removeDefender<<std::endl;
						appInstallValid = system( removeDefender.c_str() );
						
						
						logg.str("");
						logg << "PackageManager::UpgradePackage()  :Removing the defender-iot-micro-agent-value is " << appInstallValid ;
						m_exceptionLoggerObj->LogInfo( logg.str() );
						
						std::string InstallCmd = " ";
							
						//downloadDir = TEMP_PACKAGE_INSTALLPATH + appName + "_" + version + "/" + appName ;
						downloadDir = DOWNLOADPATH  + appName + "_" + version ;
						std::cout<<"********************** downloadDir "<<downloadDir<<std::endl;
						
						
						
						std::string path = DOWNLOADPATH_DEFENDER;
						//path += appName + "_" + version + "/";
						std::cout<<"***********************path "<<path<<std::endl;
						
						
						InstallCmd = "cp -f -R " + downloadDir  + " " + path ;
						std::cout<<"!!!!!!!!!!!!!copied file to inside IOT_Defender -downloadDir "<<downloadDir<<"***to path "<<path<<std::endl;
						appInstallValid = system( InstallCmd.c_str() );
						
						logg.str("");
						logg << "PackageManager::UpgradePackage()  :copied file to inside IOT_Defender -downloadDir "<<downloadDir<<"***to path "<<path;
						m_exceptionLoggerObj->LogInfo( logg.str() );
						
						
						
						std::cout<<"******************** Path Name: "<<path<<std::endl;
						 
						
						std::string InstallPackage ;
						//InstallPackage="apt-get install" +"./"+"/opt/IoT_Defender/"+ appName + "_" + version + "/" + appName+"defenderiot-debian-10-arm64-4.4.1.deb";
						InstallPackage ="sudo apt-get install -y /opt/IoT_Defender/"+ appName + "_" + version + "/" + appName +"/"+"defenderiot-debian-10-arm64-4.4.2.deb";
						std::cout<<"************** Installing deb package "<<InstallPackage<<std::endl;
						system(InstallPackage.c_str());
						
						logg.str("");
						logg << "PackageManager::UpgradePackage() :************** Installing deb package-value is " << InstallPackage ;
						m_exceptionLoggerObj->LogInfo( logg.str() );
						
						//checking status of IOT_defender
						std::string Status = "systemctl status defender-iot-micro-agent";
						system(Status.c_str());
						
						std::cout<<"************* After running systemctl cmd and it return value is "<<Status<<std::endl;
						
						std::string appendCS = "bash -c 'echo \"" + connection_string +"\""+">/etc/defender_iot_micro_agent/connection_string.txt'";
						system(appendCS.c_str());
						
						logg.str("");
						logg << "PackageManager::UpgradePackage() :************* After running systemctl cmd - value is " << appendCS ;
						m_exceptionLoggerObj->LogInfo( logg.str() );
						
						std::cout<<"************* After running bash cmd it returned value is "<<appendCS<<std::endl;

						
						m_sendMsgWrapperObj->SendReportedJsonToCloud( jsonObj, REPORT_INSTALLED );
						
						installedAppInfoJson[CUSTOM_APPS][appName][STATUS] = "Running";
						m_sendMsgWrapperObj->SendReportedJsonToCloud( installedAppInfoJson, REPORT_GENERIC );
						
						responseStructObj.status = SUCCESS;
						responseStructObj.responseMetaInformation = appName + " package updated successfully.";
									
						return responseStructObj;
					}
					else
					{
						installedAppInfoJson[CUSTOM_APPS][appName][STATUS] = "Not Updated";
						m_sendMsgWrapperObj->SendReportedJsonToCloud( installedAppInfoJson, REPORT_GENERIC );
						
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = appName + " failed : unzip failed.";
						return responseStructObj;
					}
				}
				else
				{
					installedAppInfoJson[CUSTOM_APPS][appName][STATUS] = "Not Running";
					m_sendMsgWrapperObj->SendReportedJsonToCloud( installedAppInfoJson, REPORT_GENERIC );
						
					responseStructObj.status = FAILURE;
					responseStructObj.responseMetaInformation = appName + " failed : start app:package update failed.";
					return responseStructObj;
				}
			}
			else
			{
				logg.str("");
				logg << "PackageManager::UpgradePackage()  GatewayID : " << m_gatewayId << ",  Message : Package Not Found.";
				m_exceptionLoggerObj->LogError( logg.str() );
				responseStructObj.status = FAILURE;
				responseStructObj.responseMetaInformation = appName + " package not found in gateway. Please try to install instead of upgrade.";
			}
			return responseStructObj;
		}
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "PackageManager::UpgradePackage()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "PackageManager::UpgradePackage()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured. JSON : " << jsonObj;
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	responseStructObj.status = FAILURE;
	responseStructObj.responseMetaInformation = "Unknown Exception occured.";
	return responseStructObj;
}

/**
 * @brief UpgradePackage() 			:	This method will validate the received version with current version.
 * 										if both version are different then download the updated package from
 *										cloud and unzip the package in predefined temp path and execute shell
 * 										script file. Shell script file will stop the application copy the  
 * 										application and dependencies libs to the install path location and 
 * 										restart the service.
 * 
 * 										
 *										
 * @param nlohmann::json jsonObj	:	It contains Upgrade package information like url, token, app_name, version
 *
 * @return : It will return SUCCESS if package Upgraded successfully otherwise return predefined errors codes.
 */
ResponseStruct PackageManager::UpdateGatewayAgent( nlohmann::json jsonObj )
{
	ResponseStruct responseStructObj;
	std::stringstream logg;
	try
	{
		std::string appName = jsonObj[APP_NAME];
		if( VERSION_NUMBER != jsonObj[VERSION] )
		{
			nlohmann::json gatewayAgentJson;
			gatewayAgentJson = ReadAndSetConfiguration( GATEWAYAGENT_PERSISTENCY_CONFIG );
			gatewayAgentJson[SUB_JOB_ID] = jsonObj[SUB_JOB_ID];
			WriteConfiguration( GATEWAYAGENT_PERSISTENCY_CONFIG, gatewayAgentJson );
			
			m_sendMsgWrapperObj->SendReportedJsonToCloud( jsonObj, REPORT_DOWNLOADING );
			if( DownloadPackage( jsonObj ) == SUCCESS )
			{
				m_sendMsgWrapperObj->SendReportedJsonToCloud( jsonObj, REPORT_APPLYING );
				std::string version = jsonObj[VERSION];
				std::string downloadDir = DOWNLOADPATH;
				
				auto it = m_installedPackageInfoMap.find(appName); 
				downloadDir += appName + "_" + version + "/";
				downloadDir += appName;
				std::cout << "***************** :" << downloadDir <<std::endl;
				std::string unzipCmd = UNZIP_COMMAND_PREFIX + downloadDir + ".zip -d " + TEMP_PACKAGE_INSTALLPATH;
				int returnStatus = system( unzipCmd.c_str() );
				
				if( returnStatus == 0 )
				{
					std::string shellScriptCommand = TEMP_PACKAGE_INSTALLPATH + appName + "/GatewayUpdate.sh"; 
					std::string chmodCommand = "chmod +x " + shellScriptCommand;
					std::string shellScriptCommandf = shellScriptCommand + " &";
					returnStatus = system( chmodCommand.c_str() );
					returnStatus = system( shellScriptCommandf.c_str() );
					if( returnStatus == 0 )
					{
						logg.str("");
						logg << "PackageManager::UpdateGatewayAgent()  GatewayID : " << m_gatewayId << ",  Message : Command Execute successfully. Command : " << shellScriptCommand;
						m_exceptionLoggerObj->LogInfo( logg.str() );
						responseStructObj.status = SUCCESS;
						responseStructObj.responseMetaInformation = appName + " package updated successfully.";
						return responseStructObj;
					}
					else
					{
						logg.str("");
						logg << "PackageManager::UpdateGatewayAgent()  GatewayID : " << m_gatewayId << ",  Message : Command Execution failed. Command : " << shellScriptCommand;
						m_exceptionLoggerObj->LogError( logg.str() );
						responseStructObj.status = FAILURE;
						responseStructObj.responseMetaInformation = appName + " package update failed.";
						return responseStructObj;
					}
				
				}
				else
				{
					logg.str("");
					logg << "PackageManager::UpdateGatewayAgent()  GatewayID : " << m_gatewayId << ",  Message : Unzip " << appName << " package failed.";
					m_exceptionLoggerObj->LogError( logg.str() );
					responseStructObj.status = FAILURE;
					responseStructObj.responseMetaInformation = appName + " package update failed.";
					return responseStructObj;
				}
			}
			else
			{
				logg.str("");
				logg << "PackageManager::UpdateGatewayAgent()  GatewayID : " << m_gatewayId << ",  Message : Download " << appName << " package failed.";
				m_exceptionLoggerObj->LogError( logg.str() );
				responseStructObj.status = FAILURE;
				responseStructObj.responseMetaInformation = appName + " package downloading failed.";
				return responseStructObj;
			}
		}
		else
		{
			responseStructObj.status = FAILURE;
			responseStructObj.responseMetaInformation = appName + " package is already up to date.";
			return responseStructObj;
		}
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "PackageManager::UpdateGatewayAgent()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "PackageManager::UpdateGatewayAgent()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured. JSON : " << jsonObj;
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	responseStructObj.status = FAILURE;
	responseStructObj.responseMetaInformation = "Unknown Exception occured.";
	return responseStructObj;
}


/**
 * @brief UninstallPackage() 		:	This method will uninstlled the requested package.
 * 										
 * @param nlohmann::json jsonObj	:	It contains Uninstall package information like url, token, app_name, version
 *
 * @return : It will return SUCCESS if package Uninstalled successfully otherwise return predefined errors codes.
 */
ResponseStruct PackageManager::UninstallPackage ( nlohmann::json jsonObj )
{
	ResponseStruct responseStructObj;
	std::stringstream logg;
	try
	{
		std::string appName = jsonObj[APP_NAME];
		jsonObj[COMMAND] = STOP_APP;
		jsonObj[UPDATE_FLAG] = false;
		responseStructObj = StopApplication( jsonObj );

		auto it = m_installedPackageInfoMap.find( appName ); 
		if ( it != m_installedPackageInfoMap.end() )
		{
			//remove installed libs
			std::string path = "rm -r ";
			path += INSTALLPATH + appName;
			system(path.c_str());
			
			//remove zip files
			path = "rm -r ";
			path += DOWNLOADPATH + appName + "_" + it->second->appVersion;
			system(path.c_str());
			
            //delete particular device folder if available
			path = "rm -r ";
			path += CONFIG_PATH + appName ;
			system(path.c_str());
            
            // it if only for rule engine.
            if(appName == "RuleEngine")
            { 
                path = "rm -r ";
                path += RULE_PERSISTENCY_CONFIG;
                system(path.c_str());
            }
			
			m_installedPackageInfoMap.erase (it);
			m_persistentJson.erase(m_persistentJson.find(appName));
			m_persistencyManagerObj->SetConfiguration( m_persistentJson );
			
			nlohmann::json installedAppInfoJson;
			installedAppInfoJson[INSTALLED_PACKAGES][appName] = nullptr;
			installedAppInfoJson[CUSTOM_APPS][appName] = nullptr;
			installedAppInfoJson[REGISTERED_DEVICES][appName] = nullptr;
			installedAppInfoJson[SUB_JOB_ID] = jsonObj[SUB_JOB_ID];
			m_sendMsgWrapperObj->SendReportedJsonToCloud( installedAppInfoJson, REPORT_GENERIC );
			m_sendMsgWrapperObj->SendReportedJsonToCloud( jsonObj, REPORT_UNINSTALLED );
			
			//check and remove 
			nlohmann::json installedAppInfoJson1;
			if( appName == "GatewayAgent" || appName == "MQTTAgent" || appName == "CachingAgent" )
			{
				installedAppInfoJson1[SYSTEM_APPS][appName][STATUS] = "Not Running";
				installedAppInfoJson1[SYSTEM_APPS]["GatewayAgent"][STATUS] = "Not Running";
				installedAppInfoJson1[SUB_JOB_ID] = jsonObj[SUB_JOB_ID];
				m_sendMsgWrapperObj->SendReportedJsonToCloud( installedAppInfoJson1, REPORT_GENERIC );
			}
			
			responseStructObj.status = SUCCESS;
			responseStructObj.responseMetaInformation = appName + " package uninstalled successfully.";
			return responseStructObj;
		}
		else
		{
			logg.str("");
			logg << "PackageManager::UninstallPackage()  GatewayID : " << m_gatewayId << ",  Message : Package Not Found.";
			m_exceptionLoggerObj->LogError( logg.str() );
			responseStructObj.status = FAILED;
			responseStructObj.responseMetaInformation = appName + " package uninstalling failed due to package not found in gateway.";
			return responseStructObj;
		}
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "PackageManager::UninstallPackage()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "PackageManager::UninstallPackage()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured. JSON : " << jsonObj;
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	
	responseStructObj.status = FAILURE;
	responseStructObj.responseMetaInformation = "Unknown Exception occured.";
	return responseStructObj;
}

/**
 * @brief StartApplication() 		:	This method will START the requested application.
 * 										
 * @param nlohmann::json jsonObj	:	It contains start application information like app_name, command.
 *
 * @return : It will return SUCCESS if application start successfully otherwise return predefined errors codes.
 */		
ResponseStruct PackageManager::StartApplication ( nlohmann::json jsonObj )
{
	ResponseStruct responseStructObj;
	std::stringstream logg;
    std::string appName = "";
	try
	{
		int returnvValue = FAILED;
		appName = jsonObj[APP_NAME];
		
		auto it = m_installedPackageInfoMap.find( appName ); 
		
		if ( it != m_installedPackageInfoMap.end() )
		{	
			long processId = GetProcessIdByName( appName );
			
			if( processId > 0 )
			{
				responseStructObj.status = SUCCESS;
				responseStructObj.responseMetaInformation = appName + " application already started.";
				return responseStructObj;
			}
			
			nlohmann::json appJson = it->second->appJson;
			std::string startAppCommand = INSTALLPATH + appName + "/";
			startAppCommand += appName;
			std::ifstream file( startAppCommand.c_str() );
			//to check application file is present at given location or not.
			if( file )
			{
				std::string chmodCmd = "chmod +x " + startAppCommand ;                
				system( chmodCmd.c_str() );
				if( !appJson["protocol"].is_null() )
				{
					std::string protocolName = appJson["protocol"];
					startAppCommand += " " + m_gatewayId + " " + protocolName + " &";
				}
				else
				{
					startAppCommand += " " + m_gatewayId + " " + m_cloudAppName + " &" ;
				}

				if( system( startAppCommand.c_str() ) == 0 )
				{
					m_watchDogObj->RegisterApp( appName );
					it->second->state = START;
					m_persistentJson[appName][STATE] = START;
					m_persistencyManagerObj->SetConfiguration( m_persistentJson );
					usleep(5000);
					it->second->appPID = GetProcessIdByName( appName );
					
					if((it->second->appPID <= 0 ))
					{
						
						
						std::string startAppCommanddef = INSTALLPATH + appName + "/";
						startAppCommanddef += appName;
						startAppCommanddef += "0";
						std::ifstream file( startAppCommanddef.c_str() );
						if(file)
						{
							std::string recoverAppcmd = "cp ";
							recoverAppcmd += startAppCommanddef;
							recoverAppcmd += " ";
							recoverAppcmd += startAppCommand;
							if(0 == system( recoverAppcmd.c_str() ))
								std::cout<<"\n App recovered successfully \n";
								else
									std::cout<<"\nfailed to recover app\n";
							 
						}else
							std::cout<<"\nRecovery failed to recover app, not present\n";
						
						/*std::string startDefAppCommand = INSTALLPATH + appName + "/";
					    startDefAppCommand += appName;
						startDefAppCommand += "0";
						// app, proces, not present, start backup app
						std::string chmodCmd = "chmod +x " + startDefAppCommand ;                
							system( chmodCmd.c_str() );
							if( !appJson["protocol"].is_null() )
							{
								std::string protocoldefName = appJson["protocol"];
								startDefAppCommand += " " + m_gatewayId + " " + protocoldefName + " &";
							}
							else
							{
								startDefAppCommand += " " + m_gatewayId + " " + m_cloudAppName + " &" ;
							}
							if( system( startDefAppCommand.c_str() ) == 0 )
							{
								std::cout<<"\n Previous version of Application, started...\n";
								}*/
					}
					
					nlohmann::json installedAppInfoJson;
					
					if( appName == "GatewayAgent" || appName == "MQTTAgent" || appName == "CachingAgent" )
					{
						installedAppInfoJson[SYSTEM_APPS][appName][STATUS] = "Running";
						installedAppInfoJson[SYSTEM_APPS]["GatewayAgent"][STATUS] = "Running";
						if( !jsonObj[SUB_JOB_ID].is_null() )
						{
							installedAppInfoJson[SUB_JOB_ID] = jsonObj[SUB_JOB_ID];
						}
						m_sendMsgWrapperObj->SendReportedJsonToCloud( installedAppInfoJson, REPORT_GENERIC );
					}
					else
					{
						installedAppInfoJson[CUSTOM_APPS][appName][STATUS] = "Running";
						if( !jsonObj[SUB_JOB_ID].is_null() )
						{
							installedAppInfoJson[SUB_JOB_ID] = jsonObj[SUB_JOB_ID];
						}
						m_sendMsgWrapperObj->SendReportedJsonToCloud( installedAppInfoJson, REPORT_GENERIC );
					}
					
					if( m_appHealthMonitoringObj )
					{
						m_appHealthMonitoringObj->RegisterApp( appName );
					}
					responseStructObj.status = SUCCESS;
					responseStructObj.responseMetaInformation = appName + " has started successfully.";
					
					return responseStructObj;
				}
				else
				{
					logg << "PackageManager::StartApplication()  GatewayID : " << m_gatewayId << ",  Message : Start application command execute failed. AppName : " << appName;
					m_exceptionLoggerObj->LogError( logg.str() );
					responseStructObj.status = FAILURE;
					responseStructObj.responseMetaInformation = "Cannot start " + appName;
					return responseStructObj;
				}
			}
			else
			{
				logg << "PackageManager::StartApplication()  GatewayID : " << m_gatewayId << ",  Message : Invalid file name received  AppName : " << appName;
				m_exceptionLoggerObj->LogError( logg.str() );
				responseStructObj.status = FAILURE;
                responseStructObj.responseMetaInformation = "Cannot start " + appName + " due to app not found in the gateway.";
				//checking for default app
				std::string startAppCommanddef = INSTALLPATH + appName + "/";
			    startAppCommanddef += appName;
				startAppCommanddef += "0";
			    std::ifstream file( startAppCommanddef.c_str() );
				if(file)
				{
					std::string recoverAppcmd = "cp ";
					recoverAppcmd += startAppCommanddef;
					recoverAppcmd += " ";
					recoverAppcmd += startAppCommand;
					if(0 == system( recoverAppcmd.c_str() ))
						std::cout<<"\n App recovered successfully \n";
						else
							std::cout<<"\nfailed to recover app\n";
					 
				}else
					std::cout<<"\nRecovery failed to recover app, not present\n";
				
				return responseStructObj;
			}
		}
		else
		{
			logg << "PackageManager::StartApplication()  GatewayID : " << m_gatewayId << ",  Message : Package Not Found.  AppName : " << appName;
			m_exceptionLoggerObj->LogError( logg.str() );
			
			responseStructObj.status = FAILURE;
            responseStructObj.responseMetaInformation = "Cannot start " + appName + " due to app package not found in the gateway.";
			return responseStructObj;
		}
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "PackageManager::StartApplication()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "PackageManager::StartApplication()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured. JSON : " << jsonObj;
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	responseStructObj.status = FAILURE;
    responseStructObj.responseMetaInformation = "Cannot start " + appName + " due to unknown exception occured during start app.";
	return responseStructObj;
}

/**
 * @brief StartApplication() 		:	This method will STOP the requested application.
 * 										
 * @param nlohmann::json jsonObj	:	It contains stop application information like app_name, command.
 *
 * @return : It will return SUCCESS if application stop successfully otherwise return predefined errors codes.
 */	
ResponseStruct PackageManager::StopApplication ( nlohmann::json jsonObj  )
{
	ResponseStruct responseStructObj;
	std::stringstream logg;
    std::string appName = ""; 
	try
	{
		int returnvValue = FAILED;
		appName = jsonObj[APP_NAME];
		bool updateFlag = jsonObj[UPDATE_FLAG];
		
		if( (appName == "GatewayAgent" || appName == "MQTTAgent" || appName == "CachingAgent") && !updateFlag )
		{
			logg.str( std::string() );
			logg << "PackageManager::StopApplication()  GatewayID : " << m_gatewayId << ",  Message : System app can't be stopped. AppName : " << appName;
			m_exceptionLoggerObj->LogError( logg.str() );
			responseStructObj.status = FAILURE;
			responseStructObj.responseMetaInformation = "System app can't be stopped.";
			return responseStructObj;
		}

		
		long processId = GetProcessIdByName( appName );
		
		std::cout << processId << " " << appName << "\n\n";
		
		if( processId > 0 )
		{
			m_watchDogObj->DeRegisterApp( appName );
			char stopCommand[500];
			sprintf(stopCommand,"killall -9 %s",appName.c_str() );
			if( system( stopCommand ) == 0 )
			{
				auto it = m_installedPackageInfoMap.find( appName ); 	
				if ( it != m_installedPackageInfoMap.end() )
				{
					it->second->state = STOP;
					m_persistentJson[appName]["state"] = STOP;
					m_persistencyManagerObj->SetConfiguration( m_persistentJson );
					it->second->appPID = 0;
                    if(m_appHealthMonitoringObj)
                    {
                        m_appHealthMonitoringObj->DeRegisterApp( appName );
                    }
					
				}
				
				nlohmann::json installedAppInfoJson;
				if( appName == "GatewayAgent" || appName == "MQTTAgent" || appName == "CachingAgent" )
				{
					installedAppInfoJson[SYSTEM_APPS][appName][STATUS] = "Not Running";
					installedAppInfoJson[SYSTEM_APPS]["GatewayAgent"][STATUS] = "Not Running";
                    installedAppInfoJson[SUB_JOB_ID] = jsonObj[SUB_JOB_ID];
					m_sendMsgWrapperObj->SendReportedJsonToCloud( installedAppInfoJson, REPORT_GENERIC );
				}
				else
				{
					installedAppInfoJson[CUSTOM_APPS][appName][STATUS] = "Not Running";
                    installedAppInfoJson[SUB_JOB_ID] = jsonObj[SUB_JOB_ID];
					m_sendMsgWrapperObj->SendReportedJsonToCloud( installedAppInfoJson, REPORT_GENERIC );
				}
				
				logg.str("");
				logg << "PackageManager::StopApplication()  GatewayID : " << m_gatewayId << ",  Message : Stop Application Successfully. AppName : " << appName;
				m_exceptionLoggerObj->LogInfo( logg.str() );
				responseStructObj.status = SUCCESS;
				responseStructObj.responseMetaInformation = appName + " has stopped successfully.";
				return responseStructObj;
			}
			else
			{
				nlohmann::json installedAppInfoJson;
				logg.str("");
				logg << "PackageManager::StopApplication()  GatewayID : " << m_gatewayId << ",  Message : Stop application command execution failed. AppName : " << appName;
				m_exceptionLoggerObj->LogError( logg.str() );
				responseStructObj.status = FAILURE;
				responseStructObj.responseMetaInformation = "Cannot stop " + appName;
				installedAppInfoJson[CUSTOM_APPS][appName][STATUS] = "Not Running";
				installedAppInfoJson[SUB_JOB_ID] = jsonObj[SUB_JOB_ID];
				m_sendMsgWrapperObj->SendReportedJsonToCloud( installedAppInfoJson, REPORT_GENERIC );
				return responseStructObj;
			}
		}
		else
		{
			logg.str("");
			logg << "PackageManager::StopApplication()  GatewayID : " << m_gatewayId << ",  Message : Process not found. AppName : " << appName;
			m_exceptionLoggerObj->LogError( logg.str() );
			responseStructObj.status = FAILURE;
            responseStructObj.responseMetaInformation = "Cannot stop " + appName + ". Application not running.";
			return responseStructObj;
		}
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "PackageManager::StopApplication()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what() << ", JSON : " << jsonObj;
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "PackageManager::StopApplication()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured. JSON : " << jsonObj;
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	responseStructObj.status = FAILURE;
	responseStructObj.responseMetaInformation = "Cannot stop " + appName + " due to unknown exception occured during stop app.";
	return responseStructObj;
}

/**
 * @brief RestartApplication() 		:	This method will RESTART the requested application.
 * 										
 * @param nlohmann::json jsonObj	:	It contains restart application information like app_name, command.
 *
 * @return : It will return SUCCESS if application restart successfully otherwise return predefined errors codes.
 */	
ResponseStruct PackageManager::RestartApplication ( nlohmann::json jsonObj )
{
	std::stringstream logg;
	ResponseStruct responseStructObj;
    std::string appName = "";
	try
	{
		jsonObj[UPDATE_FLAG] = true;
		responseStructObj = StopApplication( jsonObj ); 
        appName = jsonObj[APP_NAME];
		
		if( responseStructObj.status == SUCCESS )
		{
			responseStructObj = StartApplication( jsonObj );
            if( responseStructObj.status == SUCCESS )
            {
                responseStructObj.responseMetaInformation = appName + " has re-started successfully";
            }
            else
            {
            }
		}
        else
        {
            responseStructObj.responseMetaInformation = "Cannot re-start " + appName + ". Application not running.";
        }
		return responseStructObj;
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "PackageManager::RestartApplication()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "PackageManager::RestartApplication()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured. JSON : " << jsonObj;
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	
	responseStructObj.status = FAILURE;
	responseStructObj.responseMetaInformation = "Cannot re-start " + appName + " due to unknown exception occured during re-start app.";
	return responseStructObj;
}




						int is_regular_file(const char *path)
						{
							struct stat path_stat;
							stat(path, &path_stat);
							return S_ISREG(path_stat.st_mode);
						}
						   
/**
 * @brief UnzipPackage() 			:	This method will unzip the package in predefined location.
 * 										
 * @param nlohmann::json jsonObj	:	It contains unzip package information like app_name.
 *
 * @return : It will return SUCCESS if application restart successfully otherwise return predefined errors codes.
 */	
int PackageManager::UnzipPackage ( nlohmann::json jsonObj )
{
	std::stringstream logg;
	try
	{
		std::string appName = jsonObj[APP_NAME];
		auto it = m_installedPackageInfoMap.find(appName); 
		
		std::string unzipCmd = " ";
		
		if ( it != m_installedPackageInfoMap.end() )
		{		
			std::string downloadDir = it->second->appDownloadPath;
			downloadDir += it->second->packageName;
			m_persistentJson[appName]["packagename"] = it->second->packageName;
			std::string appPath = INSTALLPATH + appName + "/";
			
			unzipCmd = UNZIP_COMMAND_PREFIX + downloadDir + ".zip -d " + downloadDir;
			
			std::cout << "unzipCmd : " << unzipCmd << std::endl;
			
			int returnStatus = system( unzipCmd.c_str() );
			
			if( returnStatus == 0 )
			{
				//check, crc of app
				
				std::string crcCmd = CMD_CRC + downloadDir + "/" + appName + " | awk '{print $1}' >> "  + downloadDir + "/" + "crc.txt";
				
				logg.str("");
				logg << "\n$$$$$$$$$$$: crcCmd\n" << "\n";
				m_exceptionLoggerObj->LogDebug( logg.str() );
				
				returnStatus = system( crcCmd.c_str() );
				
				if(returnStatus != 0)
					{ 
						logg.str("");
						logg << "\n  crcCmd, failed to execute  \n";
						m_exceptionLoggerObj->LogDebug( logg.str() );
					}
					else{
						
						// open crc.txt and compare with package_config.json
						std::string  crcval = " ";
						
						std::string  crctxtfilepath = downloadDir + "/" + "crc.txt";

						logg.str("");
						logg << "\ncrc, created path: \n" << crctxtfilepath;;
						m_exceptionLoggerObj->LogDebug( logg.str() );
						std::ifstream file( crctxtfilepath );
						
						if( file )
						{
							file >> crcval;
							logg.str("");
							logg <<"\ncrc file opened : ok\n";
							logg << "packageManager::unzip::: "  << ",  Message :  open crc file: successfully\n";
							m_exceptionLoggerObj->LogDebug( logg.str() );
							file.close();
						}
						else
						{	
							logg.str("");
							logg << "\ncrc file opened : Failed\n";
							logg << "packageManager::unzip::: "  << ",  Message : Failed to open crc file\n";
							m_exceptionLoggerObj->LogError( logg.str() );
						}
						
						logg.str("");
						logg << "\n###1#####crc:sstringfrom txt#################\n"<<crcval;
						m_exceptionLoggerObj->LogDebug( logg.str() );
		
						std::string packageConfigFileName = downloadDir + "/" + PACKAGE_CONFIG_JSON;
						nlohmann::json fileContentJson = ReadAndSetConfiguration( packageConfigFileName ); 
						
						logg.str("");
						logg << "\n#######2######crc: string:from:json#########\n"<<fileContentJson["app_crc_val"];
						m_exceptionLoggerObj->LogDebug( logg.str() );

						 if(fileContentJson["app_crc_val"] != 	crcval)
						 {
							    logg.str("");
								logg <<"\n###1#####crc:failed to match#################\n"<<crcval;
								logg << "PackageManager::UnzipPackage()  GatewayID : " << m_gatewayId << ",  Message : CRC match.failed for  AppName : " << appName;
								m_exceptionLoggerObj->LogError( logg.str() );
								return PACKAGE_NOT_FOUND;
							 
						 }
						 logg.str("");
						 logg <<"\n###1#####crc:matched sucesss#################\n"<<crcval;
				         logg << "PackageManager::UnzipPackage()  GatewayID : " << m_gatewayId << ",  Message : CRC Matched Successfully. AppName : " << appName;
				         m_exceptionLoggerObj->LogDebug( logg.str() );

						
					}
		   
						   
				logg.str("");
				logg << "PackageManager::UnzipPackage()  GatewayID : " << m_gatewayId << ",  Message : Unzip package Successfully. AppName : " << appName;
				m_exceptionLoggerObj->LogDebug( logg.str() );
				return SUCCESS;
			}
			else
			{
				m_sendMsgWrapperObj->SendReportedJsonToCloud( jsonObj, REPORT_ERROR );
				logg.str("");
				logg << "PackageManager::UnzipPackage()  GatewayID : " << m_gatewayId << ",  Message : Unzip package failed. AppName : " << appName;
				m_exceptionLoggerObj->LogError( logg.str() );
				return UNZIP_PACKAGE_FAILED;
			}	
		}
		else
		{
			if(appName == "IoT_Defender")
			{
				std::string version = jsonObj[VERSION];
				
				std::string downloadDir = "/opt/IoT_Gateway/packages/" + appName + "_" + version + "/" + appName ;
				
				logg.str("");
				logg << "PackageManager::UnzipPackage()  downloadDir : " << downloadDir << "appName" << appName;
				m_exceptionLoggerObj->LogInfo( logg.str() );
			
				unzipCmd = UNZIP_COMMAND_PREFIX + downloadDir + ".zip -d " + downloadDir;
				
				logg.str("");
				logg << "PackageManager::UnzipPackage()  unzipCmd path : " << unzipCmd ;
				m_exceptionLoggerObj->LogInfo( logg.str() );
				
				
				std::cout << "unzipCmd : " << unzipCmd << std::endl;
				int returnStatus = system( unzipCmd.c_str() );
				
				if( returnStatus == 0 )
				{
					return SUCCESS;
				}
				else
				{
					return UNZIP_PACKAGE_FAILED;
				}
			}
			else
			{
				logg.str("");
				logg << "PackageManager::UnzipPackage()  GatewayID : " << m_gatewayId << ",  Message : Package not found. AppName : " << appName;
				m_exceptionLoggerObj->LogError( logg.str() );
				return PACKAGE_NOT_FOUND;
			}
		}
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "PackageManager::UnzipPackage()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "PackageManager::UnzipPackage()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured. JSON : " << jsonObj;
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	
	return EXCEPTION;
}



/**
 * @brief DownloadPackage() 		:		This method will create directory for download package and call the
 * 											GetFile method to download the package from cloud .
 * 										
 * @param nlohmann::json jsonObj	:		It contains Download package information like app_name,url,token and version.
 *
 * @return : It will return SUCCESS if download package successfully otherwise return predefined errors codes.
 */	
int PackageManager::DownloadPackage( nlohmann::json jsonObj )
{
	std::stringstream logg;
	
	try
	{
		std::string version = jsonObj[VERSION];
		std::string appName = jsonObj[APP_NAME];
		std::string path = DOWNLOADPATH;
		std::string temp_path;
		
		if(appName == "IoT_Defender")
		{
			path += appName + "_" + version + "/";
		}
		else
		{
			auto it = m_installedPackageInfoMap.find(appName); 
			path += appName + "_" + version + "/";
			
			m_persistentJson[appName]["downloaded_path"] = path;
			m_persistentJson[appName][VERSION] = version;
			m_persistencyManagerObj->SetConfiguration( m_persistentJson ); // set before, downloading
			
			if (it != m_installedPackageInfoMap.end() )
			{
				it->second->appDownloadPath = path;	
			}
		}
		temp_path = path;
		
		//create directory for download package.
		std::string createcmd = "mkdir -p " + path; 
		system( createcmd.c_str() );
		std::string url = jsonObj[URL];

		std::string packageName = GetLastToken( url, "/" );
		path += packageName;
		url += jsonObj[TOKEN];
		
		//url, space, correction
	    std::string s=url;
		std::string x = " ", y = "%20";
		size_t pos;
		while ((pos = s.find(x)) != std::string::npos) {
			s.replace(pos, 1, y);
		}
		
		int ret = GetFile( s, path );
	 
	    
		
		if(ret == 0)                              
		     {
				// m_persistencyManagerObj->SetConfiguration( m_persistentJson );
				   std::cout << "\npackageManager:GetFile:success"<<ret<<"\n\n";
				   
				   
                   std::string createcmd = "mkdir -p " + temp_path + "/" + appName;  // directory to store unzip, files
				   std::cout << "\npackageManager:tempath<ret\n"<<temp_path;
				   
					if(0 != system( createcmd.c_str() ))
                      std::cout<<"\n donloaded:creating:direcory,for app: failed\n" ;       
				   
				 }
				 
		else
		std::cout << "\npackageManager:GetFile:failed"<<ret<<"\n\n";	

		return ret;
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "PackageManager::DownloadPackage()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "PackageManager::DownloadPackage()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured. JSON : " << jsonObj;
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	return DOWNLOAD_FAILED;
}

/**
 * @brief GetFile() 		:	This method will download the package(.zip file) from cloud.
 * 										
 * @param std::string url	:	It contains the url where it from download the package.
 * @param std::string path	:	It contains download path.
 *
 * @return : It will return SUCCESS if download package successfully otherwise return predefined errors codes.
 */	
int PackageManager::GetFile( std::string url, std::string path )
{
	std::stringstream logg;
	try
	{
		bool retVal = false;
		long response_code = 404;
		CURL *curl;
		CURLcode res;
		curl = curl_easy_init();
		
		//std::cout<<"PackageManager::GetFile:url"<<url;
		//std::cout<<"PackageManager::GetFile:path"<<path;
		
		
		if( curl ) 
		{
			/* Perform the request, res will get the return code */
			res = curl_easy_perform(curl);
			int utl_len = url.length();
			//std::string new_url = curl_easy_escape(curl,url.c_str(),utl_len);
			
			//std::cout<<"PackageManager::GetFile:url"<<new_url;
			
			
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt( curl, CURLOPT_URL, url.c_str() );
			curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 500L );//20L - 500L
			curl_easy_setopt( curl, CURLOPT_LOW_SPEED_TIME, 500L ); //20L -
			curl_easy_setopt( curl, CURLOPT_LOW_SPEED_LIMIT, 30L ); //30L -
			/* example.com is redirected, so we tell libcurl to follow redirection */
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			/* create an output file and prepare to write the response */
			FILE *output_file = fopen( path.c_str(), "w" );
			curl_easy_setopt( curl, CURLOPT_WRITEDATA, output_file );
			
			
			
			curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &response_code );
			
			if( response_code == 200 && res == CURLE_OK )
			{
				logg.str("");
				logg << "PackageManager::GetFile()  GatewayID : " << m_gatewayId << ",  Message : File Downloaded Successfully. Responce Code : " << response_code;
				m_exceptionLoggerObj->LogInfo( logg.str() );
				fclose(output_file);
				curl_easy_cleanup(curl);
				return SUCCESS;
			}
			else 
			{
                curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, 0 );
                res = curl_easy_perform(curl);
                curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &response_code );
                
                if( response_code == 200 && res == CURLE_OK )
                {
                    logg.str("");
                    logg << "PackageManager::GetFile()  GatewayID : " << m_gatewayId << ",  Message : File Downloaded Successfully. Responce Code : " << response_code;
                    m_exceptionLoggerObj->LogInfo( logg.str() );
                    fclose(output_file);
                    curl_easy_cleanup(curl);
                    return SUCCESS;
                }
                else
                {
                    // remove the half or wrong downloaded file.
                    std::string removeCommand = "rm -r " + path;
                    system( removeCommand.c_str() );
                    logg.str("");
                    logg << "PackageManager::GetFile()  GatewayID : " << m_gatewayId << ",  Message : Curl Download file failed. Error code : " << response_code << ", Error Message : " << curl_easy_strerror(res);
                    m_exceptionLoggerObj->LogError( logg.str() );
                }
			}
			
			//fclose(output_file); // crashed in negative case, disabled, unit testing, change, no opened file, and redundant in success case.
			curl_easy_cleanup(curl);
		}
	}
	catch( ... )
	{
		logg.str("");
		logg << "PackageManager::GetFile()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured. ";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	return DOWNLOAD_FAILED; // 
}

/**
 * @brief AddDataIntoMap() 			:	This method will maintain persistency.
 * 										
 * @param nlohmann::json jsonObj	:	It contains install/upgrade package information like app_name, url, token 
 * 										and version.
 */	
void PackageManager::AddDataIntoMap( nlohmann::json jsonObj, int caseId )//try catch
{
	std::stringstream logg;
	try
	{
		std::string version = jsonObj["version"];
		std::string appName = jsonObj[APP_NAME];
		std::string url = jsonObj["url"];

		std::string packageName = GetLastToken(url,"/");
		size_t found = packageName.find('.');
		if ( found != std::string::npos )
		{
			packageName = packageName.substr(0,found);
		}
		
		if( caseId == INSTALL )
		{
			APPDETAILS *appDetailsObj = new APPDETAILS;
			appDetailsObj->appVersion = version;
			appDetailsObj->packageName = packageName;
			m_installedPackageInfoMap[appName] = appDetailsObj;
		}
		else
		{
			auto it = m_installedPackageInfoMap.find(appName);
			if (it != m_installedPackageInfoMap.end() )
			{
				it->second->backupVersion = it->second->appVersion;
				it->second->backupDownloadPath = it->second->appDownloadPath;
				it->second->appVersion = version;
				it->second->packageName = packageName;
				m_persistentJson[appName]["previous_downloaded_path"] = it->second->backupVersion;
				m_persistentJson[appName]["previous_version"] = it->second->appVersion;
				m_persistencyManagerObj->SetConfiguration( m_persistentJson ); // adding vlaues to exising, pesistency config. json 
			}
		}
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "PackageManager::AddDataIntoMap()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "PackageManager::AddDataIntoMap()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured. JSON : " << jsonObj;
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	
}

/**
 * @brief UpdateLibFiles() 			:	This method will copy dependency libs in predefined location.
 * 										
 * @param nlohmann::json jsonObj	:	It contains the dependency lib array.
 * @param std::string appPath		:	It contains the source directory path.
 * 
 */	
void PackageManager::UpdateLibFiles( nlohmann::json jsonObj, std::string appPath )
{
	std::stringstream logg;
	try
	{
		for (auto& x : jsonObj["dependent_libs"].items())
		{
			 std::string libfile = x.value();
			
			 std::string copyCommand = "cp -f " + appPath + "/libs/" + libfile + " " + LIBPATH;
			 
			if( system( copyCommand.c_str() ) == 0 )
			{
				std::cout << "Copy file Successfully : Command : " << copyCommand << "\n\n";
			}
			else
			{
				logg.str("");
				logg << "PackageManager::UpdateLibFiles()  GatewayID : " << m_gatewayId << ",  Message : Copy library file failed. Command : " << copyCommand;
				m_exceptionLoggerObj->LogError( logg.str() );
			}		 
		}
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "PackageManager::UpdateLibFiles()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "PackageManager::UpdateLibFiles()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured. JSON : " << jsonObj;
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief destroy an ReceivePersistencyCommand	:	This method will bind with RegisterPersistencyCB. It will get the  
 * 													persistency data and maintain persistency.
 * 
 * @param nlohmann::json jsonObject 			:	content of the request which is read the data from Persistency file.
 */
void PackageManager::ReceivePersistencyCommand( nlohmann::json jsonObj )
{
	std::stringstream logg;
	try
	{
		if ( jsonObj.is_null() )
		{
			std::string strMessage = "Received empty persistency.";
			SendNotificationToCloud ( strMessage );
		}

		nlohmann::json installedAppInfoJson;
		m_persistentJson = jsonObj["installed_app"];
		for ( auto& x : jsonObj["installed_app"].items() )
		{
			nlohmann::json jsonValueObj = x.value();
			std::string appName = x.key();
			
			if( appName == "GatewayAgent" )
			{
				continue;
			}
			
			installedAppInfoJson[INSTALLED_PACKAGES][appName] = jsonValueObj[VERSION];
			APPDETAILS *appDetailsObj = new APPDETAILS;
			appDetailsObj->appVersion = jsonValueObj[VERSION];
			appDetailsObj->packageName = jsonValueObj["packagename"];
			appDetailsObj->appDownloadPath = jsonValueObj["downloaded_path"];
			appDetailsObj->state = jsonValueObj[STATE];
			appDetailsObj->appJson = jsonValueObj["app_json"];
			
			m_installedPackageInfoMap[appName] = appDetailsObj;
			
			if( !jsonValueObj["previous_version"].empty() )
			{
				appDetailsObj->backupVersion = jsonValueObj["previous_version"];
			}
			else if( !jsonValueObj["backupDownloadPath"].empty() )
			{
				appDetailsObj->backupDownloadPath = jsonValueObj["previous_downloaded_path"];
			}
		}
        
        nlohmann::json startAppJson1;
        startAppJson1[COMMAND] = "START_APP";
        startAppJson1[APP_NAME] = "MQTTAgent";
        StartApplication( startAppJson1 );
                
        nlohmann::json startAppJson2;
        startAppJson2[COMMAND] = "START_APP";
        startAppJson2[APP_NAME] = "CachingAgent";
        StartApplication( startAppJson2 );
        
        for( auto itr1 = m_installedPackageInfoMap.begin(); itr1 != m_installedPackageInfoMap.end(); itr1++ )
        {
            std::string appName = itr1->first;
            APPDETAILS *appDetailsObj = itr1->second;
            
            if ( appDetailsObj->state == 1 && appName != "MQTTAgent" && appName != "CachingAgent" )
			{
				nlohmann::json startAppJson;
				startAppJson[COMMAND] = "START_APP";
				startAppJson[APP_NAME] = appName;
				StartApplication( startAppJson );  
			}
        }
        
		m_sendMsgWrapperObj->SendReportedJsonToCloud( installedAppInfoJson, 6 );// hard-coded, response val,coded
	}
	catch( nlohmann::json::exception &e )
	{
		logg.str("");
		logg << "PackageManager::ReceivePersistencyCommand()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "PackageManager::ReceivePersistencyCommand()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured. JSON : " << jsonObj;
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

/**
 * @brief destroy an RebootGateway	:	This method will reboot the gateway.  
 */
void PackageManager::RebootGateway()
{
	std::stringstream logg;
	logg.str("");
	logg << "PackageManager::RebootGateway()  GatewayID : " << m_gatewayId << ",  Message : Reboot Gateway.";
	m_exceptionLoggerObj->LogInfo( logg.str() );
	std::string cmd1 = "reboot -f";

    sync();
    setuid(0);
	#ifdef TRB_GATEWAY
		system(cmd1.c_str());
	#else
		reboot( RB_AUTOBOOT );
	#endif
}

/**
 * @brief SendNotificationToCloud	:	This method will send any notification to cloud.
 * 
 */ 
void PackageManager::SendNotificationToCloud( std::string messageString )
{
	std::stringstream logg;
	try
	{
		nlohmann::json notificationJson;
		notificationJson[TYPE] = "notification";
		notificationJson[MESSAGE] = messageString;
		notificationJson[TIMESTAMP] = GetTimeStamp();
		
		std::string notificationStr = notificationJson.dump();
		m_sendMsgWrapperObj->SendMessageToCloud( notificationStr.c_str(), "notification" );
	}
	catch( ... )
	{
		logg.str("");
		logg << "PackageManager::SendNotificationToCloud  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

