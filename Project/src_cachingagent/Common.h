#ifndef Common_h
#define Common_h 1
#include "ExceptionLogger.h"
#include <sstream>

#define PUBLISH_PREFIX 							"kemsys/gateway/"
#define CACHER_APP_CACHED_DATA_PREFIX			"/datacatcher/cacheddata"
#define CACHER_APP_CACHED_DATA_REQUEST_PREFIX	"/datacatcher/cacheddata/request"
#define CACHER_APP_CACHED_DATA_UPLOAD_PREFIX	"/datacatcher/cacheddata/upload"
#define CACHER_APP_CACHED_ALERT_POSTFIX			"\",\"type\": \"cached_alert\", \"data\":["
#define CACHER_APP_CACHED_TELE_ALERT_PREFIX		"{\"asset_id\": \""
#define CACHER_APP_CACHED_TELEMETRY_POSTFIX		"\",\"type\": \"cached_telemetry\", \"data\":["

#define JSON_FILE_EXTENTION						".json"



#define DOWNLOADPATH							"/opt/IoT_Gateway/packages/"
#define INSTALLPATH								"/opt/IoT_Gateway/"
#define LIBPATH									"/usr/lib/"
#define LIBDIR									"libs"

#define LOCAL_MQTT_CONFIGURATION_FILE			"/opt/IoT_Gateway/Common/LocalBrokerConfig.json"
#define TWINVERSIONFILE							"version.txt"
#define CONFIGURATIONFILE						"./config/configuration.json"
#define PACKAGE_CONFIG_JSON						"package_config.json"
#define DEVICEINFO								"./config/device_info.json"
#define POLLINGCONFIG							"./config/pollingconfig.json"
#define CACHING_AGENT_LOG_FILE					"CachingAgent_Log"

#define QOS										2

#define CACHED_DATA						        "cached_data"
#define COMMAND_INFO							"commandinfo"
#define COMMAND_TYPE							"commandtype"
#define FILEUPLOAD_REQUEST_RECEIVED				"file_upload_request_received"
#define DEVICE_DATA_BACKUP 						"device_data_backup"
#define COMMAND_SCHEMA							"commandschema"
#define REGISTER								"register"
#define RESPONSE								"response"
#define REQUEST									"request"	
#define CONNECTION_STATUS						"connection_status"	
#define CACHED_TELEMERTY_PATH					"/opt/IoT_Gateway/CachedData/Telemetry/"
#define	DB_TELEMERTY_PATH						"/root/DataBackup/Telemetry/"
#define	DB_PATH									"/root/DataBackup/"
#define TELEMETRY_STRING						"Telemetry"
#define ALERT_STRING							"Alert"
#define CACHED_ALERT_PATH						"/opt/IoT_Gateway/CachedData/Alert/"
#define	DB_ALERT_PATH							"/root/DataBackup/Alert/"
#define APP	                      			    "app"
#define GATEWAY_ID	               			 	"gateway_id"
#define TYPE	                				"type"
#define DEVICE_ID                				"asset_id"
#define FILE_NAME                				"file_name"
#define APP_NAME								"app_name"
#define DATA									"data"
#define LATEST									"latest"
#define CACHED_TELEMETRY						"cached_telemetry"
#define CACHED_ALERT							"cached_alert"
#define DB_TELEMETRY							"backed_up_telemetry"
#define DB_ALERT								"backed_up_alert"

#define MAX_LEN                					80
#define FILE_COUNT								5
#define TELEMETRY								0
#define ALERT									1
#endif