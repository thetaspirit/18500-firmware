#include "ble-schedule.h"

namespace ble_schedule
{
    uint8_t fake_data[] = {
        0x53, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x74,
        0x65, 0x73, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x02, 0x03, 0x45, 0x72, 0x65, 0x6D, 0x69,
        0x6E, 0x64, 0x65, 0x72, 0x5F, 0x31, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x3C, 0x01, 0x68, 0x04, 0x38,
        0x3E, 0x45, 0x72, 0x65, 0x6D, 0x69, 0x6E, 0x64,
        0x65, 0x72, 0x5F, 0x32, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0xA0,
        0x01, 0xA4, 0x03, 0xFC, 0x7F};

    uint8_t *buffer = NULL;
    int buff_idx = -1;

    int events_expected = -1;
    uint8_t events_seen = 0;
    uint8_t events_received = 0;
    bool parse_error = false;

    /**
     * @brief Simply reads bytes into library's internal buffer until there are no more bytes available.
     * The buffer will overwrite the oldest data if it overflows.
     *
     * @return the number of bytes read
     */
    int parse_bytes()
    {
        int b = 0; // the number of bytes read
        if (NuSerial.isConnected() && buffer != NULL && buff_idx != -1)
        {
            // If connected to a BLE host, read bytes into the buffer until there are no more bytes to be read.
            // This control loop is heavily dependent on how fast data is being sent and how fast we're reading it from the BLE Serial buffer.
            // It is recommended to put this function inside some other control loop as well.
            while (NuSerial.available())
            {
                // read one byte
                int data = NuSerial.read();
                if (data != -1)
                {
                    // if this byte is the number of events, save it
                    if (buff_idx == 33)
                    {
                        events_expected = (int)data;
                        Serial.printf("Expecting to receive %d events.\n", events_expected);
                    }
                    // looking for EVENT_CHAR bytes
                    if (buff_idx % 40 == 35)
                    {
                        if (data == EVENT_CHAR)
                        {
                            events_seen++;
                        }
                        else
                        {
                            parse_error = true;
                        }
                    }
                    // looking for strings of completed events
                    if (buff_idx % 40 == 34 && events_seen > 1)
                    {
                        events_received++;
                    }

                    // save one byte into the buffer
                    buffer[buff_idx] = (uint8_t)data;
                    Serial.printf("0x%x ", buffer[buff_idx]);

                    buff_idx++;
                    buff_idx %= BUFFER_SIZE;

                    b++;
                }
            }
        }
        return b;
    }

    void reset_buffer()
    {
        free(buffer);
        buffer = NULL;
        buffer = (uint8_t *)malloc(BUFFER_SIZE * sizeof(uint8_t));
        buff_idx = 0;
    }

    void start()
    {
        Serial.println("--Initializing--");

        // Initialize BLE stack and Nordic UART service
        NimBLEDevice::init(DEVICE_NAME);
        NimBLEDevice::getAdvertising()->setName(DEVICE_NAME);
        NuSerial.setTimeout(ULONG_MAX); // no timeout at readBytes()
        NuSerial.start();
        // Initialization complete

        buffer = (uint8_t *)malloc(BUFFER_SIZE * sizeof(uint8_t));
        buff_idx = 0;

        Serial.println("--Ready--");
    }

    void block_until_connected()
    {
        if (!NuSerial.isConnected())
        {
            Serial.println("--Waiting for connection--");
            if (NuSerial.connect()) // blocking check
            {
                Serial.println("--Connected--");
            }
        }
    }

    uint8_t receive_schedule_data(void)
    {
        int bytes_parsed = 0;
        unsigned long start_ms = millis();

        do
        {
            bytes_parsed += parse_bytes();
        } while (parse_error == 0 && (events_expected > events_received || millis() - start_ms < SCHEDULE_RECEIVE_TIMEOUT_MS));

        if (parse_error)
        {
            return 2;
        }
        if (bytes_parsed == 0)
        {
            return 1;
        }
        return 0;
    }

    void stop()
    {
        NuSerial.stop();
        Serial.println("--Stopped--");
        free(buffer);
        buffer = NULL;
        buff_idx = -1;
    }

    bool ble_is_connected()
    {
        return NuSerial.isConnected();
    }

    bool save_schedule_to_sd()
    {
        Serial.println("Saving data");
        File sched_file = utils::sd_open_file(SCHEDULE_SD_FILEPATH, FILE_WRITE);
        sched_file.write(fake_data, sizeof(fake_data));
        sched_file.close();
        Serial.println("Data saved");
        return true;

        // if (buffer == NULL || buff_idx == -1)
        // {
        //     Serial.println("Buffer is NULL.");
        //     return false;
        // }
        // if (!utils::get_sd_status())
        // {
        //     Serial.println("Can't write to SD card.");
        //     return false;
        // }

        // File sched_file = utils::sd_open_file(SCHEDULE_SD_FILEPATH, FILE_WRITE);
        // sched_file.write(buffer, buff_idx);
        // sched_file.close();
        // Serial.println("Data saved");
        // return true;
    }

    bool read_schedule_from_sd()
    {
    }

}