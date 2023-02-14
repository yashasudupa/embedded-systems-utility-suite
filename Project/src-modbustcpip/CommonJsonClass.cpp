#include "CommonJsonClass.h"

CommonJsonClass* CommonJsonClass::m_commonJsonInstance= nullptr;

CommonJsonClass* CommonJsonClass::GetInstance()
{
	 if ( !m_commonJsonInstance )
	 {
		m_commonJsonInstance = new CommonJsonClass();
	 }
        
	 return m_commonJsonInstance;
}

CommonJsonClass::CommonJsonClass()
{
    
}


CommonJsonClass::~CommonJsonClass()
{
    
}

nlohmann::json CommonJsonClass::GetLatestSensorData()
{
    usleep(10);
    return m_latestDataJson;
}


void CommonJsonClass::SetSensorData( nlohmann::json jsonData )
{
    try
    {
        if( !jsonData.is_null() )
        {
            m_latestDataJson = jsonData;
            std::cout <<"received JSON in CommonJsonClass.m_latestDataJson :  " << m_latestDataJson <<"\n\n";
        }
        
    }
    catch( ... )
    {
        std::cout << "CommonJsonClass::SetSensorData() - Unknown exception occured." << std::endl;
    }
}

void CommonJsonClass::ClearSensorData ( std::string grpName, std::string assetId )
{
    if( m_latestDataJson[assetId].contains(grpName) )
    {
        m_latestDataJson[assetId].erase(grpName);
    }
    
}

void CommonJsonClass::SetTimestamp( std::string slaveId )
{
    try
    {
        if( m_latestDataJson.contains( slaveId ) )
        {
            m_latestDataJson[slaveId][PREVIOUS_TIMESTAMP] = m_latestDataJson[slaveId][TIMESTAMP];
        } 
        else
        {
            std::cout << "CommonJsonClass::SetTimestamp - respected Slave Id data is not available previously.. SlaveId : " << slaveId << "\n\n";
        }
    }
    catch( ... )
    {
        std::cout << "CommonJsonClass::SetTimestamp - Unknown Exception Occured...\n\n";
    }
}