#include <stdio.h>
#include <unistd.h>
#include "CommunicatorManager.h"
#include "ExceptionLogger.h"

int main(int argc, char **argv)
{
	ExceptionLogger *exceptionLoggerObj = exceptionLoggerObj->GetInstance();
	
	if( exceptionLoggerObj )
	{
		//logg
		exceptionLoggerObj->Init( COMMUNICOTOR_AGENT_LOG_FILE, "MQTTAgent" );
	}
	else
	{
		
	}
	
	std::string getwayID = argv[1];
	std::cout << getwayID << std::endl;
	CommunicatorManager *communicationManagerObj = new CommunicatorManager( getwayID );
	
	if( communicationManagerObj )
	{
		exceptionLoggerObj->LogInfo("CommunicationApp :: Main  Message : CommunicatorManager object create successfully");
	}
	else
	{
		exceptionLoggerObj->LogError("CommunicationApp :: Main  Message : CommunicatorManager object creation failed.");
	}
	
	while(1)
	{
		sleep(1);
	}

	return 0;
}
