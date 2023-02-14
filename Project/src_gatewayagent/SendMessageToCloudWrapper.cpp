#include "SendMessageToCloudWrapper.h"

SendMessageToCloudWrapper* SendMessageToCloudWrapper::m_sendMsginstance= nullptr;;

SendMessageToCloudWrapper* SendMessageToCloudWrapper::GetInstance( CloudCommunicationWrapper *cloudCommObj )
{
	 if (!m_sendMsginstance)
	 {
		m_sendMsginstance = new SendMessageToCloudWrapper( cloudCommObj );
	 }
	 
	 return m_sendMsginstance;
}

SendMessageToCloudWrapper::SendMessageToCloudWrapper( CloudCommunicationWrapper *cloudCommObj )
{
	m_cloudCommInstance = cloudCommObj;
	//nlohmann::json 
	std::cout << "SendMessageToCloudWrapper::SendMessageToCloudWrapper( CloudCommunicationWrapper *cloudCommObj )" << std::endl;
	
}

SendMessageToCloudWrapper::~SendMessageToCloudWrapper()
{
	
}

int SendMessageToCloudWrapper::SendMessageToCloud( std::string message, std::string schema )
{
	int returnValue = FAILED;
	if( m_cloudCommInstance )
	{
		std::cout << "SendMessageToCloudWrapper::SendMessageToCloud " << std::endl;
		returnValue = m_cloudCommInstance->SendDeviceToCloudMessage( (char*)message.c_str(), (char*)schema.c_str() );
	}
	return returnValue;
}

bool SendMessageToCloudWrapper::UploadFilesToCloud( std::string fileContent, std::string filePathWithFileName )
{
    bool returnValue = false;
    if( m_cloudCommInstance )
	{
		returnValue = m_cloudCommInstance->UploadBlobStorage( fileContent, filePathWithFileName );
	}
    return returnValue;
}

bool SendMessageToCloudWrapper::SendReportedJsonToCloud( nlohmann::json jsonObject, long caseId )
{
	try
	{
		std::string repostedJsonStr = "";
		nlohmann::json tempJson;
		bool sendReportFlag = true;
		switch( caseId )
		{
			case REPORT_ERROR:
			{
				std::string appName = jsonObject[APP_NAME];
				tempJson[SUB_JOB_ID] = jsonObject[SUB_JOB_ID];
				tempJson[appName]["fw_pending_version"] = jsonObject["version"];
				tempJson[appName]["fw_current_version"] = "";
				tempJson[appName]["fw_update_status"] = "error";
				tempJson[appName]["fw_update_sub_status"] = "error";
			}
			break;
			case REPORT_DOWNLOADING:
			{
				std::string appName = jsonObject[APP_NAME];
				tempJson[SUB_JOB_ID] = jsonObject[SUB_JOB_ID];
				tempJson[appName]["fw_pending_version"] = jsonObject["version"];
				tempJson[appName]["fw_current_version"] = "";
				tempJson[appName]["fw_update_status"] = "downloading";
				tempJson[appName]["fw_update_sub_status"] = "Downloading image from internet";
				
			}
			break;
			case REPORT_APPLYING:
			{
				std::string appName = jsonObject[APP_NAME];
				tempJson[SUB_JOB_ID] = jsonObject[SUB_JOB_ID];
				tempJson[appName]["fw_pending_version"] = jsonObject["package_details"]["version"];
				tempJson[appName]["fw_current_version"] = "";
				tempJson[appName]["fw_update_status"] = "applying";
				tempJson[appName]["fw_update_sub_status"] = "Applying image on firmware";
			}
			break;
			case REPORT_INSTALLED:
			{
				std::string appName = jsonObject[APP_NAME];
				tempJson[SUB_JOB_ID] = jsonObject[SUB_JOB_ID];
				tempJson[appName]["fw_pending_version"] = "";
				tempJson[appName]["fw_current_version"] = jsonObject["package_details"]["version"];
				tempJson[appName]["fw_update_status"] = "installed";
				tempJson[appName]["fw_update_sub_status"] = "Successfully installed the firmware.";
			}
			break; 
			case REPORT_ROLLBACK:
			{
				std::string appName = jsonObject[APP_NAME];
				tempJson[SUB_JOB_ID] = jsonObject[SUB_JOB_ID];
				tempJson[appName]["fw_pending_version"] = "";
				tempJson[appName]["fw_current_version"] = jsonObject["package_details"]["version"];
				tempJson[appName]["fw_update_status"] = "rolledback";
				tempJson[appName]["fw_update_sub_status"] = "Successfully rolled back the firmware.";
			}
			break; 
			case REPORT_CURRENT:
			{
				std::string appName = jsonObject[APP_NAME];
				tempJson[SUB_JOB_ID] = jsonObject[SUB_JOB_ID];
				tempJson[appName]["fw_pending_version"] = "";
				tempJson[appName]["fw_current_version"] = jsonObject["package_details"]["version"];
				tempJson[appName]["fw_update_status"] = "current";
				tempJson[appName]["fw_update_sub_status"] = "Successfully installed the firmware.";
			}
			break; 
			case REPORT_GENERIC:
			{
				tempJson = jsonObject;
			}
			break;
			case REPORT_UNINSTALLED:
			{
				std::string appName = jsonObject[APP_NAME];
				tempJson[SUB_JOB_ID] = jsonObject[SUB_JOB_ID];
				tempJson[appName]["fw_pending_version"] = "";
				tempJson[appName]["fw_current_version"] = "";
				tempJson[appName]["fw_update_status"] = "uninstalled";
				tempJson[appName]["fw_update_sub_status"] = "Successfully uninstalled the package.";
			}
			break;
			default:
			sendReportFlag = false;
		}
		
		if( sendReportFlag && m_cloudCommInstance)
		{
			repostedJsonStr = tempJson.dump();
			m_cloudCommInstance->SendReportedState( (char*)repostedJsonStr.c_str() );
			std::cout << "repostedJsonStr : " << repostedJsonStr << "\n\n";	
			return true;
		}
	}
	catch(nlohmann::json::exception &e)
	{
		std::cout << e.id << " : " << e.what() << std::endl;
	}
	
	return false;
}