/**
 * @file gnss-time.h
 * @brief GNSS Time Management Library for ESP32
 *
 * @author Delaynie McMillan
 * Most of the functions themselves were written by Copilot, with me controling the overall
 * architecture and writing the header file and describing what each
 * function should do and be used for.
 *
 * This library manages synchronization of the ESP32's internal RTC with a u-blox GNSS module.
 * It provides functions to:
 * - Initialize and communicate with a u-blox MAX-M10S GNSS receiver
 * - Retrieve accurate UTC time from GNSS satellites
 * - Estimate local timezone offset from GPS position and DST rules
 * - Maintain the system clock using gettimeofday()/settimeofday() functions
 * - Periodically update the RTC from GNSS (default: once per week)
 *
 * The library uses a custom DateTime struct for convenient date/time representation
 * and handles conversions between Unix timestamps (timeval) and broken-down time.
 *
 * @note Requires SparkFun u-blox GNSS library (v3+)
 * @note Uses RTC_DATA_ATTR for persistence across deep sleep
 */

#pragma once

#ifndef ESP32TIME_H
#define ESP32TIME_H
#endif

#include <SparkFun_u-blox_GNSS_v3.h> //http://librarymanager/All#SparkFun_u-blox_GNSS_v3

#define GPS_SERIAL Serial2
#define GNSS_QUERY_TIMEOUT_MS 500
#define UTC_OFFSET_UNAVAILABLE -99

/** Update the microcontroller's internal time once every week (= 7 days = 168 hours) */
#define GNSS_UPDATE_RATE_HOURS 168

namespace gnss_time
{

    struct DateTime
    {
        uint16_t year;       // the year
        uint8_t month;       // the month [1, 12]
        uint8_t day;         // the day of the month [1, 31]
        uint8_t day_of_week; // day of the week [1, 7]

        uint8_t hour;   // hour of the day [0, 23]
        uint8_t minute; // minute of the hour [0, 59]
        uint8_t second; // second of the minute [0, 59]
    };

    /**
     * @brief Initializes communication with the U-BLOX MAX-M10S.  Call this function multiple times if init does not work on the first try.
     *
     * @param serial_port Serial port
     * @param tx_pin TX pin on the ESP32 that connects to the GPS
     * @param rx_pin RX pin on the ESP32 that connects to the GPS
     * @param max_wait_time_ms The maximum amount of milliseconds polling functions will block for when communicating with GNSS module.
     *
     * @return true if initalization was successful, false if not
     */
    bool init(int tx_pin, int rx_pin, uint16_t max_wait_time_ms);

    // bool power_off(); // put GNSS to sleep to conserve power
    // bool wake_up();   // wakes up GNSS.  must be called before calling other functions if asleep

    /**
     * @brief Uses position and date to estimate the UTC timezone offset.
     * Also takes into account rudamentary daylight savings time rules.
     * @return an hour offset from UTC.  returns UTC_OFFSET_UNAVAILABLE if it cannot get a GNSS fix.
     */
    int estimate_utc_offset();

    /**
     * @brief This library will remember the most recent (valid) UTC offset it estimated based on the user's position.
     * Note that this data will only ever get updated if 2 things happen:
     * 1) The user actually calls the estimate_utc_offset() function and
     * 2) That function was able to get a resonably accurate GNSS fix to make such an estimation.
     * @return the most recent and accurate UTC offset estimation.
     */
    int get_saved_utc_offset();

    /**
     * @brief Retrieves the current date and time and applies the given UTC offset.
     * The microcontroller uses its internal RTC to keep track of time, and if more than
     * GNSS_UPDATE_RATE_HOURS hours have passed since updating the time from GNSS,
     * this function will also attempt to retrieve an updated time from GNSS.
     *
     * @param utc_offset The UTC timezone offset in hours
     * @param datetime Pointer to DateTime struct to store the result
     * @return true if successful, false if unable to retrieve datetime
     */
    bool get_datetime(int utc_offset, DateTime *datetime);

    /**
     * @brief Retrieves the current date and time and applies the given UTC offset.
     * This function will also attempt to retrieve an updated time from GNSS, regardless
     * of when the last GNSS update was done.
     *
     * @param utc_offset The UTC timezone offset in hours
     * @param datetime Pointer to DateTime struct to store the result
     * @return true if successful, false if unable to retrieve datetime
     */
    bool get_gnss_datetime(int utc_offset, DateTime *datetime);

    int get_SIV(); // returns the number of satellites in view
    bool get_gnss_fix_ok();
    bool get_time_fully_resolved(); // returns whether or not time is able to be fully resolved
    bool get_date_valid();
    bool get_time_valid();
    bool get_confirmed_date();
    bool get_confirmed_time();
}