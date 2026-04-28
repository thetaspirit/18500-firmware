#include "system-utils.h"
#include <nvs_flash.h>
#include <nvs.h>

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
  namespace buttons
  {
    void init()
    {
      pinMode(BUTTON_1, INPUT_PULLDOWN);
      pinMode(BUTTON_2, INPUT_PULLDOWN);
      pinMode(BUTTON_3, INPUT_PULLDOWN);
      pinMode(BUTTON_4, INPUT_PULLDOWN);
    }
  }

  namespace sd_card
  {
    SPIClass shared_SPI;
    bool sd_init = false;

    bool init()
    {
      shared_SPI.begin(SHARED_SCLK, SHARED_CIPO, SHARED_COPI, SHARED_CS);
      sd_init = SD.begin(SHARED_CS, shared_SPI);
      if (!sd_init)
      {
        Serial.println("SD card init failed");
      }
      return sd_init;
    }

    fs::File sd_open_file(const char *path, const char *mode)
    {
      return SD.open(path, mode);
    }

    bool get_sd_status()
    {
      return sd_init;
    }
  }
}