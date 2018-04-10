#include <time.h>
#include "mico.h"
#include "sntp.h"
#include "mico_app_define.h"

#define TIME_SYNC_PERIOD    (3600 * SECONDS)
#ifdef JLINK_RTT_LOG
#define mico_time_log(M, ...)		SEGGER_RTT_printf(0,"[%ld][%s:%4d] " M "\r\n", mico_rtos_get_time(), SHORT_FILE, __LINE__, ##__VA_ARGS__)
#else
#define mico_time_log(M, ...) custom_log("SNTP", M, ##__VA_ARGS__)
#endif

mico_semaphore_t get_utc_sem = NULL;

static void read_utc_time_from_rtc( struct tm *utc_time )
{
    mico_rtc_time_t time;

    /*Read current time from RTC.*/
    if ( MicoRtcGetTime( &time ) == kNoErr )
    {
        utc_time->tm_sec = time.sec;
        utc_time->tm_min = time.min;
        utc_time->tm_hour = time.hr;
        utc_time->tm_mday = time.date;
        utc_time->tm_wday = time.weekday;
        utc_time->tm_mon = time.month - 1;
        utc_time->tm_year = time.year + 100;
    }
    else
    {
        mico_time_log("RTC function unsupported");
    }
}

/* Callback function when MiCO UTC time in sync to NTP server */
static void sntp_time_synced( void )
{
    struct tm *     currentTime;
    iso8601_time_t  iso8601_time;
    mico_utc_time_t utc_time;
    mico_rtc_time_t rtc_time;

    mico_time_get_iso8601_time( &iso8601_time );
    mico_time_log("%.27s", (char*)&iso8601_time);

    mico_time_get_utc_time( &utc_time );

    currentTime = localtime( (const time_t *)&utc_time );
    rtc_time.sec = currentTime->tm_sec;
    rtc_time.min = currentTime->tm_min;
    rtc_time.hr = currentTime->tm_hour;

    rtc_time.date = currentTime->tm_mday;
    rtc_time.weekday = currentTime->tm_wday;
    rtc_time.month = currentTime->tm_mon + 1;
    rtc_time.year = (currentTime->tm_year + 1900) % 100;

    MicoRtcSetTime( &rtc_time );
    mico_rtos_set_semaphore( &get_utc_sem );
}


void start_sntp(void)
{
    struct tm          utc_time;
    mico_utc_time_ms_t	utc_time_ms;
    mico_utc_time_t		utc_time_s;

    mico_rtos_init_semaphore( &get_utc_sem, 1 );
    /* Read UTC time from RTC hardware */
    read_utc_time_from_rtc( &utc_time );
    /* Set UTC time to MiCO system */
    utc_time_ms = (uint64_t) mktime( &utc_time ) * (uint64_t) 1000;
    mico_time_set_utc_time_ms( &utc_time_ms );
    mico_time_get_utc_time( &utc_time_s );
//	mico_time_log("utc ms time: %llu", (uint64_t)utc_time_ms ); // 在连上NTP服务器前就可以使用UTC时间
	mico_time_log("utc s time: %u", utc_time_s ); // 在连上NTP服务器前就可以使用UTC时间

    /* Start auto sync with NTP server */
    sntp_start_auto_time_sync( TIME_SYNC_PERIOD, sntp_time_synced );
    mico_rtos_get_semaphore( &get_utc_sem, 60000 );
}

char *sn_get_firmware_version( char version[50] )
{
    char wifi_model[13];
    char *pos;

    sprintf( wifi_model, "%s", MODEL );
    pos = wifi_model;
    pos += (strlen( pos ) - 2);
    sprintf( version, "AZURE_%s_%s_%s_%s_V%s", MANUFACTURER_NAME, wifi_model, PRODUCT_TYPE, PRODUCT_MODEL,
             APP_VERSION );

    return version;
}
