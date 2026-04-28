
#pragma once
#include "ble-schedule.h"
#include "esp_timer.h"
#include "rfid-remis.h"
#include "gnss-time.h"
#include "system-utils.h"

#define ALARM_LOG_FILEPATH "/alarm-log.csv"

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
  } alarm_t;

  struct alarm_list
  {
    alarm_t *curr_a;
    alarm_list *next_a;
  };

  /**
   * Only need to call this once on startup
   */
  void init(void);

  /**
   * Call this function on startup, or when the day changes.
   * Runs all the required setup involved in starting a new day.
   * Yes, this means alarms snoozed across midnight just get deleted.
   * Including the necessary allocates and frees.
   * @param day Day of the week [1, 7]
   */
  void setup_day(uint8_t day);

  /**
   * @return A pointer to the alarm struct for the upcoming (nearest) alarm.
   * This function also runs a check on the list of today's alarms.
   * If there is no "next alarm" for today, it will delete today's list, and
   * generate tomorrow's.  Then, it will return a pointer to the first of tomorrow's alarms.
   */
  alarm_t *find_next_alarm(void);

  /**
   * @return A pointer to the alarm in the internal field upcoming_alarm.
   * Does not do any searches.
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
   * Clears all alarm interrupts.
   * To prevent an alarm going off at the wrong time (late),
   * this should be called on startup/wake because the timer stops
   * while device is asleep.
   */
  void clear_alarm_interrupts();

  /**
   * Call this function immediately when an alarm is supposed to go off.
   * It should be attached to an esp_timer interrupt.
   * If Remigotchi is asleep when the alarm goes off, call this function (manually) after wakeup.
   * Manages state changes.  Sets value returned by get_current_alarm.
   */
  void alarm_callback(void *arg);

  /**
   * Only call this function when there is an alarm actively going off.
   * Sets the response field of the current alarm.
   * Sets value returned by get_current_alarm back to NULL.
   *
   * Also sets up the interrupt for the next alarm to go off.
   * This is where a lot of the information change happens between alarms.
   */
  void respond_to_alarm(Response r);

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
   * Note, this does NOT free any of the event_t associated with the alarms.  That free needs to be made separately.
   * @param list Pointer to the head of the event list to free.
   */
  void free_alarm_list(alarm_list *list);
}