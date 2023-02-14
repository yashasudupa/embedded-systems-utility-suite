#ifndef SendMessageToCloudWrapper_h
#define SendMessageToCloudWrapper_h 1

#include <iostream>
#include <string>
#include <string.h>
#include <nlohmann/json.hpp>
#include "Common.h"
#include "CloudCommunicationWrapper.h"
#include "GlobalOperations.h"

class SendMessageToCloudWrapper
{
public:
	~SendMessageToCloudWrapper();
	static SendMessageToCloudWrapper *GetInstance( CloudCommunicationWrapper *cloudCommObj );
	bool SendReportedJsonToCloud( nlohmann::json jsonObj, long caseId );
	int SendMessageToCloud( std::string message, std::string schema );
    bool UploadFilesToCloud( std::string fileContent, std::string filePathWithFileName );
	
private:
	SendMessageToCloudWrapper( CloudCommunicationWrapper *cloudCommObj );
	
private:
	static SendMessageToCloudWrapper *m_sendMsginstance;
	CloudCommunicationWrapper *m_cloudCommInstance;
	
};

#endif
