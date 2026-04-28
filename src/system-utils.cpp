#include "system-utils.h"
#include <nvs_flash.h>
#include <nvs.h>
#include <esp_sleep.h>
#include <sys/time.h>
#include <time.h>

namespace utils
{
  namespace configs
  {
    // NVS namespace for storing settings
    static const char *NVS_NAMESPACE = "configs";

    void init()
    {
      // Initialize NVS for settings storage
      esp_err_t err = nvs_flash_init();
      if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
      {
        nvs_flash_erase();
        nvs_flash_init();
      }
    }

    void set_sound(bool sound)
    {
      nvs_handle_t handle;
      esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
      if (err == ESP_OK)
      {
        nvs_set_u8(handle, "sound", (uint8_t)sound);
        nvs_commit(handle);
        nvs_close(handle);
      }
    }

    bool get_sound(void)
    {
      nvs_handle_t handle;
      uint8_t value = 1; // Default to enabled
      esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
      if (err == ESP_OK)
      {
        nvs_get_u8(handle, "sound", &value);
        nvs_close(handle);
      }
      return (bool)value;
    }

    void set_vibrate(bool vib)
    {
      nvs_handle_t handle;
      esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
      if (err == ESP_OK)
      {
        nvs_set_u8(handle, "vibrate", (uint8_t)vib);
        nvs_commit(handle);
        nvs_close(handle);
      }
    }

    bool get_vibrate(void)
    {
      nvs_handle_t handle;
      uint8_t value = 1; // Default to enabled
      esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
      if (err == ESP_OK)
      {
        nvs_get_u8(handle, "vibrate", &value);
        nvs_close(handle);
      }
      return (bool)value;
    }

    void set_brightness(uint8_t brightness)
    {
      nvs_handle_t handle;
      esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
      if (err == ESP_OK)
      {
        nvs_set_u8(handle, "brightness", brightness);
        nvs_commit(handle);
        nvs_close(handle);
      }
    }

    uint8_t get_brightness(void)
    {
      nvs_handle_t handle;
      uint8_t value = 2; // Default brightness (high)
      esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
      if (err == ESP_OK)
      {
        nvs_get_u8(handle, "brightness", &value);
        nvs_close(handle);
      }
      return value;
    }

    void set_remi(uint8_t remi)
    {
      nvs_handle_t handle;
      esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
      if (err == ESP_OK)
      {
        nvs_set_u8(handle, "remi", remi);
        nvs_commit(handle);
        nvs_close(handle);
      }
    }

    uint8_t get_remi(void)
    {
      nvs_handle_t handle;
      uint8_t value = 0; // Default remi character
      esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
      if (err == ESP_OK)
      {
        nvs_get_u8(handle, "remi", &value);
        nvs_close(handle);
      }
      return value;
    }

    void set_utc_offset(int offset)
    {
      nvs_handle_t handle;
      esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
      if (err == ESP_OK)
      {
        nvs_set_i32(handle, "utc_offset", offset);
        nvs_commit(handle);
        nvs_close(handle);
      }
    }

    int get_utc_offset(void)
    {
      nvs_handle_t handle;
      int32_t value = 0; // Default UTC+0
      esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
      if (err == ESP_OK)
      {
        nvs_get_i32(handle, "utc_offset", &value);
        nvs_close(handle);
      }
      return (int)value;
    }

  }

  namespace sleep
  {
    /**
     * @brief Helper function to convert a DateTime struct to a Unix timestamp (time_t)
     *
     * @param dt The DateTime struct to convert
     * @return time_t The Unix timestamp (seconds since epoch)
     */
    static time_t _dateTimeToUnixTime(const gnss_time::DateTime &dt)
    {
      // Count days since epoch (1970-01-01)
      uint32_t days = 0;

      // Count leap years from 1970 to year-1
      for (uint16_t y = 1970; y < dt.year; y++)
      {
        if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))
          days += 366;
        else
          days += 365;
      }

