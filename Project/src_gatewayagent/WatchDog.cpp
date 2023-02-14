#include "WatchDog.h"

WatchDog* WatchDog::m_watchDogInstance = nullptr;

WatchDog* WatchDog::GetInstance()
{
	 if (!m_watchDogInstance)
	 {
		m_watchDogInstance = new WatchDog();
	 }
	 return m_watchDogInstance;
}

WatchDog::WatchDog()
{
	m_threadFlg = true;
}

WatchDog::~WatchDog()
{
	m_threadFlg =false;
	if( m_watchDogInstance )
	{
		delete m_watchDogInstance;
	}
}


void WatchDog::StartWatchDogThread()
{
	m_watchDogThreadObj = std::thread([this](){
		while( m_threadFlg ) // use pro
		{
			for (auto itr = m_applicationList.begin(); itr != m_applicationList.end(); itr++)
			{
				std::cout << *itr<<"\n\n";
				long pid = GetProcessIdByName( *itr );
				if( pid == 0 )
				{
					nlohmann::json jsonCommand;
					jsonCommand[COMMAND] = START_APP;
					jsonCommand[APP_NAME] = *itr;	
					m_watchDogCB( jsonCommand );
				}
			}
			sleep(5);
		}});
}

void WatchDog::RegisterApp( std::string processname )
{
	auto it = m_applicationList.find( processname );
	if ( it == m_applicationList.end() )
	{
		std::cout << "RegisterApp successfully : " << processname << std::endl;
		m_applicationList.insert( processname );
	}
}

void WatchDog::DeRegisterApp( std::string processname )
{
	auto it = m_applicationList.find( processname );
	if (it != m_applicationList.end())
	{
		std::cout << "DeRegisterApp successfully : " << processname << std::endl;
		m_applicationList.erase (it);;
	}
}

void WatchDog::RegisterWatchDogCB( std::function<void(nlohmann::json)> cb )
{
	m_watchDogCB = cb;
}