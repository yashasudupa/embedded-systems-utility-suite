#ifndef PackageManager_h
#define PackageManager_h 1

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <curl/easy.h>
#include <dirent.h>

#include "SendMessageToCloudWrapper.h"
#include "AppHealthMonitoring.h"
#include "PersistencyManager.h"
#include "GlobalOperations.h"
#include "ExceptionLogger.h"
#include "WatchDog.h"
#include "Common.h"


enum appstate{ NOT_AVAILABLE=-1,STOP=0,START,INSTALLED };

typedef struct AppDetails 
{
    std::string appDownloadPath;
	std::string packageName;
	std::string appVersion;
	std::string backupVersion;
	std::string backupDownloadPath;
	std::string device_id;
    long appPID;
	nlohmann::json appJson;
	appstate state;
} APPDETAILS;

class PackageManager
{
public:
	PackageManager( std::string gatewayId, std::string cloudAppName, std::string processName );
	~PackageManager();
	
	ResponseStruct InstallPackage ( nlohmann::json jsonObj ); // json insted of string;
	ResponseStruct UpgradePackage ( nlohmann::json jsonObj );
	ResponseStruct UninstallPackage ( nlohmann::json jsonObj );
	ResponseStruct StartApplication ( nlohmann::json jsonObj );
	ResponseStruct StopApplication ( nlohmann::json jsonObj );
	ResponseStruct RestartApplication ( nlohmann::json jsonObj );
	void ReceivePersistencyCommand( nlohmann::json jsonObj );
	void RebootGateway ();

private:	
	void CopyConfigFiles( std::string appName, std::string destinationPath );
	void AddDataIntoMap( nlohmann::json jsonObj, int caseId );
	void UpdateLibFiles( nlohmann::json jsonObj, std::string appPath  );
	void InitPackageManager();
	
	int UnzipPackage( nlohmann::json jsonObj ); // unzip + install or upadate
	int DownloadPackage( nlohmann::json jsonObj );
	int GetFile( std::string url, std::string path );
	int ValidateJson( nlohmann::json jsonObj, int caseId );
	void SendNotificationToCloud( std::string messageString );
	ResponseStruct UpdateGatewayAgent( nlohmann::json jsonObj );
	
	friend class PersistencyManager;
private:
	std::map <std::string, APPDETAILS*> m_installedPackageInfoMap;
	
	std::string m_gatewayId;
	std::string m_cloudAppName;
	std::string m_processName;
	
	PersistencyManager *m_persistencyManagerObj;
	AppHealthMonitoring *m_appHealthMonitoringObj;
	nlohmann::json m_persistentJson;
	WatchDog *m_watchDogObj;
	SendMessageToCloudWrapper *m_sendMsgWrapperObj;
	ExceptionLogger *m_exceptionLoggerObj; 
};

#endif
