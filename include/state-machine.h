#pragma once

namespace states
{
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
    BLUETOOTH,
    GNSS
  };

  enum class GNSSstate
  {
    TIME,
    TIMEZONE
  };

  enum class TimeState
  {
    SYNC,
    TIMEOUT,
    DONE
  };

  enum class TimezoneState
  {
    SYNC,
    TIMEOUT,
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

  extern MainState main_state;
  extern MenuState menu_state;
  extern GNSSstate gnss_state;
  extern TimeState time_state;
  extern TimezoneState timezone_state;
  extern NotifState notif_state;

}