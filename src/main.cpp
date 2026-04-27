#include <Arduino.h>
#include "ble-schedule.h"
#include "gnss-time.h"
#include "remigraphics.h"
#include "rfid-remis.h"
#include "system-utils.h"

void setup()
{
  Serial.begin(115200);
  analogReadMilliVolts(BATT_MON);
  utils::buttons_init();

  while (!digitalRead(BUTTON_1))
  {
  }
  utils::sd_begin();

  char *schedule_name = ble_schedule::get_schedule_name();
  Serial.println(schedule_name);
  free(schedule_name);

  Serial.printf("Events: %d Remi: %d\n", ble_schedule::get_num_events(), ble_schedule::get_remi());

  ble_schedule::event_t test_event;

  ble_schedule::get_event(0, &test_event);
  Serial.print("Event name: ");
  Serial.println(test_event.name);
  Serial.print("Period: ");
  Serial.println(test_event.period);
  Serial.print("Start time: ");
  Serial.println(test_event.start_time);
  Serial.print("End time: ");
  Serial.println(test_event.end_time);
  Serial.print("Days of week: ");
  Serial.println(test_event.days_of_week);

  ble_schedule::get_event(1, &test_event);
  Serial.print("Event name: ");
  Serial.println(test_event.name);
  Serial.print("Period: ");
  Serial.println(test_event.period);
  Serial.print("Start time: ");
  Serial.println(test_event.start_time);
  Serial.print("End time: ");
  Serial.println(test_event.end_time);
  Serial.print("Days of week: ");
  Serial.println(test_event.days_of_week);
}

void loop()
{
}