
#include "system-utils.h"
#include "alarms.h"
namespace utils
{
  namespace configs
  {
    // NVS namespace for storing settings
    static const char *NVS_NAMESPACE = "configs";

    bool _sound;
    bool _vib;
    uint8_t _brightness;
    int _utc_offset;
    uint8_t _remi;
    uint8_t _health;
    bool values_loaded = false;

    void reset_values()
    {
      _sound = true;
      _vib = true;
      _brightness = 2;
      _utc_offset = -4;
      _remi = 1;
      _health = 14;
      values_loaded = true;
    }

    void write_sound_to_mem(bool sound)
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

    bool get_sound_from_mem(void)
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

    void write_vibrate_to_mem(bool vib)
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

    bool get_vibrate_from_mem(void)
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

    void write_brightness_to_mem(uint8_t brightness)
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

    uint8_t get_brightness_from_mem(void)
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

    void write_remi_to_mem(uint8_t remi)
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

    uint8_t get_remi_from_mem(void)
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

    void write_health_to_mem(uint8_t health)
    {
      nvs_handle_t handle;
      esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
      if (err == ESP_OK)
      {
        nvs_set_u8(handle, "health", health);
        nvs_commit(handle);
        nvs_close(handle);
      }
    }

    uint8_t get_health_from_mem(void)
    {
      nvs_handle_t handle;
      uint8_t value = 0; // Default remi character
      esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
      if (err == ESP_OK)
      {
        nvs_get_u8(handle, "health", &value);
        nvs_close(handle);
      }
      return value;
    }

    void write_utc_offset_to_mem(int offset)
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

    int get_utc_offset_from_mem(void)
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

    void load_values_from_memory()
    {
      _sound = get_sound_from_mem();
      _vib = get_vibrate_from_mem();
      _brightness = get_brightness_from_mem();
      _utc_offset = get_utc_offset_from_mem();
      _remi = get_remi_from_mem();
      _health = get_health_from_mem();

      values_loaded = true;
    }

    void write_values_to_memory()
    {
      write_sound_to_mem(_sound);
      write_vibrate_to_mem(_vib);
      write_brightness_to_mem(_brightness);
      write_utc_offset_to_mem(_utc_offset);
      write_remi_to_mem(_remi);
      write_health_to_mem(_health);
    }

    void init()
    {
      // Initialize NVS for settings storage
      esp_err_t err = nvs_flash_init();
      if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
      {
        nvs_flash_erase();
        nvs_flash_init();
        reset_values();
      }
      else
      {
        load_values_from_memory();
      }
    }

    void toggle_sound()
    {
      _sound = !_sound;
    }

    void toggle_vibrate()
    {
      _vib = !_vib;
    }

    void toggle_brightness()
    {
      if (_brightness == 1)
      {
        _brightness = 2;
      }
      else if (_brightness == 2)
      {
        _brightness = 1;
      }
    }

    void set_sound(bool sound)
    {
      _sound = sound;
    }

    bool get_sound()
    {
      return _sound;
    }

    void set_vibrate(bool vib)
    {
      _vib = vib;
    }

    bool get_vibrate()
    {
      return _vib;
    }

    void set_brightness(uint8_t brightness)
    {
      _brightness = brightness;
    }

    uint8_t get_brightness()
    {
      return _brightness;
    }

    void set_remi(uint8_t remi)
    {
      _remi = remi;
    }

    uint8_t get_remi()
    {
      return _remi;
    }

    void cycle_health()
    {
      _health -= 4;
      _health %= 15;
    }

    void set_health(uint8_t health)
    {
      _health = health;
    }

    uint8_t get_health()
    {
      return _health;
    }

    void increment_utc_offset()
    {
      _utc_offset++;
      if (_utc_offset > 12)
      {
        _utc_offset = 12;
      }
    }

    void decrement_utc_offset()
    {
      _utc_offset--;
      if (_utc_offset < -12)
      {
        _utc_offset = -12;
      }
    }

    void set_utc_offset(int utc)
    {
      _utc_offset = utc;
    }

    int get_utc_offset()
    {
      return _utc_offset;
    }

  }

  namespace sleep
  {

    void set_next_wakeup_time(uint16_t time)
    {
      uint16_t now = gnss_time::get_minutes_after_midnight();
      if (time <= now)
      {
        DEBUG_PRINTF("Error: wakeup time = %d but now = %d.  Device wakeup not set.\n", time, now);
        return;
      }

      uint16_t minutes = time - now; // minutes from now until wakeup
      uint64_t microseconds = minutes * 6e7;
      esp_sleep_enable_timer_wakeup(microseconds);
    }

    void trigger_sleep() {}

    void go_to_sleep()
    {
      configs::write_values_to_memory();
      set_next_wakeup_time(alarms::get_upcoming_alarm()->time);
      DEBUG_PRINTLN("--Going to sleep now.--");
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
        DEBUG_PRINTLN("SD card init failed");
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