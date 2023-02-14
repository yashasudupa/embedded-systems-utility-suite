#include "DeviceAppManager.h"

DeviceAppManager::DeviceAppManager( std::string processName, std::string gatewayId, std::string libName ):
			m_gatewayId( gatewayId ),
			m_processName( processName ),
			m_libName( libName )
{
	m_exceptionLoggerObj = m_exceptionLoggerObj->GetInstance();
	InitDeviceAppManager();
}

DeviceAppManager::~DeviceAppManager()
{
	if ( m_deviceManagerObj )
	{
		delete m_deviceManagerObj;
	}
	
	if ( m_localBrokerObj )
	{
		delete m_localBrokerObj;
	}
}

void DeviceAppManager::InitDeviceAppManager()
{
	std::stringstream logg;
	try
	{
		m_publishTopic = KEMSYS_PREFIX + m_gatewayId + DEVICE_APP_RESPONSE_PREFIX;
		m_localBrokerObj = new LocalBrokerCommunicationManager();
		m_appId = GetProcessIdByName( m_processName );
		if( m_localBrokerObj )
		{
			logg << "DeviceAppManager::InitDeviceAppManager()  GatewayID : " << m_gatewayId << ",  Message : LocalBrokerCommunicationManager object created Successfully.";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			RegisterApp( m_processName );
			std::string appIdStr = std::to_string( m_appId ); 
			std::string subRequestTopic = KEMSYS_PREFIX + m_gatewayId + DEVICE_APP_PREFIX+ appIdStr +REQUEST_PREFIX;
			m_localBrokerObj->RegisterCB( std::bind( &DeviceAppManager::ReceiveSubscribedData, this, std::placeholders::_1) );
			m_localBrokerObj->SubscribeTopic( subRequestTopic );
		}
		else
		{
			logg << "DeviceAppManager::InitDeviceAppManager()  GatewayID : " << m_gatewayId << ",  Message : LocalBrokerCommunicationManager object creation Failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
		
		m_deviceManagerObj = new DeviceManager( m_libName, m_processName );
		if( m_deviceManagerObj )
		{
			logg << "DeviceAppManager::InitDeviceAppManager()  GatewayID : " << m_gatewayId << ",  Message : DeviceManager object created Successfully.";
			m_exceptionLoggerObj->LogInfo( logg.str() );
			m_deviceManagerObj->RegisterAppCB(std::bind( &DeviceAppManager::RegisterDeviceId, this, std::placeholders::_1));
			m_deviceManagerObj->DeviceDataCB(std::bind( &DeviceAppManager::DeviceDataReceive, this, std::placeholders::_1));
			//TBD : check file is present or not
			m_deviceManagerObj->Discoverdevice("", false);
		}
		else
		{
			logg << "DeviceAppManager::InitDeviceAppManager()  GatewayID : " << m_gatewayId << ",  Message : DeviceManager object creation Failed.";
			m_exceptionLoggerObj->LogError( logg.str() );
		}
		SendNotificationToCloud();
	}
	catch(nlohmann::json::exception &e)
	{
		logg.str("");
		logg << "DeviceAppManager::InitDeviceAppManager()  GatewayID : " << m_gatewayId << ",  Message : Error code :  " << e.id << " Error Messag : " << e.what();
		m_exceptionLoggerObj->LogException( logg.str() );
	}
	catch( ... )
	{
		logg.str("");
		logg << "DeviceAppManager::InitDeviceAppManager()  GatewayID : " << m_gatewayId << ",  Message : Unknown exception occured.";
		m_exceptionLoggerObj->LogException( logg.str() );
	}
}

void DeviceAppManager::SendNotificationToCloud()
{
	nlohmann::json connectionFailJson;

	connectionFailJson[TYPE] = "notification";
	connectionFailJson[MESSAGE] = m_processName + " Application has restarted";
	connectionFailJson[EVENT] = "application_restart";
	connectionFailJson[TIMESTAMP] = GetTimeStamp();
	connectionFailJson[COMMAND_INFO][COMMAND_TYPE] = "response";
	connectionFailJson[COMMAND_INFO][APP_NAME] = m_processName;
	connectionFailJson[COMMAND_INFO][APP_ID] = m_appId;
	std::string publishMsg = connectionFailJson.dump();
	m_localBrokerObj->PublishData( publishMsg.c_str(), m_publishTopic );
}

void DeviceAppManager::RegisterDeviceId( nlohmann::json registerDeviceJson )
{
	registerDeviceJson[COMMAND_INFO][APP_NAME] = m_processName;
	registerDeviceJson[COMMAND_INFO][APP_ID] = m_appId;
	std::string registerDeviceTopic = KEMSYS_PREFIX + m_gatewayId + DEVICE_APP_REGISTER_PREFIX;
	m_localBrokerObj->PublishData( registerDeviceJson.dump(),registerDeviceTopic );
}

void DeviceAppManager::RegisterApp( std::string deviceId )
{
	nlohmann::json registerAppJson;
	registerAppJson[COMMAND_INFO][APP_NAME] = m_processName;
	registerAppJson[COMMAND_INFO][APP_ID] = m_appId;
	registerAppJson[COMMAND_INFO][COMMAND_TYPE] = APP_REGISTER;
	
	std::string registerDeviceTopic = KEMSYS_PREFIX + m_gatewayId + DEVICE_APP_REGISTER_PREFIX;
	m_localBrokerObj->PublishData( registerAppJson.dump(),registerDeviceTopic);
}

void DeviceAppManager::DeviceDataReceive( nlohmann::json deviceDataJson )
{
	
	std::string data = deviceDataJson.dump();	
	if ( m_localBrokerObj )
	{
		m_localBrokerObj->PublishData( data.c_str(), m_publishTopic );
	}
}

void DeviceAppManager::ReceiveSubscribedData( std::string data )
{
	nlohmann::json jsonObj = nlohmann::json::parse( data );
	m_deviceManagerObj->SetConfig( jsonObj );
}