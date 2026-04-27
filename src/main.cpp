#include <Arduino.h>
#include "ble-schedule.h"
#include "gnss-time.h"
#include "remigraphics.h"
#include "rfid-remis.h"
#include "system-utils.h"

void setup()
{
  pinMode(BUTTON_1, INPUT_PULLDOWN);
  pinMode(BUTTON_2, INPUT_PULLDOWN);
  pinMode(BUTTON_3, INPUT_PULLDOWN);
  pinMode(BUTTON_4, INPUT_PULLDOWN);

  // put your setup code here, to run once:
  Serial.begin(115200);

  while (!digitalRead(BUTTON_1))
  {
  }
  ble_schedule::start();
  ble_schedule::block_until_connected();

  while (!digitalRead(BUTTON_2))
  {
    ble_schedule::receive_schedule_data();
  }
  ble_schedule::save_schedule_to_sd();

  while (!digitalRead(BUTTON_3))
  {
  }
  ble_schedule::stop();
}

void loop()
{
}