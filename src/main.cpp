#include <Arduino.h>
#include "ble-schedule.h"
#include "gnss-time.h"
#include "remigraphics.h"
#include "rfid-remis.h"
#include "system-utils.h"
#include "state-machine.h"

void setup()
{
  Serial.begin(115200);
  Serial.println("Beginning.");
  analogReadMilliVolts(BATT_MON);
  utils::buttons::init();

  utils::configs::init();
  utils::shared_spi::init();
  utils::sd_card::init();

  while (!digitalRead(BUTTON_1))
  {
  }

  rfid::init();

  // Test load_today_schedule function
  Serial.println("Testing load_today_schedule:");
  const char *days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

  for (int day = 1; day <= 7; day++)
  {
    int num_events = 0;
    ble_schedule::event_list *events = ble_schedule::load_today_schedule(day, num_events);

    Serial.print(days[day - 1]);
    Serial.print(" (");
    Serial.print(num_events);
    Serial.println(" events):");

    ble_schedule::event_list *curr = events;
    while (curr != nullptr)
    {
      Serial.print("  - ");
      Serial.println(curr->curr_e->name);
      curr = curr->next_e;
    }

    ble_schedule::free_event_list(events);
  }
  Serial.println("load_today_schedule test complete!");
}

void loop()
{
}