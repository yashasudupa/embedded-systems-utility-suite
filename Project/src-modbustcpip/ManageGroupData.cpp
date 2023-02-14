#include "ManageGroupData.h"

ManageGroupData::ManageGroupData( std::string grpName, std::string deviceId, std::string processName ):
    m_grpName( grpName ),
    m_threadStartStaus( true ),
    m_ThreadRunning( true ),
    m_uploadCountConstant( 1 ),
    m_tempuploadCountConstant( 1 ),
    m_deviceId( deviceId ),
    m_turboTimeoutCountConst(0),
    m_turboTimeoutCount(0),
    m_measurementFreq1(5000),
    m_uploadCount(0)
{
    
    m_commandJson[APP_NAME] = processName;
	m_commandJson[APP_ID] = GetProcessIdByName(processName);
	m_commandJson[COMMAND_TYPE] = RESPONSE;
    
    m_telemetryJsonArray[COMMAND_INFO] = m_commandJson;
	m_telemetryJsonArray[COMMAND_INFO][COMMAND_SCHEMA] = "telemetry";
    m_telemetryJsonArray[DEVICE_ID] = m_deviceId; 
    m_telemetryJsonArray["group_name"] = grpName; 
    m_telemetryJsonArray[TYPE] = MULTI_TELEMETRY;
}


ManageGroupData::~ManageGroupData()
{
    m_ThreadRunning = false;
    m_threadStartStaus = false;
}


void ManageGroupData::SetGroupDetails( GROUP_STRUCT *groupStructObj )
{
    try
    {
        m_groupStructObj = groupStructObj;
        m_measurementFreq = groupStructObj->measuredFrequency;
        m_measurementFreq1 = m_measurementFreq * 1000;
        m_uploadFreq = groupStructObj->uploadFrequency;
        std::cout << "SetGroupDetails m_uploadFreq " << m_uploadFreq << "\n\n"; 
        std::cout << "SetGroupDetails m_measurementFreq " << m_measurementFreq << "\n\n"; 
        std::cout << "SetGroupDetails m_uploadCountConstant " << m_uploadCountConstant << "\n\n"; 
        
        if( m_uploadFreq >= m_measurementFreq )
        {
            m_uploadCountConstant = m_uploadFreq/m_measurementFreq;
            m_tempuploadCountConstant = m_uploadCountConstant;
        }
    }
    catch( nlohmann::json::exception &e )
	{
		std::cout <<"ManageGroupData::SetGroupDetails() : " << e.id << " : " << e.what() << std::endl;
	}
    catch( ... )
	{
		std::cout <<"ReadProperties::SetGroupDetails : Unknown Exception occured." << std::endl;
	}
}

void ManageGroupData::SetDataJson( nlohmann::json dataJson )
{
    m_dataJson = dataJson;
}


void ManageGroupData::SetTurboMode( long turboTimeOut, bool setMode  )
{
    if( setMode == true )
    {
        std::cout << "************turboTimeOut : " << turboTimeOut << "\n\n";
        std::cout << "************m_measurementFreq : " << m_measurementFreq << "\n\n";
        m_uploadCountConstant = 1;
        m_turboTimeoutCountConst = turboTimeOut / m_measurementFreq;
    }
    else
    {
        m_uploadCountConstant = m_tempuploadCountConstant;
    }
}

void ManageGroupData::StartDeviceStateGetterThread()
{
    std::cout << "start StartDeviceStateGetterThread : " <<  "\n\n";
    m_threadObj = std::thread([this](){
        while( m_ThreadRunning )
        {
            try
            {
                if( m_threadStartStaus )
                {

                        nlohmann::json latestData = m_dataJson; //m_commonJsonObj->GetLatestSensorData();                            
                        std::cout << "common JSON : " << latestData << "\n\n";
                        
                        nlohmann::json telemetryJson;
                        bool dataSendFlag = false;
                        
                        if( latestData[m_deviceId].contains(m_grpName) )
                        {
                            telemetryJson = latestData[m_deviceId][m_grpName];
                            telemetryJson["ts"] = latestData[m_deviceId]["ts"];
                            //telemetryJson["group_name"] = m_grpName;
                            m_dataJson.clear();
                        }
                        if( !telemetryJson[MEASURED_PROPERTY_TYPE].is_null() )
                        {
                            m_uploadCount++;
                            
                            if( m_turboTimeoutCountConst > 0 )
                            {
                                m_turboTimeoutCount++;
                            }
                            
                            std::cout << "************m_measurementFreq1 : " << m_measurementFreq1/1000 << "\n\n";
                            std::cout << "************m_uploadCountConstant : " << m_uploadCountConstant << "\n\n";
                            std::cout << "************m_uploadCount : " << m_uploadCount << "\n\n";
                            std::cout << "************telemetryJson : " << telemetryJson << "\n\n\n";
                            if( !telemetryJson.is_null() && m_uploadCount >= m_uploadCountConstant )
                            {
                                
                                m_telemetryJsonArray[LATEST] = telemetryJson;  
                                if(m_propertiesCB)
                                {
                                    m_propertiesCB( m_telemetryJsonArray );
                                }
                                m_telemetryJsonArray[DATA].clear();
                                m_telemetryJsonArray[LATEST].clear();
                                m_uploadCount = 0;
                            }
                            else
                            {
                                m_telemetryJsonArray[DATA].push_back( telemetryJson );
                                dataSendFlag = true; 
                            }
                            
                            if( m_turboTimeoutCount > m_turboTimeoutCountConst  )
                            {
                                m_turboTimeoutCountConst = 0;
                                m_turboTimeoutCount = 0;
                                m_uploadCountConstant = m_tempuploadCountConstant;
                            }
                        }
                }
                usleep( m_measurementFreq1 );
            }
            catch( nlohmann::json::exception &e )
            {
                std::cout <<"StartDeviceStateGetterThread::StartDeviceStateGetterThread() : " << e.id << " : " << e.what() << std::endl;
            }
            catch( ... )
            {
                std::cout <<"ReadProperties::StartDeviceStateGetterThread : Unknown Exception occured." << std::endl;
            }
			
        }
    });	
	
}

void ManageGroupData::RegisterPropertiesCB( std::function<void(nlohmann::json)> cb )
{
	m_propertiesCB = cb;
}