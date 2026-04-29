
#include "alarms.h"
#include "state-machine.h"
#include <cstring>

namespace alarms
{
  esp_timer_handle_t alarm;

  alarm_t *upcoming_alarm;                // always points to the alarm that is set up to go off next
  alarm_t *current_alarm;                 // is NULL unless there is supposed to be an alarm firing right now
  alarm_list *today_alarms;               // always points to the head of the alarm list
  ble_schedule::event_list *today_events; // always points to the head of the event list

  void init()
  {
    esp_timer_create_args_t timer_args = {
        .callback = &alarm_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "remigotchi_alarm"};
    esp_timer_create(&timer_args, &alarm);

    gnss_time::DateTime dt;
    gnss_time::get_rtc_datetime(utils::configs::get_utc_offset(), &dt);
    setup_day(dt.day_of_week);
  }

  void setup_day(uint8_t day)
  {
    // de-allocate current lists
    free_alarm_list(today_alarms);
    ble_schedule::free_event_list(today_events);

    // get the specified day's lists
    int num_events = 0;
    today_events = ble_schedule::load_today_schedule(day, num_events);
    today_alarms = event_list_to_alarm_list(today_events);
  }

  alarm_t *find_next_alarm()
  {
    // traverse today_alarms to find the next one after now
    uint16_t now = gnss_time::get_minutes_after_midnight();
    gnss_time::DateTime dt;
    gnss_time::get_rtc_datetime(utils::configs::get_utc_offset(), &dt);
    uint8_t day = dt.day_of_week;

    alarm_list *al = today_alarms;

    while (al != nullptr)
    {
      if (al->curr_a->time >= now)
      {
        return al->curr_a;
      }
      al = al->next_a;
    }

    // if we reach this point, either al = nullptr, or all the times in the list are before now.
    // either way, we setup for tomorrow.
    uint8_t tomorrow = (day % 7) + 1;
    Serial.printf("No more alarms today.  Setting up day %d's alarms.\n", tomorrow);
    setup_day(tomorrow);

    // NOTE you could do recursion with a return find_next_alarm(); but for safety's sake, we'll return the first alarm on tomorrow's list
    if (today_alarms != nullptr)
    {
      return today_alarms->curr_a;
    }
    return nullptr;
  }

  alarm_t *get_upcoming_alarm()
  {
    return upcoming_alarm;
  }

  alarm_t *get_current_alarm()
  {
    return current_alarm;
  }

  void set_alarm_interrupt(alarm_t *a)
  {
    uint16_t now = gnss_time::get_minutes_after_midnight();
    uint16_t minutes = a->time - now; // minutes from now until alarm
    uint64_t microseconds = minutes * 6e7;
    esp_timer_start_once(alarm, microseconds);
  }

  void clear_alarm_interrupts()
  {
    esp_timer_stop(alarm);
  }

  void alarm_callback(void *arg)
  {
    states::set_alarm_state();

    current_alarm = upcoming_alarm;
  }

  void respond_to_alarm(Response r)
  {
    if (current_alarm == NULL)
    {
      return;
    }

    File log_file = utils::sd_card::sd_open_file(ALARM_LOG_FILEPATH, FILE_APPEND);
    if (!log_file)
    {
      Serial.println("Failed to open schedule file.");
    }
    else
    {
      gnss_time::DateTime dt;
      gnss_time::get_rtc_datetime(utils::configs::get_utc_offset(), &dt);
      log_file.printf("%02d/%02d/%04d %02d:%02d,%s", dt.day, dt.month, dt.year, dt.hour, dt.minute, current_alarm->e->name);
      switch (r)
      {
      case Response::DONE:
        log_file.println("DONE");
        break;
      case Response::IGNORE:
        log_file.println("IGNORE");
        break;
      case Response::SNOOZE:
        log_file.println("SNOOZE");
        break;
      default:
        log_file.println("blank");
        break;
      }
      log_file.close();
    }

    current_alarm = NULL;

    // TODO if r == SNOOZE, add in insert an alarm_t into today's list of alarms

    upcoming_alarm = find_next_alarm();
    set_alarm_interrupt(upcoming_alarm);
  }

  /**
   * @brief Helper function to insert an alarm into a sorted linked list
   * Maintains chronological order based on alarm time
   *
   * @param head Pointer to the head of the alarm_list (may be nullptr)
   * @param alarm Pointer to the alarm_t to insert
   * @return Pointer to the (possibly new) head of the list
   */
  static alarm_list *_insert_alarm_sorted(alarm_list *head, alarm_t *alarm)
  {
    // Create new node
    alarm_list *new_node = (alarm_list *)malloc(sizeof(alarm_list));
    if (new_node == nullptr)
      return head;

    new_node->curr_a = alarm;
    new_node->next_a = nullptr;

    // If list is empty, return new node as head
    if (head == nullptr)
      return new_node;

    // If alarm should come before head
    if (alarm->time < head->curr_a->time)
    {
      new_node->next_a = head;
      return new_node;
    }

    // Find insertion point
    alarm_list *curr = head;
    while (curr->next_a != nullptr && curr->next_a->curr_a->time <= alarm->time)
    {
      curr = curr->next_a;
    }

    // Insert node
    new_node->next_a = curr->next_a;
    curr->next_a = new_node;

    return head;
  }

  alarm_list *event_list_to_alarm_list(ble_schedule::event_list *e)
  {
    alarm_list *alarm_head = nullptr;

    // Iterate through all events in the event list
    ble_schedule::event_list *curr_event = e;
    while (curr_event != nullptr)
    {
      ble_schedule::event_t *event = curr_event->curr_e;

      // Generate alarms for this event from start_time to end_time, spaced by period
      // If period is 0, create just one alarm at start_time
      uint16_t current_time = event->start_time;

      if (event->period == 0)
      {
        // Single alarm at start_time
        alarm_t *alarm = (alarm_t *)malloc(sizeof(alarm_t));
        if (alarm == nullptr)
        {
          free_alarm_list(alarm_head);
          return nullptr;
        }

        alarm->time = event->start_time;
        alarm->e = event;

        alarm_head = _insert_alarm_sorted(alarm_head, alarm);
      }
      else
      {
        // Multiple alarms spaced by period
        while (current_time <= event->end_time)
        {
          alarm_t *alarm = (alarm_t *)malloc(sizeof(alarm_t));
          if (alarm == nullptr)
          {
            free_alarm_list(alarm_head);
            return nullptr;
          }

          alarm->time = current_time;
          alarm->e = event;

          alarm_head = _insert_alarm_sorted(alarm_head, alarm);

          current_time += event->period;
        }
      }

      // Move to next event
      curr_event = curr_event->next_e;
    }

    return alarm_head;
  }

  void free_alarm_list(alarm_list *list)
  {
    alarm_list *curr = list;
    while (curr != nullptr)
    {
      alarm_list *next = curr->next_a;
      // Free the alarm_t struct
      free(curr->curr_a);
      // Free the alarm_list node
      free(curr);
      curr = next;
    }
  }
}