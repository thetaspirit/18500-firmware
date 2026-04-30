#pragma once
#include "system-utils.h"
#include "ble-schedule.h"
#include "gnss-time.h"
#include "rfid-remis.h"
#include "alarms.h"
#include "debug.h"

namespace states
{
  // Current state count is at a total of 22
  enum class MainState
  {
    HOME,
    MENU,
    TODAY,
    NOTIF
  };

  enum class MenuState
  {
    SOUND,
    VIBRATION,
    BRIGHTNESS,
    BLUETOOTH_HIGHLIGHTED, // the bluetooth option on the menu is just highlighted, we are not in the bluetooth submenu
    BLUETOOTH,             // we are in the bluetooth submenu (not in the main menu!)
    GNSS_HIGHLIGHTED,      // the gps option on the menu is just highlighted, we are not in the gps submenu
    GNSS                   // we are in the gps submenu (not in the main menu!)
  };

  enum class BluetoothState
  {
    READY,     // ready to begin connection (but not actively in pairing mode at that point)
    WAITING,   // waiting for a connection
    RECEIVING, // actively trying to receive data
    DONE,      // just finished receiving data
    TIMEOUT    // timed out because either never received a connection or received bad data
  };

  enum class GNSSstate
  {
    TIME_HIGHLIGHTED,
    TIME,
    TIMEZONE_HIGHLIGHTED,
    TIMEZONE
  };

  enum class TimeState
  {
    // READY,
    SYNC,
    TIMEOUT,
    DONE
  };

  enum class TimezoneState
  {
    // READY,
    SYNC,
    // TIMEOUT,
    DONE
  };

  enum class NotifState
  {
    ALARM,
    DONE,
    SNOOZE,
    IGNORE,
    WRONG_RFID
  };

  void init(void);

  void background_task(void *parameters);

  /**
   * Call this function when it's time to show an alarm!
   */
  void set_alarm_state();

  MainState get_main(void);
  MenuState get_menu(void);
  BluetoothState get_bluetooth(void);
  GNSSstate get_gnss(void);
  TimeState get_time(void);
  TimezoneState get_timezone(void);
  NotifState get_notif(void);

}