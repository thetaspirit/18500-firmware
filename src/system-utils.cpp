#include "system-utils.h"

namespace utils
{

    SPIClass shared_SPI;

    bool sd_init = false;

    void buttons_init()
    {
        pinMode(BUTTON_1, INPUT_PULLDOWN);
        pinMode(BUTTON_2, INPUT_PULLDOWN);
        pinMode(BUTTON_3, INPUT_PULLDOWN);
        pinMode(BUTTON_4, INPUT_PULLDOWN);
    }

    bool sd_begin()
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