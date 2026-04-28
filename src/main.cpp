#include <Arduino.h>
#include <nvs_flash.h>
#include "ble-schedule.h"
#include "gnss-time.h"
#include "remigraphics.h"
#include "rfid-remis.h"
#include "system-utils.h"

void setup()
{
  Serial.begin(115200);
  analogReadMilliVolts(BATT_MON);
  utils::buttons::init();

  utils::configs::init();

  while (!digitalRead(BUTTON_1))
  {
  }

  // Test NVS settings implementation
  Serial.printf("\n=== Testing NVS Settings Implementation ===\n");

  // Test sound setting
  // utils::configs::set_sound(true);
  bool sound = utils::configs::get_sound();
  Serial.printf("Sound: %s\n", sound ? "enabled" : "disabled");

  // Test vibration setting
  // utils::configs::set_vibrate(true);
  bool vibrate = utils::configs::get_vibrate();
  Serial.printf("Vibration: %s\n", vibrate ? "enabled" : "disabled");

  // Test brightness setting
  // utils::configs::set_brightness(2);
  uint8_t brightness = utils::configs::get_brightness();
  Serial.printf("Brightness: %d\n", brightness);

  // Test remi character setting
  // utils::configs::set_remi(3);
  uint8_t remi = utils::configs::get_remi();
  Serial.printf("Remi Character: %d\n", remi);

  // Test UTC offset setting
  // utils::configs::set_utc_offset(-5);
  int utc_offset = utils::configs::get_utc_offset();
  Serial.printf("UTC Offset: %d\n", utc_offset);

  Serial.printf("=== NVS Settings Test Complete ===\n\n");
}

void loop()
{
}