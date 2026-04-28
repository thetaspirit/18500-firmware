/**
 * Settings that the system should store on the SD card:
 * sound setting
 * vibration setting
 * brightness level
 * UTC offset
 * Remi character
 */

#pragma once
#include <SD.h>

#define BUTTON_1 14
#define BUTTON_2 15
#define BUTTON_3 16
#define BUTTON_4 48

#define BATT_MON 7

#define SHARED_COPI 38
#define SHARED_CIPO 39
#define SHARED_SCLK 40
#define SD_CS 41

#define RFID_CS 42
#define RFID_RST 47

namespace utils
{
  namespace configs
  {
    /**
     * All this is stuff that's saved in NVS
     */

    void init(void);

    void set_sound(bool sound);
    bool get_sound(void);

    void set_vibrate(bool vib);
    bool get_vibrate(void);

    /**
     * Value is a 2 for high or 1 for low.
     * All other numbers are invalid.
     */
    void set_brightness(uint8_t brightness);
    uint8_t get_brightness(void);

    void set_remi(uint8_t remi);
    uint8_t get_remi(void);

    void set_utc_offset(int offset);
    int get_utc_offset(void);

  }
  namespace buttons
  {
    /**
     * Initializes buttons.
     */
    void init(void);
  }

  namespace shared_spi
  {
    extern SPIClass shared_SPI;
    bool init();
  }

  namespace sd_card
  {

    /**
     * Important: utils::shared_spi::init() must be called BEFORE this function!
     * wrapper for SD.begin function
     */
    bool init();

    /**
     * Wrapper for SD.open function.
     * Don't forget to call close on this file when done!
     * Caller is responsible for calling close.
     */
    fs::File sd_open_file(const char *path, const char *mode);

    /**
     * Return status of SD card, including whether or not it was properly initialized
     *
     */
    bool get_sd_status();
  }
}