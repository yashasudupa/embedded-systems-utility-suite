#ifndef Common_h
#define Common_h 1
#include <sstream>
#include <string>
#include <list>
#include <map>
#include "ExceptionLogger.h"
#include "StringParser.h"
#define ESCSEQ						'/'


#define VERSION_NUMBER		"2.0.2"

enum packageManagerStatus { EXCEPTION = -2, FAILED = -1, SUCCESS = 0, DOWNLOAD_FAILED, 
					UNZIP_PACKAGE_FAILED, EMPTY_JSON, KEY_NOT_FOUND, NULL_VALUE_FOUND, 
					PACKAGE_NOT_FOUND, APP_NOT_FOUND, START_APP_FAILED, 
					VERSION_NOT_UPDATED,PROCESS_NOT_PRESENT, FAILED_TO_STOP_PROCESS,SYSTEM_APP};


enum caseId {	INSTALL = 1, UPGRADE, DELETE, START_APPLICATION, STOP_APPLICATION,
				RESTART_APPLICATION, REGISTER_DEVICES_CASE, DEREGISTER_DEVICES_CASE,
				SET_PROPERTIES_CASE, SET_POLLING_CONFIG_CASE, CHANGE_DEVICE_MODE_CASE,
				SET_VALUE_CANAGE_CASE, SET_DEVICE_RULES_CASE, REPLACE_DEVICE_RULES_CASE,
				DELETE_DEVICE_RULES_CASE,REGISTER_SLAVES_CASE,SET_SLAVE_CONFIGURATION_CASE,
                RESET_SENSOR_BOARD_CASE,RESET_MESH_NETWORK_CASE,RESET_SENSOR_CASE,
                SET_BOARD_CONFIGURATION_CASE,RESET_BLUENRG_CASE,SET_POSITION_CONFIGURATION_CASE,
                SET_MOP_CONFIGURATION_CASE,SET_SENSOR_FOTA_CASE, GET_DEVICE_OR_GET_POS_CONFIG_CASE,SYSTEM_REBOOT
            };
            
enum reportTwin {REPORT_ERROR = 0, REPORT_DOWNLOADING, REPORT_APPLYING, 
					REPORT_INSTALLED, REPORT_ROLLBACK, REPORT_CURRENT, REPORT_GENERIC, REPORT_UNINSTALLED };
			
#define FAILURE 		1

struct ResponseStruct
{
	bool status = FAILURE;
	std::string responseMetaInformation;
};

#define PUBLISH_PREFIX 							"kemsys/gateway/"
#define COMMUNICATOR_APP_RESPONSE_PREFIX		"/communicatorapp/response"
#define COMMUNICATOR_APP_REQUEST_PREFIX			"/communicatorapp/request"
#define GATEWAY_AGENT_CONNECTION_PREFIX			"/cloud/connectionstatus"
#define COMMUNCATOR_TO_GW_PREFIX				"/gatewayagent/broadsens"


#ifndef SIEMENS_GATEWAY
    #define UNZIP_COMMAND_PREFIX					"unzip -o "
#else
    #define UNZIP_COMMAND_PREFIX					"busybox unzip -o "
#endif
#define CMD_CRC                                 "cksum "
#define GATEWAY_DEFAULT_PERSISTENCY_CONFIG      "./config/persistency_config/gateway_persistency_default_config.json"    
#define GATEWAY_PERSISTENCY_CONFIG 				"./config/persistency_config/gateway_persistency_config.json"
#define GATEWAYAGENT_PERSISTENCY_CONFIG 		"./config/persistency_config/gatewayagent_self_persistency.json"
#define RULE_PERSISTENCY_CONFIG 		        "./config/persistency_config/ruleconfig.json"
#define GATEWAY_AGENT_LOG_FILE					"GatewayAgent_Log"
#define LOG_FILE_PATH							"./logs/"

#define RESTART_GATEWAY_FILE					"/opt/Gateway_Reboot.txt"
#define DOWNLOADPATH							"/opt/IoT_Gateway/packages/"
#define INSTALLPATH								"/opt/IoT_Gateway/"
#define DOWNLOADPATH_DEFENDER					"/opt/IoT_Defender/"
#define TEMP_PACKAGE_INSTALLPATH				"/opt/IoT_Gateway/TempPackage/"
#define TWINVERSIONFILE							"version.txt"
#define APP_CRCFILE                             "crc.txt"
#define LOCAL_MQTT_CONFIGURATION_FILE			"/opt/IoT_Gateway/Common/LocalBrokerConfig.json"
#define CONFIGURATIONFILE						"./config/configuration.json"
#define PACKAGE_CONFIG_JSON						"package_config.json"
#define LIBPATH									"/usr/lib/"
#define CONFIG_PATH								"./config/"
#define SET_THRESHOLD							"set_threshold"

#define GATEWAY_REBOOT_PERSISTENCY_CONFIG	    "./config/persistency_config/gateway_reboot_persistency_config.json"

