#include "ble-schedule.h"

namespace ble_schedule
{
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
        if (buffer == NULL || buff_idx == -1)
        {
            Serial.println("Buffer is NULL.");
            return false;
        }
        if (!utils::get_sd_status())
        {
            Serial.println("Can't write to SD card.");
            return false;
        }

        File sched_file = utils::sd_open_file(SCHEDULE_SD_FILEPATH, FILE_WRITE);
        sched_file.write(buffer, buff_idx);
        sched_file.close();
        Serial.println("Data saved");
        return true;
    }

    char *get_schedule_name()
    {
        File sched_file = utils::sd_open_file(SCHEDULE_SD_FILEPATH, FILE_READ);
        if (!sched_file)
        {
            Serial.println("Failed to open schedule file.");
            return NULL;
        }

        // Skip the first byte ('S')
        sched_file.read();

        // Read 32 bytes of schedule name
        char name_buffer[32]; // 32 bytes + null terminator
        sched_file.read((uint8_t *)name_buffer, 32);

        sched_file.close();

        // Allocate memory for the returned string
        char *schedule_name = (char *)malloc(strlen(name_buffer) + 1);
        strcpy(schedule_name, name_buffer);

        return schedule_name;
    }

    uint8_t get_num_events()
    {
        File sched_file = utils::sd_open_file(SCHEDULE_SD_FILEPATH, FILE_READ);
        if (!sched_file)
        {
            Serial.println("Failed to open schedule file.");
            return 0;
        }

        // Skip the first byte ('S') and 32 bytes of schedule name
        sched_file.seek(33);

        // Read the byte representing the number of events
        uint8_t num_events = sched_file.read();

        sched_file.close();

        return num_events;
    }

    uint8_t get_remi()
    {
        File sched_file = utils::sd_open_file(SCHEDULE_SD_FILEPATH, FILE_READ);
        if (!sched_file)
        {
            Serial.println("Failed to open schedule file.");
            return 0;
        }

        // Skip the first byte ('S'), 32 bytes of schedule name, and one byte for number of events
        sched_file.seek(34);

        // Read the byte representing the number of events
        uint8_t remi = sched_file.read();

        sched_file.close();

        return remi;
    }

    void get_event(uint8_t event_idx, event_t *e)
    {
        File sched_file = utils::sd_open_file(SCHEDULE_SD_FILEPATH, FILE_READ);
        if (!sched_file)
        {
            Serial.println("Failed to open schedule file.");
            return;
        }

        // Read the number of events directly from the open file
        sched_file.seek(33);
        uint8_t num_events = sched_file.read();

        // Check if event_idx is within bounds
        if (event_idx >= num_events)
        {
            Serial.println("Event index out of range.");
            sched_file.close();
            return;
        }

        // Calculate the file position of the requested event
        // File layout: [S(1)] [name(32)] [num_events(1)] [remi(1)] [E(1) + event_t(39)]*
        // Position = 35 (header) + event_idx * 40 (1 byte 'E' + 39 bytes event_t)
        int file_position = 35 + event_idx * 40;

        // Seek to the 'E' character
        sched_file.seek(file_position);

        // Skip the 'E' character
        sched_file.read();

        // Read 39 bytes into the event_t structure
        sched_file.read((uint8_t *)e, 39);

        // Convert big-endian uint16_t fields to little-endian
        e->period = (e->period >> 8) | ((e->period & 0xFF) << 8);
        e->start_time = (e->start_time >> 8) | ((e->start_time & 0xFF) << 8);
        e->end_time = (e->end_time >> 8) | ((e->end_time & 0xFF) << 8);

        sched_file.close();
    }

}