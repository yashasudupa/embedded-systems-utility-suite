#include "GatewayAgentManager.h"

int main(int argc, char **argv)
{    
	#ifdef BLUENRG_RESET
		time_t start = time(NULL);
		time_t time_monitor = start;
	#endif
    
	#ifdef KEMSYS_GATEWAY
        std::string addResolConf = "echo \"nameserver 8.8.8.8\" >> /etc/resolv.conf";
        system( addResolConf.c_str() );
        sleep(5);
    #endif
	
	std::string configFilePath = "./config/";
	mkdir ( configFilePath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO );
	chmod( configFilePath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO );
	
	configFilePath = "./config/persistency_config";
	mkdir ( configFilePath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO );
	chmod( configFilePath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO );
	
	mkdir ( TEMP_PACKAGE_INSTALLPATH, S_IRWXU | S_IRWXG | S_IRWXO );
	chmod( TEMP_PACKAGE_INSTALLPATH, S_IRWXU | S_IRWXG | S_IRWXO );
	
	ExceptionLogger *exceptionLoggerObj = exceptionLoggerObj->GetInstance();
	
	if( exceptionLoggerObj )
	{
        //Example : GatewayAgent_Log_09-09-2021.log
		exceptionLoggerObj->Init( GATEWAY_AGENT_LOG_FILE, "GatewayAgent" );
	}
	
	// Add log -> application is started and Exception log
	
	GatewayAgentManager *m_gatewayAgentManagerObj = new GatewayAgentManager( argv[0] );
	
	
	if( m_gatewayAgentManagerObj )
	{
		std::stringstream logg;
		logg.str("");
		logg << "main() :  Message : GatewayAgentManager object created Successfully. Gateway Version : " << VERSION_NUMBER;
		exceptionLoggerObj->LogInfo( logg.str() );
	}

	while(1)
	{
		#ifdef BLUENRG_RESET
		//std::cout << "Entering BlueNRG" << std::endl;
		m_gatewayAgentManagerObj->BluenrgMonitor(&time_monitor);
		#endif
		sleep(1);
	}
	
	if( m_gatewayAgentManagerObj )
	{
		delete m_gatewayAgentManagerObj;
	}
	
	if( exceptionLoggerObj )
	{
		delete exceptionLoggerObj;
	}
	
	// Add log ->  Exception log
    return 0;
}