#define LIBDIR									"libs"

#define START_APP						"start_app"
#define STOP_APP						"stop_app"
#define RESTART_APP						"restart_app"
#define RESTART_GATEWAY					"restart_gateway"
#define INSTALL_PACKAGE					"install_package"
#define INSTALLED_PACKAGES				"installed_packages"
#define SYSTEM_APPS						"system_apps"
#define CUSTOM_APPS						"apps"
#define UPGRADE_PACKAGE					"upgrade_package"
#define DELETE_PACKAGE					"delete_package"
#define UPLOAD_LOG_FILES				"upload_log_files"
#define TEST_CONNECTION_GATEWAY			"test_gateway_connection"
#define STATUS							"status"
#define MQTT_DETAILS					"mqtt_details"

#define INSTALL_NGROK					"install_ngrok"
#define UNINSTALL_NGROK					"uninstall_ngrok"

#define REBOOT_TIMER                    "reboot_timer"
#define SYSTEM_REBOOT_TIME			    "system_reboot_time"


#define TYPE 							"type"
#define APP_NAME						"app_name"
#define APP_ID							"appid"
#define VERSION							"version"
#define URL								"url"
#define TOKEN							"token"
#define COMMAND							"command"
#define DEVICE_DATA_BACKUP 				"device_data_backup"
#define FILEUPLOAD_REQUEST_RECEIVED		"file_upload_request_received"
#define COMMAND_INFO					"commandinfo"
#define COMMAND_TYPE					"commandtype"
#define COMMAND_SCHEMA					"commandschema"
#define FILE_NAME       				"file_name"
#define MESSAGE              	 		"message"
#define EVENT               			"event"
#define TIMESTAMP               		"timestamp"
#define STATE 							"state"
#define DOLLER_VERSION 					"$version"
#define PACKAGE_DETAILS 				"package_details"
#define CONNECTION_STATUS 				"connection_status"

#define DESIRED							"desired"
#define PACKAGE_MANAGEMENT				"package_management"
#define MEMORY_AVAILABLE        		"Memory Available"
#define RAM_USAGE              			"ram_usage"
#define CPU_USAGE              			"cpu_usage"
#define SYSTEM_PATH             		"./"
#define GATEWAY_ID	            		"gateway_id"
#define CHANGE_DEVICE_MODE	    		"change_asset_mode"
#define SET_POLLING_CONFIG				"set_polling_info"
#define SET_CONFIGURATION			    "set_asset_configuration"
#define REGISTER_DEVICES				"register_assets"
#define REGISTERED_DEVICES				"registered_devices"
#define DEREGISTER_DEVICES				"deregister_assets"
#define SET_PROPERTIES					"set_properties"
#define SET_VALUE_CANAGE				"set_change_value_state"
#define DEVICE_ID               		"asset_id"
#define SET_DEVICE_RULES               	"set_asset_rules"
#define REPLACE_DEVICE_RULES            "replace_device_rules"
#define DELETE_DEVICE_RULES             "delete_device_rules"
#define QOS								2
#define UPDATE_FLAG						"update_flag"
#define BUFF_TIME						1800

#define CLOUD_DETAILS					"cloud_details"
#define HOST_NAME						"HostName"
#define CLOUD_APP						"app"
#define DEVICEID						"DeviceId"
#define SHARED_ACCESS_KEY				"SharedAccessKey"
#define CORELATION_ID					"correlation_id"
#define REPORTED_TWIN					"reported_twin"
#define RULE_ENGINE						"rule_engine"
#define CACHED_TELEMETRY				"cached_telemetry"
#define CACHED_ALERT    				"cached_alert"
#define TELEMETRY_STRING				"telemetry"
#define REGISTER						"register"
#define RESPONSE						"response"
#define REQUEST							"request"
//#define DEVICES							"devices" // old
#define DEVICES							"assets"
#define SUB_JOB_ID                      "sub_job_id"
#define CONFIGURATION                   "configuration"
#define SET_SENSOR_FOTA_MODE             "set_fota_mode"

#define REGISTER_SLAVES                     "register_slaves"
#define DEREGISTER_SLAVES                   "deregister_slaves"
#define RESET_SENSOR_BOARD                  "reset_sensor_board"
#define RESET_MESH_NETWORK                  "reset_mesh_network"
#define RESET_SENSOR                        "reset_sensor"
#define SET_BOARD_CONFIGURATION             "set_board_configuration"
#define SET_SLAVE_CONFIGURATION             "set_slave_configuration"
#define GET_DEVICE_CONFIGURATION			"get_device_configuration"
#define READ_SENSOR_POSITION_CONFIGURATION  "read_sensor_positioning_configuration"
#define RESET_BLUENRG                       "reset_bluenrg"
#define SET_POSITION_CONFIGURATION          "set_sensor_pos_config"
#define SET_MOP_CONFIGURATION               "set_sensor_mop"
#define READ_SENSOR_PROPERTY               "read_sensor_property"



#endif
