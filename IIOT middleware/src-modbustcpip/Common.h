#ifndef Common_h
#define Common_h 1

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
#define PACKAGE_CONFIG_JSON			    "package_config.json"
#define DEVICEINFO_PATH				    "./config/device_info.json"
#define POLLINGCONFIG_PATH			    "./config/pollingconfig.json"
#define COMMAND						    "command"

#define CHANGE_DEVICE_MODE			    "change_asset_mode"
#define SET_CONFIGURATION			    "set_asset_configuration"
#define ALL_PROPERTIES			        "all_props_at_fixed_interval"
#define CHANGED_PROPERTIES			    "changed_props_at_fixed_interval"
#define DEVICE_CONFIGURATION_JSON	    "device_configuration.json"


#define SET_POLLING_CONFIG			    "set_polling_info"
#define REGISTER_DEVICES			    "register_assets"
#define DEREGISTER_DEVICES			    "deregister_assets"
#define SET_PROPERTIES				    "set_properties"
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
#define MULTI_TELEMETRY				    "multi_telemetry"
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
#define GROUP_NAME					        "g"
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
    DATA_DECIMAL_FLOAT= 10,
	DATA_FLOAT_REVERSE_ABCD = 11,
	DATA_FLOAT_REVERSE_BACD = 12,
	DATA_FLOAT_REVERSE_CDAB = 13
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

#endif