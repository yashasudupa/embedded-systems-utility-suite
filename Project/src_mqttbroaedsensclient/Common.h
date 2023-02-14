#ifndef Common_h
#define Common_h 1
#include <sstream>
#include "ExceptionLogger.h"
#include <nlohmann/json.hpp>
#define FAILURE 		1

enum communicationAppStatus { EXCEPTION = -2, FAILED = -1, SUCCESS = 0, EMPTY_JSON, 
						KEY_NOT_FOUND, NULL_VALUE_FOUND, NOT_REGISTERD,  
						APP_NOT_FOUND, PROCESS_NOT_PRESENT,FAILED_TO_STOP_PROCESS, APP_NAME_NOT_FOUND };
						
enum communicationAppCaseID { APP_REGISTER_CASE = 0, REQUEST_CASE, RESPONSE_CASE };


/*

	0x00: SVT100-T temperature sensor
	0x01: SVT200-V real-time vibration sensor
	0x10: SVT100-A and SVT200-A sensor acceleration
	0x11: SVT100-A and SVT200-A sensor temperature 
	0x00: SVT100-T temperature sensor
	0x01: SVT200-V real-time vibration sensor
	0x10: SVT100-A and SVT200-A sensor acceleration
	0x11: SVT100-A and SVT200-A sensor temperature

*/

enum SensorTypes
		{
			SVT100_T=0x00, SVT200_V=0x01, SVT100_A_SVT200_A=0x10, 
			SVT101_A_SVT201_A=0x11, SVT100_T_TS=0x20, SVT200_V_VS=0x21, 
			SVT100_A_SVT200_A_0x10=0x30, SVT100_A_SVT200_A_0x11=0x31};


struct ResponseStruct
{
	bool status = FAILURE;
	std::string responseMetaInformation;
};


//ModbusTCP

#define DERIVED_PROPERTIES					"edge_derived_properties"
#define G1_MEASUREMENT_FREQUENCY		    "g1_measurement_frequency_in_ms"
#define G2_MEASUREMENT_FREQUENCY		    "g2_measurement_frequency_in_ms"
#define G3_MEASUREMENT_FREQUENCY		    "g3_measurement_frequency_in_ms"

#define G1_INGESTION_FREQUENCY		        "g1_ingestion_frequency_in_ms"
#define G2_INGESTION_FREQUENCY		        "g2_ingestion_frequency_in_ms"
#define G3_INGESTION_FREQUENCY		        "g3_ingestion_frequency_in_ms"

#define G1_TURBO_MODE_FREQUENCY		        "g1_turbo_mode_frequency_in_ms"
#define G2_TURBO_MODE_FREQUENCY		        "g2_turbo_mode_frequency_in_ms"
#define G3_TURBO_MODE_FREQUENCY		        "g3_turbo_mode_frequency_in_ms"

#define HOST_ADDRESS				    "host_address"
#define PORT_NUMBER					    "port_number"
#define SLAVE_ID					    "slave_id"

#define SLAVEID					        "sid"
#define PREVIOUS_TIMESTAMP				"previous_timestamp"
#define GROUP_NAME					    "g"

#define DOWNLOADPATH				    "/opt/IoT_Gateway/packages/"
#define INSTALLPATH					    "/opt/IoT_Gateway/"
#define LIBPATH						    "/usr/lib/"
#define LIBDIR						    "libs"

#define COMMAND_INFO				    "commandinfo"
#define COMMAND_TYPE				    "commandtype"
#define COMMAND_SCHEMA				    "commandschema"
#define REGISTER					    "register"
#define RESPONSE					    "response"

#define REGISTER_SLAVES                     "register_slaves"
#define DEREGISTER_SLAVES                   "deregister_slaves"

#define REGISTER_DEVICES_JSON		    "register_devices.json"
#define POLLING_CONFIG_JSON			    "polling_config.json"
#define PACKAGE_CONFIG_JSON			    "package_config.json"

#define TWINVERSIONFILE				    "version.txt"
#define CONFIGURATIONFILE			    "./config/configuration.json"
#define DEVICEINFO_PATH				    "./config/device_info.json"
#define POLLINGCONFIG_PATH			    "./config/pollingconfig.json"
#define COMMAND					    "command"

