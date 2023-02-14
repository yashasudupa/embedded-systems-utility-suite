#ifndef Common_h
#define Common_h 1
#include <sstream>
#include "ExceptionLogger.h"
#define FAILURE 		1

enum communicationAppStatus { EXCEPTION = -2, FAILED = -1, SUCCESS = 0, EMPTY_JSON, 
						KEY_NOT_FOUND, NULL_VALUE_FOUND, NOT_REGISTERD,  
						APP_NOT_FOUND, PROCESS_NOT_PRESENT,FAILED_TO_STOP_PROCESS, APP_NAME_NOT_FOUND };
						
enum communicationAppCaseID { APP_REGISTER_CASE = 0, REQUEST_CASE, RESPONSE_CASE };


struct ResponseStruct
{
	bool status = FAILURE;
	std::string responseMetaInformation;
};

#define DOWNLOADPATH				"/opt/IoT_Gateway/packages/"
#define INSTALLPATH					"/opt/IoT_Gateway/"
#define TWINVERSIONFILE				"version.txt"
#define CONFIGURATIONFILE			"configuration.json"
#define PACKAGE_CONFIG_JSON			"package_config.json"
#define LIBPATH						"/usr/lib/"
#define LIBDIR						"libs"

#define COMMUNICATIONAPP_PERSISTENCY_CONFIG 	"./config/persistency_config/communicationApp_persistency_config.json"
#define LOCAL_MQTT_CONFIGURATION_FILE			"/opt/IoT_Gateway/Common/LocalBrokerConfig.json"

#define QOS										2

#define PUBLISH_PREFIX 										"kemsys/gateway/"
#define DEVICEAPP_REGISTER_PREFIX 							"/deviceapp/register"
#define DEVICEAPP_RESPONSE_PREFIX 							"/deviceapp/response"
#define COMMUNICATORAPP_REQUEST_PREFIX 						"/communicatorapp/request"
#define COMMUNICATORAPP_RESPONSE_PREFIX 					"/communicatorapp/response"
#define CLOUD_CONNECTIONSTATUS_PREFIX 						"/cloud/connectionstatus"
#define DATACACHER_CACHED_DATA_UPLOAD_PREFIX 				"/datacatcher/cacheddata/upload"
#define DATACACHER_CACHED_DATA_PREFIX 						"/datacatcher/cacheddata"
#define DATACACHER_CACHED_DATA_REQUEST_PREFIX 				"/datacatcher/cacheddata/request"
#define BROADSENS_TO_COMMUNICATOR_PREFIX					"/communicatorapp/broadsens"
#define COMMUNCATOR_TO_GW_PREFIX							"/gatewayagent/broadsens"
#define BROADSENS_RESPONSE								    "br_sns_response"
#define RULE_ENGINE_RESPONSE_PREFIX 						"/rule_engine/response"
#define RULE_ENGINE_REQUEST_PREFIX 							"/rule_engine/request"
#define COMMUNICATOR_RULE_ENGINE_RESPONSE_PREFIX 			"/communicatorapp/rule_device/response"
#define REQUEST_PREFIX 										"/request"
#define DEVICEAPP_PREFIX 									"/deviceapp/"


#define COMMUNICOTOR_AGENT_LOG_FILE		"MQTTAgent_Log"
#define COMMAND							"command"
#define COMMAND_INFO					"commandinfo"
#define COMMAND_TYPE					"commandtype"
#define DEVICE_DATA_BACKUP 				"device_data_backup"
#define REGISTER						"register"
#define RESPONSE						"response"
#define REQUEST							"request"
#define CONNECTION_STATUS				"connection_status"		

#define TYPE 							"type"
#define MESSAGE               			"message"
#define EVENT               			"event"
#define TIMESTAMP              			"timestamp"
#define DEVICE_ID                		"asset_id"
#define APP_NAME						"app_name"
#define APP_ID							"appid"

#define REGISTER_DEVICES				"register_assets"
#define REGISTERED_DEVICES				"registered_devices"
#define DEREGISTER_DEVICES				"deregister_assets"
#define CHANGE_DEVICE_MODE				"change_asset_mode"
#define SET_POLLING_INFO				"set_polling_info"
#define SET_CONFIGURATION			    "set_asset_configuration"
#define SET_PROPERTIES					"set_properties"
#define CACHED_DATA						"cached_data"
#define APPS							"apps"
#define SET_VALUE_CANAGE				"set_change_value_state"
#define RULE_ENGINE_DATA				"rule_engine"
#define STATUS							"status"
#define NOTIFICATION					"notification"
#define C2DMESSAGE						"c2dmessage"
#define CORELATION_ID					"correlation_id"

#define SET_DEVICE_RULES               	"set_asset_rules"
#define REPLACE_DEVICE_RULES            "replace_device_rules"
#define DELETE_DEVICE_RULES             "delete_device_rules"
#define SUB_JOB_ID                      "sub_job_id"

#define REGISTER_SLAVES                     "register_slaves"
#define DEREGISTER_SLAVES                   "deregister_slaves"
#define RESET_SENSOR_BOARD                  "reset_sensor_board"
#define RESET_MESH_NETWORK                  "reset_mesh_network"
#define RESET_SENSOR                        "reset_sensor"
#define SET_BOARD_CONFIGURATION             "set_board_configuration"
#define SET_SLAVE_CONFIGURATION             "set_slave_configuration"
#define READ_SENSOR_PROPERTY               "read_sensor_property"
#define SET_FOTA_MODE                      "set_fota_mode"

#endif
