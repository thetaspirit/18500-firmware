
#pragma once
#include "esp_timer.h"
#include "rfid-remis.h"
#include "system-utils.h"
#include "gnss-time.h"
#include "ble-schedule.h"

namespace alarms
{

  enum class Response
  {
    NONE,
    DONE,
    IGNORE,
    SNOOZE
  };

  typedef struct
  {
    uint16_t time; // time at which the alarm should go off, in minutes after midnight.  This is to account for recurring events.
    ble_schedule::event_t *e;
    Response r;
  } alarm_t;

  struct alarm_list
  {
    alarm_t *curr_a;
    alarm_list *next_a;
  };

  /**
   * Runs all the required setup involved in starting a new day.
   * Yes, this means alarms snoozed across midnight just get deleted.
   * Including the necessary allocates and frees.
   */
  void setup_today(void);

  /**
   * @return A pointer to the alarm struct for the upcoming (nearest) alarm.
   * This function also runs a check on the list of today's alarms.
   * If there is no "next alarm" for today, it will delete today's list, and
   * generate tomorrow's.  Then, it will return a pointer to the first of tomorrow's alarms.
   */
  alarm_t *get_upcoming_alarm(void);

  /**
   * @return The current alarm that should be firing right now.
   * Returns NULL if there is no alarm that should be going off right now.
   */
  alarm_t *get_current_alarm();

  /**
   * Sets a timer interrupt to go off at the time specified by the given alarm.
   * This should only be used by one alarm at a time!
   * This function will overwrite any previously-existing alarms produced by this function call.
   */
  void set_alarm_interrupt(alarm_t *a);

  /**
   * Call this function immediately when an alarm is supposed to go off.
   * It should be attached to an esp_timer interrupt.
   * If Remigotchi is asleep when the alarm goes off, call this function (manually) after wakeup.
   * Manages state changes.  Sets value returned by get_current_alarm.
   */
  void alarm_callback(void *arg);

  /**
   * Set the response field of an alarm to the given response.
   * Sets value returned by get_current_alarm back to NULL.
   *
   * Also sets up the interrupt for the next alarm to go off.
   * This is where a lot of the information change happens between alarms.
   */
  void respond_to_alarm(alarm_t *a, Response r);

  /**
   * Takes in a list of events, and creates a linked-list of alarms out of them.
   * The alarms are linked in chronological order.
   * Unlike the event list, this list also accounts for recurring events,
   * and includes each instance of the event as a separate alarm in the list.
   */
  alarm_list *event_list_to_alarm_list(ble_schedule::event_list *e);

  /**
   * Frees memory allocated by event_list_to_alarm_list.
   * This frees both the alarm_list and alarm_t structs.
   * @param list Pointer to the head of the event list to free.
   */
  void free_alarm_list(alarm_list *list);
}