#define CHANGE_DEVICE_MODE			    "change_asset_mode"
#define SET_CONFIGURATION			    "set_asset_configuration"
#define ALL_PROPERTIES			        "all_props_at_fixed_interval"
#define CHANGED_PROPERTIES			    "changed_props_at_fixed_interval"
#define DEVICE_CONFIGURATION_JSON	    "device_configuration.json"
#define SET_THRESHOLD					"set_threshold"

#define REGISTER_DEVICES			    "register_assets"
#define SET_VALUE_CANAGE			    "set_change_value_state"

#define POLLING_FREQUENCY			    "polling_frequency"
#define UPLOAD_FREQUENCY			    "upload_frequency"

#define TYPE 						    "type"
#define DATA 						    "data"
#define LATEST 						    "latest"
#define MESSAGE               		    "message"
#define EVENT               		    "event"
#define TIMESTAMP              		    "timestamp"
#define DEVICE_ID                	    "asset_id"
#define APP_NAME					    "app_name"
#define APP_ID						    "appid"
#define MULTI_TELEMETRY				    "telemetry"
#define MULTI_ALERT					    "multi_alert"
#define ALERT						    "alert"
#define CODE						    "code"
#define ALERT_MSG_DATA                  "alert_message_date"

#define COMMFAIL					    false
#define CONNECTED					    true

#define PROPERTIES					        "properties"

#define PROPERTY_NAME				        "pn"
#define DATA_TYPE					        "d"
#define SECONDARY_DATA_TYPE			        "sd"
#define START_ADDRESS				        "sa"
#define ALARM						        "a"
#define LAST_ADDRESS				        "la"
#define PRECISION					        "p"
#define FUNCTION_CODE				        "fc"
#define BIT_NUMBER					        "bn"
#define VALUE						        "v"
#define QOS							        2
#define SUB_JOB_ID					        "sub_job_id"
#define STATUS						        "status"
#define ALERT_START_EVENT			        "alert_start_event"


#define DEVICES				                "assets" //new
#define ASSET_CONFIGURATION				    "asset_configuration" //new
#define CONFIGURATION				        "configuration"
#define TURBO_MODE_TIMEOUT			        "turbo_mode_timeout_in_milli_sec"
#define TURBO_MODE_FREQUENCY		        "turbo_mode_frequency_in_milli_sec"
#define MEASUREMENT_FREQUENCY		        "measurement_frequency_in_milli_sec"
#define TELEMETRY_MODE				        "telemetry_mode"
#define INGESTION_SETTINGS_TYPE			    "ingestion_settings_type"
#define INGESTION_SETTINGS_FREQUENCY	    "ingestion_settings_frequency_in_milli_sec"
#define DERIVED_PROPERTY_TYPE		        "ed"
#define MEASURED_PROPERTY_TYPE		        "m"

#define SET_SLAVE_CONFIGURATION             "set_slave_configuration"

#define LOWER_BOUND							"lower_bound"
#define UPPER_BOUND							"upper_bound"

enum _Data_Types_ 
{
	DATA_CHAR = 0 , 
	DATA_SIGNED_16 = 1,
	DATA_UNSIGNED_16 = 2,
	DATA_LONG=3,
	DATA_LONG_REVERSE=4,
	DATA_FLOAT = 5,
	DATA_FLOAT_REVERSE = 6,
	DATA_SIGNED_64 = 7,
	DATA_UNSIGNED_64 = 8,
    DATA_BYTE = 9,
    DATA_DECIMAL_FLOAT= 10
};

enum DATATYPE { ANALOG = 'a', DIGITAL = 'd', STRING = 's' };


typedef struct commandStruct
{
	short wordQuantity;
	short prevWordQuantity;
	int functionCode;
	int dataType;
	int blockNo;	// User specifies block no for reading a block of variables together
	long strtAddr;
	long lastAddr;
	
}CMD_STRUCT;

typedef union ValueType
{
    double analogValue;     
    bool digitalValue; 
    char stringValue[1000];
}VALUE_TYPE; 

typedef struct propertiesStruct
{
	std::string tagName;
	std::string grpName;
	char datatype;
	short secondaryDatatype ;
	float scaling;
	long startAddress ;
	long lastAddress ;
	long functionCode ;
	bool isAlarm;
	short precision;
	short blockNumber;
	short bitNumber;
	VALUE_TYPE *vt;

} PROPERTIES_STRUCT;


typedef struct groupStruct
{
	long measuredFrequency = 5000;//in milisecond
	long uploadFrequency = 10000;//in milisecond
	long turboModeFrequency = 5000;//in milisecond
	long turboTimeout = 5000;//in milisecond
    bool changeValueState = false;
	
}GROUP_STRUCT;


