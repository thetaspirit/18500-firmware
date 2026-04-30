#include "gnss-time.h"

#include "time.h"
#include <sys/time.h>

#ifdef RTC_DATA_ATTR
RTC_DATA_ATTR static time_t _last_gnss_update_time = 0;
#else
static time_t _last_gnss_update_time = 0;
#endif

namespace gnss_time
{

  /** internal ublox serial gnss object. completely abstracted away from client code. */
  SFE_UBLOX_GNSS_SERIAL _gnss;

  /** internal variable to keep track of sleep state */
  bool _is_asleep;

  /** maximum wait time parameter for when querying GNSS module */
  uint16_t _max_wait_ms;

  /**
   * @brief Helper function to calculate day of week from date (Zeller's congruence)
   * @return 0 = Sunday, 1 = Monday, ..., 6 = Saturday
   */
  static uint8_t _calculateDayOfWeek(uint8_t year, uint8_t month, uint8_t day)
  {
    // Convert to full year (2000 + year for 21st century)
    uint16_t fullYear = 2000 + year;

    // Adjust month and year for Zeller's formula (Jan and Feb are months 13 and 14 of previous year)
    if (month <= 2)
    {
      month += 12;
      fullYear -= 1;
    }

    // Zeller's congruence
    uint16_t q = day;
    uint16_t m = month;
    uint16_t k = fullYear % 100;
    uint16_t j = fullYear / 100;

    uint16_t h = (q + (13 * (m + 1)) / 5 + k + (k / 4) + (j / 4) - 2 * j) % 7;
    // Convert to 0 = Sunday, 1 = Monday, etc. Zeller gives 0 = Saturday
    return (h + 6) % 7;
  }

