#include <stdio.h>
#include <unistd.h>
#include "DeviceAppManager.h"


int main(int argc, char **argv)
{
	ExceptionLogger *exceptionLoggerObj = exceptionLoggerObj->GetInstance();
	
	if( exceptionLoggerObj )
	{
		exceptionLoggerObj->Init( DEVICE_APP_LOG_FILE, "DeviceApplication" );
	}
	
	std::string processName = GetLastToken( argv[0],"/" );
	std::string gatewayId = argv[1];
	std::string libName = argv[2];
	DeviceAppManager *deviceAppManagerObj = new DeviceAppManager( processName, gatewayId, libName );
	
	if( deviceAppManagerObj )
	{
		exceptionLoggerObj->LogInfo("DeviceApplication :: Main()  Message : DeviceAppManager object create successfully");
	}
	else
	{
		exceptionLoggerObj->LogError("DeviceApplication :: Main()  Message : DeviceAppManager object creation failed.");
	}
	
	while(1)
	{
		sleep(1);
	}
	
	if( deviceAppManagerObj )
	{
		delete deviceAppManagerObj;
	}
	return 0;
}
