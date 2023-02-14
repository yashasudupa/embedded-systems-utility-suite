#ifndef Common_h
#define Common_h 1
#include "ExceptionLogger.h"
#include <sstream>

#define LOCAL_MQTT_CONFIGURATION_FILE			"/opt/IoT_Gateway/Common/LocalBrokerConfig.json"
#define DOWNLOADPATH							"/opt/IoT_Gateway/packages/"
#define INSTALLPATH								"/opt/IoT_Gateway/"
#define LIBPATH									"/usr/lib/"
#define LIBDIR									"libs"
#define APP_REGISTER							"app_register"

#define SET_FOTA_MODE                       "set_fota_mode"

//Json file Names
#define REGISTER_DEVICES_JSON			"register_devices.json"
#define POLLING_CONFIG_JSON				"polling_config.json"
#define PACKAGE_CONFIG_JSON				"package_config.json"
#define SET_VALUE_CANAGE_JSON			"value_change_info.json"
#define DEVICE_CONFIGURATION_JSON		"device_configuration.json"
#define ALL_PROPERTIES			        "all_props_at_fixed_interval"
#define CHANGED_PROPERTIES			    "changed_props_at_fixed_interval"

#define COMMAND_INFO					"commandinfo"
#define COMMAND_TYPE					"commandtype"
#define TWINVERSIONFILE					"version.txt"
#define DLLExtention					".so"

//paths
#define CONFIGURATIONFILE				"./config/configuration.json"
#define DEVICE_APP_LOG_FILE				"DeviceApplication_Log"

//modes
#define CHANGE_DEVICE_MODE				"change_asset_mode"
#define SET_CONFIGURATION				"set_asset_configuration"
#define GET_DEVICE_CONFIGURATION		"get_device_configuration"
#define READ_SENSOR_POSITION_CONFIGURATION          "read_sensor_positioning_configuration"

#define SET_POLLING_CONFIG				"set_polling_info"
#define REGISTER_DEVICES				"register_assets"
#define DEREGISTER_DEVICES				"deregister_assets"
#define SET_PROPERTIES					"set_properties"
#define SET_VALUE_CANAGE				"set_change_value_state"

#define COMMAND							"command"
#define TYPE 							"type"
#define MESSAGE               			"message"
#define EVENT               			"event"
#define TIMESTAMP              			"timestamp"
#define DEVICE_ID               		"asset_id"
#define APP_NAME						"app_name"
#define APP_ID							"appid"
#define QOS								2
#define SUB_JOB_ID						"sub_job_id"
#define STATUS							"status"
#define PROCESS_NAME				    "process_name"


#define KEMSYS_PREFIX							"kemsys/gateway/"
#define REQUEST_PREFIX							"/request"
#define DEVICE_APP_PREFIX						"/deviceapp/" 
#define DEVICE_APP_RESPONSE_PREFIX				"/deviceapp/response"
#define DEVICE_APP_REGISTER_PREFIX				"/deviceapp/register"

#define MEASURED_PROPERTIES						"measured_properties"
#define WRITABLE_PROPERTIES						"writable_properties"
#define READABLE_PROPERTIES						"readable_properties"
#define DERIVED_PROPERTIES						"edge_derived_properties"
#define ALERTS_PROPERTIES						"alerts"

#define TURBO_MODE_TIMEOUT			            "turbo_mode_timeout_in_milli_sec"
#define TURBO_MODE_FREQUENCY		            "turbo_mode_frequency_in_milli_sec"
#define MEASUREMENT_FREQUENCY		            "measurement_frequency_in_milli_sec"
#define TELEMETRY_MODE				            "telemetry_mode"
#define INGESTION_SETTINGS_TYPE			        "ingestion_settings_type"
#define INGESTION_SETTINGS_FREQUENCY	        "ingestion_settings_frequency_in_milli_sec"

#define REGISTER_SLAVES                     "register_slaves"
#define DEREGISTER_SLAVES                   "deregister_slaves"
#define RESET_SENSOR_BOARD                  "reset_sensor_board"
#define RESET_MESH_NETWORK                  "reset_mesh_network"
#define RESET_BLUENRG                       "reset_bluenrg"
#define RESET_SENSOR                        "reset_sensor"
#define SET_BOARD_CONFIGURATION             "set_board_configuration"
#define SET_SLAVE_CONFIGURATION             "set_slave_configuration"
#define SET_POSITION_CONFIGURATION          "set_sensor_pos_config"
#define SET_MOP_CONFIGURATION               "set_sensor_mop"
#define READ_SENSOR_PROPERTY               "read_sensor_property"

#endif