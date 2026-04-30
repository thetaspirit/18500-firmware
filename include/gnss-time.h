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
#include <SparkFun_u-blox_GNSS_v3.h> //http://librarymanager/All#SparkFun_u-blox_GNSS_v3
#include "system-utils.h"
#include "debug.h"

#ifndef ESP32TIME_H
#define ESP32TIME_H
#endif

// the TX pin on the ESP32 that should connect to the GPS
#define GNSS_TX 4
// the RX pin on the ESP32 that should connect to the GPS
#define GNSS_RX 5
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

  enum UpdateStatus
  {
    UPDATED_BOTH,
    UPDATED_TIME_ONLY,
    UPDATED_DATE_ONLY,
    UPDATED_NONE
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

  bool init();

  // bool power_off(); // put GNSS to sleep to conserve power
  // bool wake_up();   // wakes up GNSS.  must be called before calling other functions if asleep

  /**
   * @brief Uses position and date to estimate the UTC timezone offset.
   * Also takes into account rudamentary daylight savings time rules.
   * This function is blocking.
   * @return an hour offset from UTC.  returns UTC_OFFSET_UNAVAILABLE if it cannot get a GNSS fix.
   */
  int estimate_utc_offset();

  /**
   * @brief Attempts to retrieve the current date and time from GNSS and store that into the RTC.
   * This function is blocking.
   * @return Status indicating what it was or was not able to acquire.
   */
  UpdateStatus update_gnss_datetime();

  /**
   * @brief Attempts to retrieve the current date and time from GNSS and applies the given UTC offset to it.
   * Note that this function does not modify the RTC.
   *
   * @param utc_offset The UTC timezone offset in hours
   * @param datetime Pointer to DateTime struct to store the result
   * @return Status indicating what it was or was not able to acquire.
   */
  UpdateStatus get_gnss_datetime(int utc_offset, DateTime *datetime);

  /**
   * @brief Retrieves whatever time the internal RTC has, and applies the provided UTC offset.
   */
  void get_rtc_datetime(int utc_offset, DateTime *datetime);

  /**
   * Uses the RTC, does not make calls to GNSS.
   */
  uint16_t get_minutes_after_midnight();

}