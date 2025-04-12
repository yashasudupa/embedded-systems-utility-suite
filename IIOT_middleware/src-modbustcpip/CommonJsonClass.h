#ifndef CommonJsonClass_h
#define CommonJsonClass_h 1
#include <iostream>
#include <nlohmann/json.hpp>
#include "Common.h"
#include "GlobalOperations.h"

class CommonJsonClass
{
public:
    static CommonJsonClass *GetInstance();
    ~CommonJsonClass();
    nlohmann::json GetLatestSensorData();
    void SetSensorData( nlohmann::json jsonData );
    void SetTimestamp( std::string slaveId );
    void ClearSensorData ( std::string grpName, std::string assetId );
    
private:
    CommonJsonClass();
    
private:
    static CommonJsonClass *m_commonJsonInstance;
    nlohmann::json m_latestDataJson;
};

#endif