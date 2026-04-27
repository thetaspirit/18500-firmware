#include "ble-schedule.h"

namespace ble_schedule
{
    uint8_t *buffer = NULL;
    int buff_idx = -1;

    /**
     * @brief Simply reads bytes into library's internal buffer until there are no more bytes available.
     * The buffer will overwrite the oldest data if it overflows.
     *
     * @return the number of bytes read
     */
    int read_bytes()
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
                    // save one byte into the buffer
                    buffer[buff_idx] = (uint8_t)data;
                    // Serial.printf("0x%x %c i = %d\n", buffer[buff_idx], buffer[buff_idx], buff_idx);
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
        // TODO add parsing logic
        // call read_bytes
        // inspect buffer to see how many event_ts are expected
        // call read_bytes again if expecting more data
        // handle timeout/give up as well

        if (read_bytes())
        {
            Serial.println();
        }

        return 0;
    }

    void stop()
    {
        NuSerial.stop();
        Serial.println("--Stopped--");
        free(buffer);
        buffer = NULL;
    }

    bool ble_is_connected()
    {
        return NuSerial.isConnected();
    }

    bool save_schedule_to_sd()
    {
        if (buffer == NULL)
        {
            Serial.println("Buffer is NULL.");
            return false;
        }

        for (int i = 0; i < buff_idx; i++)
        {
            Serial.printf("%x ", buffer[i]);
        }
        Serial.println("END OF BUFFER");
        return true;
    }

}