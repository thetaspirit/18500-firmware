#include <Arduino.h>

#include "ble-schedule.h"
#include "gnss-time.h"
#include "rfid-remis.h"
#include "system-utils.h"
#include "state-machine.h"
#include "alarms.h"
#include "debug.h"

void print_states(void *parameters)
{
  while (true)
  {
    DEBUG_PRINTLN("--States:--");
    DEBUG_PRINTF("* Main state: %d\n", states::get_main());
    DEBUG_PRINTF("* Menu state: %d\n", states::get_menu());
    DEBUG_PRINTF("* Bluetooth state: %d\n", states::get_bluetooth());
    DEBUG_PRINTF("* GNSS state: %d\n", states::get_gnss());
    DEBUG_PRINTF("* Time state: %d\n", states::get_time());
    DEBUG_PRINTF("* Timezone state: %d\n", states::get_timezone());
    DEBUG_PRINTF("* Notif state: %d\n", states::get_notif());

    UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
    DEBUG_PRINTF("Stack remaining: %u bytes\n", watermark);

    vTaskDelay(pdMS_TO_TICKS(15000));
  }
}

void setup()
{
  Serial.begin(115200);
  DEBUG_PRINTLN("Beginning.");
  analogReadMilliVolts(BATT_MON);
  utils::buttons::init();

  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
  digitalWrite(RED, LOW);
  digitalWrite(YELLOW, LOW);
  digitalWrite(GREEN, LOW);

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, HIGH);

  pinMode(MOTOR, OUTPUT);
  digitalWrite(MOTOR, HIGH);

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