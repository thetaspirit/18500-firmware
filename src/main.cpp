#include <Arduino.h>
#include "ble-schedule.h"
#include "gnss-time.h"
#include "rfid-remis.h"
#include "system-utils.h"
#include "state-machine.h"
#include "alarms.h"

void print_daily_schedule()
{
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

void print_schedule()
{
  int num_events = ble_schedule::get_num_events();

  for (int i = 0; i < num_events; i++)
  {
    ble_schedule::event_t e;
    ble_schedule::get_event(i, &e);
    Serial.printf("Event: %s\nPeriod: %d\nStart: %d End %d\n",
                  e.name, e.period, e.start_time, e.end_time);
  }
}

void print_monday_alarms()
{
  // Test event_list_to_alarm_list
  Serial.println("\nTesting event_list_to_alarm_list:");
  int num_events = 0;
  ble_schedule::event_list *events = ble_schedule::load_today_schedule(1, num_events); // Load Monday's schedule

  if (events != nullptr)
  {
    Serial.print("Loaded ");
    Serial.print(num_events);
    Serial.println(" events for Monday");

    // Convert event list to alarm list
    alarms::alarm_list *alarms_list = alarms::event_list_to_alarm_list(events);

    if (alarms_list != nullptr)
    {
      Serial.println("Successfully converted event list to alarm list:");

      alarms::alarm_list *curr_alarm = alarms_list;
      int alarm_count = 0;
      while (curr_alarm != nullptr)
      {
        Serial.print("  Alarm ");
        Serial.print(alarm_count);
        Serial.print(": ");
        Serial.print(curr_alarm->curr_a->e->name);
        Serial.print(" at ");
        Serial.println(curr_alarm->curr_a->time);

        curr_alarm = curr_alarm->next_a;
        alarm_count++;
      }

      Serial.print("Total alarms: ");
      Serial.println(alarm_count);

      // Free the alarm list
      alarms::free_alarm_list(alarms_list);
      Serial.println("Alarm list freed successfully");
    }
    else
    {
      Serial.println("Failed to convert event list to alarm list");
    }

    // Free the event list
    ble_schedule::free_event_list(events);
  }
  else
  {
    Serial.println("No events found for Monday");
  }
  Serial.println("event_list_to_alarm_list test complete!\n");
}

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