      // Count days for months in the current year
      const uint8_t daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
      bool isLeapYear = (dt.year % 4 == 0 && dt.year % 100 != 0) || (dt.year % 400 == 0);

      for (uint8_t m = 1; m < dt.month; m++)
      {
        days += daysInMonth[m];
        if (m == 2 && isLeapYear)
          days++;
      }

      // Add days in current month
      days += dt.day - 1;

      // Convert to seconds and add time components
      return (time_t)days * 86400 + dt.hour * 3600 + dt.minute * 60 + dt.second;
    }

    void set_next_wakeup_time(const gnss_time::DateTime &wakeup_time)
    {
      // Get current time
      struct timeval now;
      gettimeofday(&now, NULL);
      time_t current_time = now.tv_sec;

      // Convert target wakeup time to Unix timestamp
      time_t wakeup_timestamp = _dateTimeToUnixTime(wakeup_time);

      // Calculate seconds until wakeup
      int64_t seconds_until_wakeup = wakeup_timestamp - current_time;

      // If the target time has already passed today, schedule for tomorrow
      if (seconds_until_wakeup <= 0)
      {
        seconds_until_wakeup += 86400; // Add one day (86400 seconds)
      }

      // Convert seconds to microseconds and configure the timer
      uint64_t microseconds = (uint64_t)seconds_until_wakeup * 1000000;
      esp_sleep_enable_timer_wakeup(microseconds);
    }

    void go_to_sleep()
    {
      esp_deep_sleep_start();
    }
  }
  namespace buttons
  {
    void init()
    {
      pinMode(BUTTON_1, INPUT_PULLDOWN);
      pinMode(BUTTON_2, INPUT_PULLDOWN);
      pinMode(BUTTON_3, INPUT_PULLDOWN);
      pinMode(BUTTON_4, INPUT_PULLDOWN);
    }

    void wait_for_button_release(int pin_num)
    {
      while (digitalRead(pin_num))
      {
        vTaskDelay(10);
      }
      vTaskDelay(DEBOUNCE);
    }
  }

  namespace shared_spi
  {
    SPIClass shared_SPI;
    bool init(void)
    {
      // Initialize the shared SPI bus
      // Note: chip select pin is not used here as CS is handled per-device
      shared_SPI.begin(SHARED_SCLK, SHARED_CIPO, SHARED_COPI);
      return true;
    }
  }

  namespace sd_card
  {
    bool sd_init = false;

    /**
     * Call before accessing SD card to ensure proper bus control
     */
    void select_sd()
    {
      // Deselect RFID reader
      digitalWrite(RFID_CS, HIGH);
      vTaskDelay(1);
      // Select SD card
      digitalWrite(SD_CS, LOW);
      vTaskDelay(1);
    }

    /**
     * Call after SD card access completes
     */
    void deselect_sd()
    {
      // Release SD card
      digitalWrite(SD_CS, HIGH);
      vTaskDelay(1);
    }

    bool init()
    {
      // Setup SD card CS pin
      pinMode(SD_CS, OUTPUT);
      digitalWrite(SD_CS, HIGH); // CS idle HIGH

      // Temporarily deselect RFID reader to avoid bus conflicts
      pinMode(RFID_CS, OUTPUT);
      digitalWrite(RFID_CS, HIGH);

      // Initialize SD card WITHOUT passing CS pin - we'll manage it manually
      // Use SD.begin with just the SPI object
      sd_init = SD.begin(SD_CS, shared_spi::shared_SPI);
      if (!sd_init)
      {
        Serial.println("SD card init failed");
      }

      // After init, ensure SD CS is released
      digitalWrite(SD_CS, HIGH);

      return sd_init;
    }

    fs::File sd_open_file(const char *path, const char *mode)
    {
      select_sd();
      fs::File file = SD.open(path, mode);
      deselect_sd();
      return file;
    }

    bool get_sd_status()
    {
      return sd_init;
    }
  }
}