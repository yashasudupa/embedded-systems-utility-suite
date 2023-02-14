#include "PersistencyManager.h"

PersistencyManager::PersistencyManager()
{
	
}

PersistencyManager::~PersistencyManager()
{
	
}

void PersistencyManager::SetConfiguration( nlohmann::json jsonObj )
{
	if( !jsonObj.is_null() )
	{
		m_PersistencyFileContent["installed_app"] = jsonObj;
		WriteConfiguration( GATEWAY_PERSISTENCY_CONFIG, m_PersistencyFileContent );
	}
	else
	{
		
	}
	std::cout << " ******* m_PersistencyFileContent : " << m_PersistencyFileContent << "\n\n********";
}

nlohmann::json PersistencyManager::MaintainPersistency(ExceptionLogger *m_exceptionLoggerObj)
{
	std::stringstream logg;
    
	m_PersistencyFileContent = ReadAndSetConfiguration( GATEWAY_PERSISTENCY_CONFIG );
	if( !m_PersistencyFileContent.is_null() )
	{ 
		std::cout<<"\n[GatewayAgent]m_PersistencyFileContent: ok\n";
		}
	else
	{
		// use default configuration to start
		m_PersistencyFileContent = ReadAndSetConfiguration( GATEWAY_DEFAULT_PERSISTENCY_CONFIG );
		
		logg.str("");
	    logg << "PersistencyManager::MaintainPersistency: " << ",  Message : using default persistancy configurations.";
		m_exceptionLoggerObj->LogInfo( logg.str() );
		
		if( m_PersistencyFileContent.is_null() )
		{
		    std::cout<<"\n[GatewayAgent]m_PersistencyFileContent: Default configuartions misssing,\n";

            std::cout<<"\n[GatewayAgent]Failed to recover persistancy from defaul: failed\n";
            logg.str("");
            logg << "Persistancy::PersistencyManager::MaintainPersistency " << ",  Message : PersistencyManager object creation failed.";
            m_exceptionLoggerObj->LogError( logg.str() );
		}
        else
        {
			
			// cp default config, json data to cmain config.
			
			std::string main_cfg = GATEWAY_PERSISTENCY_CONFIG;
							   
			std::string def_cfg =  GATEWAY_DEFAULT_PERSISTENCY_CONFIG;
			
			std::string cfg_bck_cmd = "cp -f " + def_cfg + " " + main_cfg;
							   
			int er = system( cfg_bck_cmd.c_str());
			
			if(er != 0)
			   {
                      std::cout<<"\n[GatewayAgent]Failed to recover persistancy from defaul: failed\n";
					   logg.str("");
					   logg << "Persistancy::PersistencyManager::MaintainPersistency " << ",  Message : PersistencyManager object creation failed.";
					   m_exceptionLoggerObj->LogError( logg.str() );
			   }
		}
			
		
	}
	return m_PersistencyFileContent;
}
