#include "ble-schedule.h"

namespace ble_schedule
{
    uint8_t *buffer = NULL;
    int buff_idx = -1;

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

    void stop()
    {
        NuSerial.stop();
            Serial.println("--Stopped--");
        free(buffer);
    }

    bool ble_is_connected() {
        return NuSerial.isConnected();
    }

    void read_bytes()
    {
        if (NuSerial.isConnected() && buffer != NULL && buff_idx != -1)
        {
            // If connected to a BLE host, read bytes into the buffer until there are no more bytes to be read.
            // This control loop is heavily dependent on how fast data is being sent and how fast we're reading it from the BLE Serial buffer.
            // It is recommended to put this function inside some other control loop as well.
            while (NuSerial.available())
            {
                int data = NuSerial.read();
                if (data != -1)
                {
                    buffer[buff_idx] = (uint8_t)data;
                    Serial.printf("0x%x %c i = %d\n", buffer[buff_idx], buffer[buff_idx], buff_idx);

                    buff_idx++;
                    buff_idx %= BUFFER_SIZE;
                }
            }
        }
    }

    void reset_buffer()
    {
        free(buffer);
        buffer = (uint8_t *)malloc(BUFFER_SIZE * sizeof(uint8_t));
        buff_idx = 0;
    }

}