typedef struct DeviceInfo 
{
	void *readPropertiesObj = NULL;
	nlohmann::json deviceProperties;
	nlohmann::json connectionJson;
	std::string processName;
    std::string assetId;
	

} DEVICEINFOSTRUCT;





//MQTT Broiadsens Client

#define COMMUNICATIONAPP_PERSISTENCY_CONFIG 	"./config/persistency_config/communicationApp_persistency_config.json"
#define LOCAL_MQTT_CONFIGURATION_FILE			"/opt/IoT_Gateway/Common/BroadsensLocalBrokerConfig.json"
#define LOCAL_MQTT_SNS_FILE			"/opt/IoT_Gateway/Common/BroadsensBrokerConfig.json"

#define QOS										1

#define PUBLISH_PREFIX 										"kemsys/gateway/"
#define DEVICEAPP_REGISTER_PREFIX 							"/deviceapp/register"
#define DEVICEAPP_RESPONSE_PREFIX 							"/deviceapp/response"
#define COMMUNICATORAPP_REQUEST_PREFIX 						"/communicatorapp/request"
#define COMMUNICATORAPP_RESPONSE_PREFIX 					"/communicatorapp/response"
#define BROADSENS_TO_CLOUD_PREFIX							"/communicatorapp/broadsens"
#define CLOUD_CONNECTIONSTATUS_PREFIX 						"/cloud/connectionstatus"
#define DATACACHER_CACHED_DATA_UPLOAD_PREFIX 				"/datacatcher/cacheddata/upload"
#define DATACACHER_CACHED_DATA_PREFIX 						"/datacatcher/cacheddata"
#define DATACACHER_CACHED_DATA_REQUEST_PREFIX 				"/datacatcher/cacheddata/request"
#define RULE_ENGINE_RESPONSE_PREFIX 						"/rule_engine/response"
#define RULE_ENGINE_REQUEST_PREFIX 							"/rule_engine/request"
#define COMMUNICATOR_RULE_ENGINE_RESPONSE_PREFIX 			"/communicatorapp/rule_device/response"
#define REQUEST_PREFIX 										"/request"
#define DEVICEAPP_PREFIX 									"/deviceapp/"
#define BROADSENS_PREFIX 									"GW3335/#"
#define BROADSENS_GW_INFO 									"GW3335/GW_info"
#define BROADSENS_ACCE_PREFIX 								"GW3335/acce"
#define BROADSENS_GTEMP_PREFIX								"GW3335/gtmp"
#define BROADSENS_ATEMP_PREFIX								"GW3335/atmp"
#define BROADSENS_GVIB_PREFIX								"GW3335/gvib"
#define BROADSENS_FFT_PREFIX								"GW3335/fft"
#define BROADSENS_SNS_INFO_PREFIX							"GW3335/sensorList"
#define BROADSENS_MQTTfft                                    "GW3335/mqttFFT"
#define BROADSENS_DAQSTATUS                                    "GW3335/DAQStatus"
#define BROADSENS_CONTROL_FFT								"control/FFT"

#define TOTAL_NO_OF_AXIS_WITH_COORDINATES  6


#define COMMUNICOTOR_AGENT_LOG_FILE		"MQTTAgent_Log"
#define DEVICE_DATA_BACKUP 				"device_data_backup"
#define REQUEST							"request"
#define CONNECTION_STATUS				"connection_status"		

#define TYPE 							"type"
#define MESSAGE               			"message"
#define EVENT               			"event"
#define TIMESTAMP              			"timestamp"
#define DEVICE_ID                		"asset_id"
#define APP_NAME						"app_name"
#define APP_ID							"appid"

#define REGISTERED_DEVICES				"registered_devices"
#define DEREGISTER_DEVICES				"deregister_assets"
#define SET_POLLING_INFO				"set_polling_info"
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

#define RESET_SENSOR_BOARD                  "reset_sensor_board"
#define RESET_MESH_NETWORK                  "reset_mesh_network"
#define RESET_SENSOR                        "reset_sensor"
#define SET_BOARD_CONFIGURATION             "set_board_configuration"
#define SET_SLAVE_CONFIGURATION             "set_slave_configuration"
#define READ_SENSOR_PROPERTY               "read_sensor_property"
#define SET_FOTA_MODE                      "set_fota_mode"

#endif
