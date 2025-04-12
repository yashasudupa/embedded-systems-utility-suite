#ifndef WatchDog_h
#define WatchDog_h 1

#include <iostream>
#include <stdlib.h>
#include <string>
#include <thread>
#include <list>
#include <string.h>
#include <functional>
#include <nlohmann/json.hpp>


#include "Common.h"
#include "GlobalOperations.h"
#include "SendMessageToCloudWrapper.h"

class WatchDog
{
public:
	
	~WatchDog();
	void StartWatchDogThread();
	void RegisterApp( std::string processname );
	void DeRegisterApp( std::string processname );
	void RegisterWatchDogCB( std::function<void(nlohmann::json)> cb );
	static WatchDog *GetInstance();

private:
	WatchDog();

private:

	static WatchDog *m_watchDogInstance;
	std::set< std::string > m_applicationList;
	std::thread m_watchDogThreadObj;
	std::function<void(nlohmann::json)> m_watchDogCB;
	bool m_threadFlg;
	
};

#endif