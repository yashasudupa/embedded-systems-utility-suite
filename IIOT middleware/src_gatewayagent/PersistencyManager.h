#ifndef PersistencyManager_h
#define PersistencyManager_h 1

#include <iostream>
#include <nlohmann/json.hpp>
#include "Common.h"
#include "GlobalOperations.h"


class PersistencyManager
{
public:
	PersistencyManager();
	~PersistencyManager();
	void SetConfiguration( nlohmann::json jsonObj );
	nlohmann::json MaintainPersistency(ExceptionLogger *m_exceptionLoggerObj);
	
private:

private:
	nlohmann::json m_PersistencyFileContent;
};

#endif
