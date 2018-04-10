/**
 ******************************************************************************
 * @file    mico_main.c
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
#include "mico_app_define.h"

#define mico_main_log(M, ...) custom_log("azure_mqtt_demo", M, ##__VA_ARGS__)


static mico_semaphore_t wait_sem = NULL;

static void micoNotify_ConnectFailedHandler(OSStatus err, void* inContext)
{
    mico_main_log("Failed to join wifi router!");
}

static void micoNotify_WifiStatusHandler( WiFiEvent status, void* const inContext )
{
    UNUSED_PARAMETER( inContext );
    switch ( status )
    {
        case NOTIFY_STATION_UP:
            mico_main_log("wifi router was connected!");
            if( wait_sem != NULL )
                mico_rtos_set_semaphore( &wait_sem );
            break;
        case NOTIFY_STATION_DOWN:
            mico_main_log("wifi router was disconnected!");
            break;
        default:
            break;
    }
}


int application_start( void )
{
    OSStatus err = kNoErr;

    mico_rtos_init_semaphore( &wait_sem, 1 );

    /* Start MiCO system functions according to mico_config.h */
    err = mico_system_init( mico_system_context_init( 0 ) );
    require_noerr( err, exit );

    /* Register user function when wlan connection status is changed */
    err = mico_system_notify_register( mico_notify_WIFI_STATUS_CHANGED, (void *)micoNotify_WifiStatusHandler, NULL );
    require_noerr( err, exit );

    /* Register user function when wlan connection is faild in one attempt */
    err = mico_system_notify_register( mico_notify_WIFI_CONNECT_FAILED, (void *)micoNotify_ConnectFailedHandler, NULL );
    require_noerr( err, exit );

    /* Wait for wlan connection */
    mico_rtos_get_semaphore( &wait_sem, MICO_WAIT_FOREVER );
    mico_main_log( "wifi connected successful" );

    azure_main();

exit:
    if (wait_sem != NULL)
    {
        mico_rtos_deinit_semaphore(&wait_sem);
        wait_sem = NULL;
    }
    mico_rtos_delete_thread(NULL);
    return err;
}

