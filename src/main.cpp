#include <Arduino.h>

#include "ble-schedule.h"
#include "gnss-time.h"
#include "rfid-remis.h"
#include "system-utils.h"
#include "state-machine.h"
#include "alarms.h"

void print_states(void *parameters)
{
  while (true)
  {
    Serial.println("--States:--");
    Serial.printf("* Main state: %d\n", states::get_main());
    Serial.printf("* Menu state: %d\n", states::get_menu());
    Serial.printf("* Bluetooth state: %d\n", states::get_bluetooth());
    Serial.printf("* GNSS state: %d\n", states::get_gnss());
    Serial.printf("* Time state: %d\n", states::get_time());
    Serial.printf("* Timezone state: %d\n", states::get_timezone());
    Serial.printf("* Notif state: %d\n", states::get_notif());

    UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
    Serial.printf("Stack remaining: %u bytes\n", watermark);

    vTaskDelay(pdMS_TO_TICKS(15000));
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Beginning.");
  analogReadMilliVolts(BATT_MON);
  utils::buttons::init();
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
  digitalWrite(RED, LOW);
  digitalWrite(YELLOW, LOW);
  digitalWrite(GREEN, LOW);

  utils::configs::init();
  utils::shared_spi::init();
  utils::sd_card::init();
  rfid::init();
  gnss_time::init();
  alarms::init();

  states::init();

  xTaskCreate(
      print_states, // function
      "Debug Task", // name
      3000,         // stack size
      NULL,         // parameter
      1,            // priority
      NULL          // handle
  );

  xTaskCreatePinnedToCore(
      states::background_task, // function
      "Background Task",       // name
      8192,                    // stack size
      NULL,                    // parameter
      2,                       // priority
      NULL,                    // handle
      1                        // core
  );

  while (true)
  {
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void loop()
{
}