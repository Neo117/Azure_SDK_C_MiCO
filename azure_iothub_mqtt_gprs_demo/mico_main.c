/**
 ******************************************************************************
 * @file    hello_world.c
 * @author  William Xu
 * @version V1.0.0
 * @date    21-May-2015
 * @brief   First MiCO application to say hello world!
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2016 MXCHIP Inc.
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
 *
 ******************************************************************************
 */

#include "mico.h"

#define gprs_log(format, ...)  custom_log("GPRS", format, ##__VA_ARGS__)

static mico_semaphore_t conn_sem;

void clientNotify_GPRSStatusHandler(notify_gprs_t status, const mico_gprs_net_addr_t *net_addr )
{
  switch (status)
  {
  case NOTIFY_GPRS_UP:
    gprs_log("GPRS up");
    gprs_log(" IP      : %s\r\n", net_addr->ip);
    gprs_log(" Gateway : %s\r\n", net_addr->gate);
    gprs_log(" Netmask : %s\r\n", net_addr->mask);
    mico_rtos_set_semaphore(&conn_sem);
    break;
  case NOTIFY_GPRS_DOWN:
    break;
  default:
    break;
  }
}

int application_start( void )
{
    mico_rtos_init_semaphore(&conn_sem, 1);
    mico_system_notify_register( mico_notify_GPRS_STATUS_CHANGED, clientNotify_GPRSStatusHandler, NULL );
    cli_init();
    MicoInit();

    gprs_log("Opening GPRS ...");
    mico_gprs_open();
    mico_rtos_get_semaphore(&conn_sem, MICO_WAIT_FOREVER);
    gprs_log("Open GPRS success");

    azure_main();

    return 0;
}
