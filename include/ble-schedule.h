/**
 * Order of events:
 * 1. User puts Remigotchi into BLE paring mode.
 *    This calls the start() function, and then the block_until_connected() function.
 * 2. User begins to upload schedule through the desktop app.
 * 3. This app first sends the 'S' packet header to indicate an incoming schedule_t.
 * 4. The schedule_t packet will include the number of event_t's to expect, and Remigotchi will parse that
 *    to know how many bytes total to expect.
 * 5. The app will then send the 'E' packet header to indicate an incoming event_t.
 *    The app will continue to send event_t's preceeded by the 'E' header every time until all of the event_t's have been sent.
 * 6. Remigotchi will read ALL of that data into an internal buffer, and once it's received the last byte, it will say
 *    that the data has been transfered over successfully and will prompt the user to disconnect the Bluetooth.
 */

#pragma once

#include <NimBLEDevice.h>
#include "NuSerial.hpp"

#define DEVICE_NAME "Remigotchi 18500"

/* Size of buffer, in bytes */
#define BUFFER_SIZE 512

#define SCHEDULE_CHAR 'S'
#define EVENT_CHAR 'E'

namespace ble_schedule
{
    typedef struct
    {
        // 32 bytes
        char name[32];
        // 2 bytes
        uint16_t period;
        uint16_t start_time;
        uint16_t end_time;
        // 1 byte
        uint8_t days_of_week;
        uint8_t padding[1];
    } event_t; // 40 bytes total

    typedef struct
    {
        char name[32];
        uint8_t num_events;
    } schedule_t; // 33 bytes total

    /**
     * Calls internal init functions/setup and starts advertising.
     * Allocates memory for internal buffer to store data from BLE.
     */
    void start(void);

    /**
     * Blocks until connection is acquired.
     * Will actually wait forever.
     */
    void block_until_connected(void);

    /**
     * Receives data from BLE and reads it into internal buffer.
     * This function IS blocking, and if no data is received, it will timeout.
     * @return 0 = all good, 1 = timeout (no data at all), 2 = other problem (some data received, not not as expected)
     */
    uint8_t receive_schedule_data(void);

    /**
     * Calls internal NuSerial stop function.
     * Frees internal buffer.
     */
    void stop(void);

    /**
     * @return Whether or not the device is connected to another device.
     */
    bool ble_is_connected(void);

    /**
     * Saves data in the internal buffer to the SD card.
     * (Data in buffer remains untouched).
     * NOTE: Do NOT call this function before stop(), when the internal buffer gets de-allocated.
     * @return False if SD card is missing or some other problem (like no buffer)
     */
    bool save_schedule_to_sd(void);

    /**
     * Gets the name of the schedule currently loaded into Remigotchi's SD card.
     * Undefined behavior if there is no schedule loaded into Remigotchi.
     */
    char *get_schedule_name(uint8_t schedule_idx);

    /**
     * Gets the number of events in the schedule currently loaded into Remigotchi's SD card.
     * Undefined behavior if there is no schedule loaded into Remigotchi.
     */
    uint8_t get_num_events(uint8_t schedule_idx);

    /**
     * Provide access to a specific event, identified by event_idx.
     * @param event_idx Index of which event to get.  Valid range: [0, total # of events -1]
     * @param e Variable to provide caller access to an event_t.
     * Undefined behavior if there is no schedule loaded into Remigotchi.
     * Undefined behavior if event_idx is outside the valid range.
     */
    void get_event(uint8_t event_idx, event_t &e);
}