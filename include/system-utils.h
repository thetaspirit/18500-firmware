/**
 * Settings that the system should store on the SD card:
 * sound setting
 * vibration setting
 * brightness level
 * UTC offset
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
#define SHARED_CS 41

namespace utils
{
    /**
     * Initializes buttons.
     */
    void buttons_init(void);

    /**
     * wrapper for SD.begin function
     */
    bool sd_begin();

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