  /**
   * @brief Convert a DateTime struct (UTC) to a timeval struct
   * @param dt The DateTime struct in UTC
   * @param tv Pointer to timeval struct to store the result
   */
  static void _dateTimeToTimeval(const DateTime &dt, struct timeval &tv)
  {
    // Convert broken-down time to Unix timestamp
    // This algorithm works for years 2000-2099
    uint16_t year = dt.year;
    uint8_t month = dt.month;
    uint8_t day = dt.day;

    // Count days since epoch (1970-01-01)
    uint32_t days = 0;

    // Count leap years from 1970 to year-1
    for (uint16_t y = 1970; y < year; y++)
    {
      if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))
        days += 366;
      else
        days += 365;
    }

    // Count days for months in the current year
    const uint8_t daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);

    for (uint8_t m = 1; m < month; m++)
    {
      days += daysInMonth[m];
      if (m == 2 && isLeapYear)
        days++;
    }

    // Add days in current month
    days += day - 1;

    // Convert to seconds and add time components
    time_t seconds = (time_t)days * 86400 + dt.hour * 3600 + dt.minute * 60 + dt.second;

    tv.tv_sec = seconds;
    tv.tv_usec = 0;
  }

  /**
   * @brief Convert a timeval struct to a DateTime struct, applying UTC offset
   * @param tv The timeval struct (contains seconds since epoch)
   * @param utc_offset The UTC offset in hours to apply
   * @param dt Pointer to DateTime struct to store the result
   */
  static void _timevalToDateTime(const struct timeval &tv, int utc_offset, DateTime &dt)
  {
    // Add UTC offset to get local time
    time_t seconds = tv.tv_sec + ((time_t)utc_offset * 3600);

    // Constants
    const uint32_t SECONDS_PER_DAY = 86400;
    uint32_t daysSinceEpoch = seconds / SECONDS_PER_DAY;
    uint32_t secondsInDay = seconds % SECONDS_PER_DAY;

    // Extract time of day
    dt.hour = (secondsInDay / 3600) % 24;
    dt.minute = (secondsInDay / 60) % 60;
    dt.second = secondsInDay % 60;

    // Calculate date from days since epoch
    uint16_t year = 1970;
    uint32_t remainingDays = daysSinceEpoch;

    // Count years from epoch
    while (true)
    {
      uint16_t daysThisYear;
      if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        daysThisYear = 366;
      else
        daysThisYear = 365;

      if (remainingDays < daysThisYear)
        break;

      remainingDays -= daysThisYear;
      year++;
    }

    dt.year = year;

    // Count months
    const uint8_t daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);

    uint8_t month = 1;
    while (remainingDays > 0 && month <= 12)
    {
      uint8_t daysThisMonth = daysInMonth[month];
      if (month == 2 && isLeapYear)
        daysThisMonth = 29;

      if (remainingDays < daysThisMonth)
        break;

      remainingDays -= daysThisMonth;
      month++;
    }

    dt.month = (month > 12) ? 12 : month;
    dt.day = remainingDays + 1;
    dt.day_of_week = _calculateDayOfWeek(year % 100, month, dt.day);
  }
  int get_SIV()
  {
    _gnss.getPVT(_max_wait_ms);
    return _gnss.getSIV(_max_wait_ms);
  }

  bool get_gnss_fix_ok()
  {
    return _gnss.getGnssFixOk();
  }
  bool get_time_fully_resolved()
  {
    return _gnss.getTimeFullyResolved(_max_wait_ms);
  }

  bool get_date_valid()
  {
    return _gnss.getDateValid(_max_wait_ms);
  }
  bool get_time_valid()
  {
    return _gnss.getTimeValid(_max_wait_ms);
  }
  bool get_confirmed_date()
  {
    return _gnss.getConfirmedDate(_max_wait_ms);
  }
  bool get_confirmed_time()
  {
    return _gnss.getConfirmedTime(_max_wait_ms);
  }

  // ######################################### INTERFACE FUNCTIONS BELOW #########################################
  bool init(int tx_pin, int rx_pin, uint16_t max_wait_time_ms)
  {
    _is_asleep = false;
    _max_wait_ms = max_wait_time_ms;

    do
    {
      DEBUG_PRINTLN("GNSS: trying 38400 baud");
      GPS_SERIAL.begin(38400, SERIAL_8N1, rx_pin, tx_pin, false, 20000UL, 112);
      if (_gnss.begin(GPS_SERIAL) == true)
        break;

      vTaskDelay(100);
      DEBUG_PRINTLN("GNSS: trying 9600 baud");
      GPS_SERIAL.begin(9600, SERIAL_8N1, rx_pin, tx_pin, false, 20000UL, 112);
      if (_gnss.begin(GPS_SERIAL) == true)
      {
        DEBUG_PRINTLN("GNSS: connected at 9600 baud, switching to 38400");
        _gnss.setSerialRate(38400);
        vTaskDelay(100);
      }
      else
      {
        // _gnss.factoryDefault();
        vTaskDelay(2000); // Wait a bit before trying again to limit the Serial output
      }
    } while (1);
    DEBUG_PRINTLN("GNSS serial connected");

    _gnss.setUART1Output(COM_TYPE_UBX); // Set the UART port to output UBX only
    // _gnss.setNavigationFrequency(4);    // set navigation frequency to 4 Hz
    _gnss.saveConfiguration(); // Save the current settings to flash and BBR

    return true;
  }

  bool init()
  {
    return init(GNSS_TX, GNSS_RX, 1100);
  }

  int estimate_utc_offset()
  {
    uint32_t horiz_acc = _gnss.getHorizontalAccuracy();
    DEBUG_PRINTF("Horizontal acc = %d\n", horiz_acc);

    // give up if location solution is bad
    if (!get_gnss_fix_ok())
    {
      return UTC_OFFSET_UNAVAILABLE;
    }

    // Get longitude from GNSS module
    int32_t longitude = _gnss.getLongitude();

    // Longitude is in degrees * 10^7, so divide by 10^7 to get decimal degrees
    double lonDegrees = longitude / 10000000.0;

    // Calculate base timezone offset from longitude
    // Each 15 degrees of longitude = 1 hour of time difference
    int8_t baseTimezoneOffset = (int8_t)round(lonDegrees / 15.0);

    // Get date for DST calculation
    uint8_t year = _gnss.getYear() % 100;
    uint8_t month = _gnss.getMonth();
    uint8_t day = _gnss.getDay();

    // Calculate day of week to check for DST
    uint8_t dayOfWeek = _calculateDayOfWeek(year, month, day);

    // Check if DST is active (US DST rules)
    int8_t utcOffset = baseTimezoneOffset;

    // DST: Second Sunday in March to First Sunday in November
    if (month > 3 && month < 11)
    {
      utcOffset += 1; // DST active (April through October)
    }
    else if (month == 3)
    {
      // March: DST starts on second Sunday
      uint8_t firstSundayDow = _calculateDayOfWeek(year, 3, 1);
      int firstSunday = 1 + (7 - firstSundayDow) % 7;
      int secondSunday = firstSunday + 7;
      if (day >= secondSunday)
        utcOffset += 1;
    }
    else if (month == 11)
    {
      // November: DST ends on first Sunday
      uint8_t firstSundayDow = _calculateDayOfWeek(year, 11, 1);
      int firstSunday = 1 + (7 - firstSundayDow) % 7;
      if (day < firstSunday)
        utcOffset += 1;
    }

    return utcOffset;
  }

  UpdateStatus update_gnss_datetime()
  {
    UpdateStatus s = UPDATED_NONE;

    // Retrieve the currently-stored timeval from the RTC
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // Convert the timeval to DateTime (in UTC)
    DateTime utc_datetime;
    _timevalToDateTime(tv, 0, utc_datetime);

    // Extract date values from GNSS
    if (get_date_valid())
    {
      utc_datetime.year = _gnss.getYear();
      utc_datetime.month = _gnss.getMonth();
      utc_datetime.day = _gnss.getDay();
      utc_datetime.day_of_week = _calculateDayOfWeek(utc_datetime.year % 100, utc_datetime.month, utc_datetime.day);

      DEBUG_PRINTF("Date is %02d/%02d/%d\n", utc_datetime.day, utc_datetime.month, utc_datetime.year);

      s = UPDATED_DATE_ONLY;
    }

    // Extract time values from GNSS
    if (get_time_valid())
    {
      utc_datetime.hour = _gnss.getHour();
      utc_datetime.minute = _gnss.getMinute();
      utc_datetime.second = _gnss.getSecond();

      DEBUG_PRINTF("Time is %02d:%02d:%02d\n", utc_datetime.hour, utc_datetime.minute, utc_datetime.second);

      if (s == UPDATED_DATE_ONLY)
      {
        s = UPDATED_BOTH;
      }
      else
      {
        s = UPDATED_TIME_ONLY;
      }
    }

    // Convert DateTime (UTC) acquired from GNSS to timeval
    _dateTimeToTimeval(utc_datetime, tv);

    // Set the system clock with the GNSS time
    settimeofday(&tv, NULL);

    // Update the last GNSS update time
    // only update it if we successfully read from GNSS
    if (s == UPDATED_BOTH)
    {
      _last_gnss_update_time = tv.tv_sec;
    }

    return s;
  }

  UpdateStatus get_gnss_datetime(int utc_offset, DateTime *datetime)
  {
    UpdateStatus s = UPDATED_NONE;

    DateTime utc_datetime;
    // Extract date values from GNSS
    if (get_date_valid())
    {
      utc_datetime.year = _gnss.getYear();
      utc_datetime.month = _gnss.getMonth();
      utc_datetime.day = _gnss.getDay();
      utc_datetime.day_of_week = _calculateDayOfWeek(utc_datetime.year % 100, utc_datetime.month, utc_datetime.day);

      s = UPDATED_DATE_ONLY;
    }

    // Extract time values from GNSS
    if (get_time_valid())
    {
      utc_datetime.hour = _gnss.getHour();
      utc_datetime.minute = _gnss.getMinute();
      utc_datetime.second = _gnss.getSecond();

      if (s == UPDATED_DATE_ONLY)
      {
        s = UPDATED_BOTH;
      }
      else
      {
        s = UPDATED_TIME_ONLY;
      }
    }

    *datetime = utc_datetime;
    return s;
  }

  void get_rtc_datetime(int utc_offset, DateTime *datetime)
  {
    // Retrieve time from RTC
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // Apply given UTC offset and convert it to a datetime variable.
    _timevalToDateTime(tv, utc_offset, *datetime);
  }

  uint16_t get_minutes_after_midnight()
  {
    gnss_time::DateTime dt;
    gnss_time::get_rtc_datetime(utils::configs::get_utc_offset(), &dt);
    uint16_t now = ((uint16_t)dt.hour * 60) + (uint16_t)dt.minute;
    return now;
  }

}