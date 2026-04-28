
#include "alarms.h"
#include <cstring>

namespace alarms
{
  alarm_t *current_alarm; // is NULL unless there is supposed to be an alarm firing right now
  alarm_list *today;      // always points to the head of the alarm list

  void respond_to_alarm(alarm_t *a, Response r)
  {
    a->r = r;
    current_alarm = NULL;
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
        alarm->r = Response::NONE;

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
          alarm->r = Response::NONE;

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