/**
 ******************************************************************************
 * @file    azure_main.c
 * @author  Murphy Zhao
 * @version V1.0.0
 * @date    28-Feb-2017
 * @brief   Azure connect Demo
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2014 MXCHIP Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 */
#include "mico.h"
#include "iothub_message.h"
#include "iothub_client.h"
#include "iothub_client_ll.h"
#include "iothubtransportmqtt.h"
#include "platform_mico.h"
#include "mico_time.h"
#include "certs.h"
#include "azure_config.h"
#include "mico_config.h"
#include "mico_app_define.h"

#define azure_main_log(M, ...) custom_log("azure_mqtt_demo", M, ##__VA_ARGS__)

static int callbackCounter;

volatile bool iothub_connected = false;

static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
    int* counter = (int*)userContextCallback;
    const char* buffer;
    size_t size;
    azure_main_log("fun: ReceiveMessageCallback -->\r\n");
    if (IoTHubMessage_GetByteArray(message, (const unsigned char**)&buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        azure_main_log("unable to retrieve the message data\r\n");
    }
    else
    {
        azure_main_log("Received Message [%d] with Data: <<<%.*s>>> & Size=%d\r\n", *counter, (int)size, buffer, (int)size);
    }

    /* Retrieve properties from the message */
    MAP_HANDLE mapProperties = IoTHubMessage_Properties(message);
    if (mapProperties != NULL)
    {
        const char*const* keys;
        const char*const* values;
        size_t propertyCount = 0;
        if (Map_GetInternals(mapProperties, &keys, &values, &propertyCount) == MAP_OK)
        {
            azure_main_log("key-value propertyCount:%d",propertyCount);
            if (propertyCount > 0)
            {
                for (size_t index = 0; index < propertyCount; index++)
                {
                    azure_main_log("\tKey: %s Value: %s\r\n", keys[index], values[index]);
                }
            }
        }
    }

    /* Some device specific action code goes here... */
    (*counter)++;
    return IOTHUBMESSAGE_ACCEPTED;
}

static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    EVENT_INSTANCE* eventInstance = (EVENT_INSTANCE*)userContextCallback;
    azure_main_log("Confirmation[%d] received for message tracking id = %u with result = %d\r\n", callbackCounter, eventInstance->messageTrackingId, result);
    /* Some device specific action code goes here... */
    callbackCounter++;
    IoTHubMessage_Destroy(eventInstance->messageHandle);
}

static void StartAzureIoTTest()
{
    IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

    char msgText[128];
    char propText[32];
    EVENT_INSTANCE messages[MESSAGE_COUNT];

    srand((unsigned int)time(NULL));
    double avgWindSpeed = 10.0;

    callbackCounter = 0;
    int receiveContext = 0;

    /* Try to connect the Azure IoT hub */
    for (int i = 0; i < 10; i++)
    {
        if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol)) != NULL)
        {
            break;
        }
    }
    if(iotHubClientHandle == NULL)
    {
        azure_main_log("ERROR: iotHubClientHandle is NULL!\r\n");
        return;
    }

    bool traceOn = true;
    IoTHubClient_LL_SetOption(iotHubClientHandle, "logtrace", &traceOn);

    if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
    {
        azure_main_log("failure to set option \"TrustedCerts\"\r\n");
    }


    /* Setting Message call back, so we can receive Commands. */
    if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
    {
        azure_main_log("ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
    }
    else
    {
        azure_main_log("IoTHubClient_LL_SetMessageCallback...successful.\r\n");

       /* Now that we are ready to receive commands, let's send some messages */
       for (int iterator = 0; iterator < MESSAGE_COUNT; iterator++)
       {
           sprintf_s(msgText, sizeof(msgText), "{\"deviceId\":\"mico dev kit\",\"windSpeed\":%.2f}", avgWindSpeed + (rand() % 4 + 2));
           if ((messages[iterator].messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)msgText, strlen(msgText))) == NULL)
           {
               azure_main_log("ERROR: iotHubMessageHandle is NULL!\r\n");
           }
           else
           {
               messages[iterator].messageTrackingId = iterator;
               MAP_HANDLE propMap = IoTHubMessage_Properties(messages[iterator].messageHandle);
               (void)sprintf_s(propText, sizeof(propText), "PropMsg_%u", iterator);
               if (Map_AddOrUpdate(propMap, "PropName", propText) != MAP_OK)
               {
                   azure_main_log("ERROR: Map_AddOrUpdate Failed!\r\n");
               }

               if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messages[iterator].messageHandle, SendConfirmationCallback, &messages[iterator]) != IOTHUB_CLIENT_OK)
               {
                   azure_main_log("ERROR: IoTHubClient_LL_SendEventAsync..........FAILED!\r\n");
               }
               else
               {
                   azure_main_log("IoTHubClient_LL_SendEventAsync accepted message [%d] for transmission to IoT Hub.\r\n", (int)iterator);
               }
           }

           IoTHubClient_LL_DoWork(iotHubClientHandle);
           /* Print out the memory usage */
           azure_main_log("****Memory info: total %d, free %d.", MicoGetMemoryInfo()->total_memory, MicoGetMemoryInfo()->free_memory);
           mico_thread_sleep(1);
       }

       azure_main_log("iothub_client_sample_mqtt has gotten quit message, call DoWork %d more time to complete final sending...\r\n", DOWORK_LOOP_NUM);
       for (size_t index = 0; index < DOWORK_LOOP_NUM; index++)
       {
           IoTHubClient_LL_DoWork(iotHubClientHandle);
           mico_thread_sleep(1);
       }

       /* Print out the memory usage */
       azure_main_log("****Memory info: total %d, free %d.", MicoGetMemoryInfo()->total_memory, MicoGetMemoryInfo()->free_memory);
    }

    IoTHubClient_LL_Destroy(iotHubClientHandle);
    azure_main_log("Azure IoT MQTT testing has done.");
}


void mqtt_client_thread( mico_thread_arg_t arg )
{
    UNUSED_PARAMETER( arg );

    azure_main_log("Azure IoT MQTT testing start.");

    StartAzureIoTTest();

    mico_rtos_delete_thread( NULL );
}


int azure_main(void)
{
    OSStatus err = kNoErr;

    /* firmware_version < 50 byte */
    char firmware_version[50] = {0};
    sn_get_firmware_version( firmware_version );
    azure_main_log(" azure_iothub_mqtt_demo firmware_version: %s ",firmware_version);

    /* Read RTC time,set UTC time */
    start_sntp();

    /* Get Memory information */
    azure_main_log("****Memory info: total %d, free %d.", MicoGetMemoryInfo()->total_memory, MicoGetMemoryInfo()->free_memory);

//    /* Start auto sync with NTP server */
//    start_sntp();

    /* Start Azure IoT MQTT testing */
    azure_main_log( "Start Azure IoT MQTT testing..." );
    err = mico_rtos_create_thread( NULL, MICO_APPLICATION_PRIORITY, "mqtt_client", mqtt_client_thread, 0x4000, 0 );
    require_noerr_string( err, exit, "ERROR: Unable to start the mqtt client thread." );

exit:
    return err;
}
