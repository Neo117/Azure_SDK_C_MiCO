/**
 ******************************************************************************
 * @file    azure_config.h
 * @author  Murphy zhao
 * @version V1.0.0
 * @date    28-Feb-2017
 * @brief   This file provide constant definition and type declaration for Azure
 *          running.
 ******************************************************************************
 */

#define MESSAGE_COUNT   5
#define DOWORK_LOOP_NUM ( MESSAGE_COUNT + 1 )

/************************************************************************
 * Please get your IoT hub connection string from Azure portal, the string should be like this:
 * HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key> */
const char* connectionString = "";

typedef struct EVENT_INSTANCE_TAG
{
    IOTHUB_MESSAGE_HANDLE messageHandle;
    size_t messageTrackingId;  // For tracking the messages within the user callback.用于跟踪回调函数信息
} EVENT_INSTANCE;               //
