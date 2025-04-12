#ifndef ASSET_MANAGER
#define ASSET_MANAGER 1

#include "ReadProperties.h"

#define GROUP_G1            "g1"
#define GROUP_G2            "g2"
#define GROUP_G3            "g3"


class AssetManager
{
    
public:
    AssetManager( std::string assetId, std::string processName );   
    ~AssetManager();   
    void SetConfig( nlohmann::json config );
    void SetPropertyJson( nlohmann::json propertyJson );
    void RegisterDevice( nlohmann::json slaveJson );
    void RegisterPropertiesCB( std::function<void(nlohmann::json)> cb );
    void DeRegisterSlaves( nlohmann::json config );
    void SendNotificationToCloud( nlohmann::json config, int caseId );
    void PropertiesReceiver( nlohmann::json jsonObj );
    void LoadProperties();
    void UpdateTwin();
    
     
private:

     
private:
    std::map<std::string, DEVICEINFOSTRUCT*> m_slaveConfigMap; 
    std::string m_assetId;
    std::string m_processName;
    std::string m_configFilePath;
    std::string m_deviceMode;
 //   STORE_THRESHOLD *storeing;
    nlohmann::json m_propertyJson;
    nlohmann::json m_slaveJson;
    nlohmann::json m_commandJson;
    nlohmann::json m_lastReceivedCommandJson;
    
    bool m_startupFlag;
    bool m_flag;
    std::thread m_threadObj;
	double Lower_bound = 0;
	double Upper_bound=0;
    long m_turboTimeout;
    
    std::function<void(nlohmann::json)> m_propertiesCB;
    
    
    
};


#endif