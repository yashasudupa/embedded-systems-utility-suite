#ifndef ProvisionDevice_h
#define ProvisionDevice_h 1

#include "azure_prov_client/prov_device_ll_client.h"
#include "azure_prov_client/prov_security_factory.h"

#include "iothubtransportmqtt.h"

#include "azure_prov_client/prov_transport_mqtt_client.h"

MU_DEFINE_ENUM_STRINGS_WITHOUT_INVALID(PROV_DEVICE_RESULT, PROV_DEVICE_RESULT_VALUE);
MU_DEFINE_ENUM_STRINGS_WITHOUT_INVALID(PROV_DEVICE_REG_STATUS, PROV_DEVICE_REG_STATUS_VALUES);

#define MESSAGES_TO_SEND                2
#define TIME_BETWEEN_MESSAGES_SECONDS   2

typedef struct CLIENT_SAMPLE_INFO_TAG
{
    PROV_DEVICE_LL_HANDLE handle;
    unsigned int sleep_time_msec;
    char* iothub_uri;
    char* device_id;
    bool registration_complete;
} CLIENT_SAMPLE_INFO;

typedef struct IOTHUB_CLIENT_SAMPLE_INFO_TAG
{
    bool connected;
    bool stop_running;
} IOTHUB_CLIENT_SAMPLE_INFO;

#endif
