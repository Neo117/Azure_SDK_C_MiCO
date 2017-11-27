#include "mico.h"
#include "mico_app_define.h"
#include "mqtt_client_interface.h"

#define mqtt_log(M, ...) custom_log("mqtt", M, ##__VA_ARGS__)

char *mqtt_client_id_get( char clientid[30] )
{
    uint8_t mac[6];
    char mac_str[13];

    mico_wlan_get_mac_address( mac );
    sprintf( mac_str, "%02X%02X%02X%02X%02X%02X",
             mac[0],
             mac[1], mac[2], mac[3], mac[4], mac[5] );
    sprintf( clientid, "MiCO_%s", mac_str );

    return clientid;
}


void iot_subscribe_callback_handler( MQTT_Client *pClient, char *topicName,
                                     uint16_t topicNameLen,
                                     IoT_Publish_Message_Params *params,
                                     void *pData )
{
    IOT_UNUSED( pData );
    IOT_UNUSED( pClient );
    mqtt_log("Subscribe callback");
    mqtt_log("%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *)(params->payload));
}

static void mqtt_sub_pub_main( mico_thread_arg_t arg )
{
    IoT_Error_t rc = MQTT_FAILURE;

    char clientid[40];
    char cPayload[100];
    int i = 0;
    MQTT_Client client;
    IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
    IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;
    IoT_Publish_Message_Params paramsQOS0;
    IoT_Publish_Message_Params paramsQOS1;

    /*
     * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
     *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
     *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
     */
    mqttInitParams.enableAutoReconnect = false;
    mqttInitParams.pHostURL = MQTT_HOST;
    mqttInitParams.port = MQTT_PORT;
    mqttInitParams.mqttPacketTimeout_ms = 20000;
    mqttInitParams.mqttCommandTimeout_ms = 20000;
    mqttInitParams.tlsHandshakeTimeout_ms = 5000;
    mqttInitParams.disconnectHandler = NULL;
    mqttInitParams.disconnectHandlerData = NULL;
#ifdef MQTT_USE_SSL
    mqttInitParams.pRootCALocation = MQTT_ROOT_CA_FILENAME;
    mqttInitParams.pDeviceCertLocation = MQTT_CERTIFICATE_FILENAME;
    mqttInitParams.pDevicePrivateKeyLocation = MQTT_PRIVATE_KEY_FILENAME;
    mqttInitParams.isSSLHostnameVerify = false;
    mqttInitParams.isClientnameVerify = false;
    mqttInitParams.isUseSSL = true;
#else
    mqttInitParams.pRootCALocation = NULL;
    mqttInitParams.pDeviceCertLocation = NULL;
    mqttInitParams.pDevicePrivateKeyLocation = NULL;
    mqttInitParams.isSSLHostnameVerify = false;
    mqttInitParams.isClientnameVerify = false;
    mqttInitParams.isUseSSL = false;
#endif

    rc = mqtt_init( &client, &mqttInitParams );
    if ( MQTT_SUCCESS != rc )
    {
        mqtt_log("aws_iot_mqtt_init returned error : %d ", rc);
        goto exit;
    }

    connectParams.keepAliveIntervalInSec = 30;
    connectParams.isCleanSession = true;
    connectParams.MQTTVersion = MQTT_3_1_1;
    connectParams.pClientID = MQTT_CLIENT_ID;
    connectParams.clientIDLen = (uint16_t) strlen( MQTT_CLIENT_ID );
    connectParams.isWillMsgPresent = false;
    connectParams.pUsername = MQTT_USERNAME;
    connectParams.usernameLen = strlen(MQTT_USERNAME);
    connectParams.pPassword = MQTT_PASSWORD;
    connectParams.passwordLen = strlen(MQTT_PASSWORD);

RECONN:
    mqtt_log("Connecting...");
    rc = mqtt_connect( &client, &connectParams );
    if ( MQTT_SUCCESS != rc )
    {
        mqtt_log("Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
        sleep(2);
        goto RECONN;
    }

    mqtt_log("Subscribing...");
    rc = mqtt_subscribe( &client, MQTT_SUB_NAME, strlen( MQTT_SUB_NAME ), QOS0,
                         iot_subscribe_callback_handler, NULL );
    if ( MQTT_SUCCESS != rc )
    {
        mqtt_log("Error subscribing : %d ", rc);
        goto RECONN;
    }

    mqtt_log("publish...");
    sprintf( cPayload, "%s : %d ", "hello from SDK", i );

    paramsQOS0.qos = QOS0;
    paramsQOS0.payload = (void *) cPayload;
    paramsQOS0.isRetained = 0;

    paramsQOS1.qos = QOS1;
    paramsQOS1.payload = (void *) cPayload;
    paramsQOS1.isRetained = 0;

    while ( 1 )
    {
        //Max time the yield function will wait for read messages
        rc = mqtt_yield( &client, 100 );
        if ( MQTT_SUCCESS != rc )
        {
            // If the client is attempting to reconnect we will skip the rest of the loop.
            mico_rtos_thread_sleep( 1 );
            mqtt_log("mqtt disconnect");
            mqtt_disconnect( &client );
            goto RECONN;
        }

        mqtt_log("-->sleep, rc:%d", rc);
        sprintf( cPayload, "%s : %d ", "hello from SDK QOS0", i++ );
        paramsQOS0.payloadLen = strlen( cPayload );
        mqtt_publish( &client, MQTT_SUB_NAME, strlen( MQTT_SUB_NAME ), &paramsQOS0 );

        sprintf( cPayload, "%s : %d ", "hello from SDK QOS1", i++ );
        paramsQOS1.payloadLen = strlen( cPayload );
        rc = mqtt_publish( &client, MQTT_SUB_NAME, strlen( MQTT_SUB_NAME ), &paramsQOS1 );
        if ( rc == MQTT_REQUEST_TIMEOUT_ERROR )
        {
            mqtt_log("QOS1 publish ack not received");
            rc = MQTT_SUCCESS;
        }
    }

    exit:
    mico_rtos_delete_thread( NULL );
}

OSStatus start_mqtt_sub_pub( void )
{
#ifdef MQTT_USE_SSL
    uint32_t stack_size = 0x3000;
#else
    uint32_t stack_size = 0x2000;
#endif
    return mico_rtos_create_thread( NULL, MICO_APPLICATION_PRIORITY, "mqtt", mqtt_sub_pub_main,
                                    stack_size,
                                    0 );
}
