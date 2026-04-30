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
#include <nvs_flash.h>
#include <nvs.h>
#include <esp_sleep.h>
#include <sys/time.h>
#include <time.h>
#include "gnss-time.h"
#include "debug.h"

#define RED 35
#define YELLOW 36
#define GREEN 37

#define BUTTON_1 14
#define BUTTON_2 15
#define BUTTON_3 16
#define BUTTON_4 48
#define DEBOUNCE 100

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

    void toggle_sound(void);
    void toggle_vibrate(void);
    void toggle_brightness(void);

    void set_sound(bool sound);
    bool get_sound(void);

    void set_vibrate(bool vib);
    bool get_vibrate(void);

    /**
     * 2 = high (very bright), 1 = low (less bright)
     * All other numbers are invalid.
     */
    void set_brightness(uint8_t brightness);
    /**
     * 2 = high (very bright), 1 = low (less bright)
     * All other numbers are invalid.
     */
    uint8_t get_brightness(void);

    void set_remi(uint8_t remi);
    uint8_t get_remi(void);

    /**
     * 2 = happy/idle, 1 = sad
     */
    void set_emotion(uint8_t emotion);
    /**
     * 2 = happy/idle, 1 = sad
     */
    uint8_t get_emotion(void);

    void increment_utc_offset(void);
    void decrement_utc_offset(void);
    void set_utc_offset(int offset);
    int get_utc_offset(void);

    /**
     * Saves all of the variables at their current states to memory.
     * Call this function before sleeping.
     * Also call this function somewhat periodically so that if Remigotchi spontaneously dies, maybe the data won't be lost.
     */
    void write_values_to_memory(void);

  }
  namespace sleep
  {
    /**
     * Based on inactivity, returns whether or not device should sleep.
     */
    bool is_time_to_sleep(void);

    /**
     * Reset the sleep timer.
     */
    void reset_sleep_timer(void);

    /**
     * Put a specific amount of time on the clock before sleep.
     */
    void set_sleep_timer_ms(unsigned long ms);

    /**
     * @brief Sets the ESP32 to wake up at a specific time of day.
     *
     * This function configures the ESP32's internal RTC timer to trigger a wake-up
     * from deep sleep at the specified time. If the provided time is earlier than
     * the current time, the device will not wake up at all.
     *
     * @param wakeup_time An integer representing a time of day (presumeably one after the time it gets called), in minutes after midnight.
     * If the time listed does happen to be before the time of calling, a wakeup is not set.
     *
     * @note Call this before go_to_sleep() to enable timed wake-up.
     * @note The RTC should be kept reasonably accurate for reliable wake-up timing.
     */
    void set_next_wakeup_time(uint16_t time);

    /**
     * Handles any state/variable changes needed before going to sleep, then puts Remigotchi into deep sleep.
     */
    void go_to_sleep(void);
  }
  namespace buttons
  {
    /**
     * Initializes buttons.
     */
    void init(void);

    /**
     * This function blocks its thread until the given button is released.
     * Accounts for debouncing.
     */
    void wait_for_button_release(int pin_num);

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