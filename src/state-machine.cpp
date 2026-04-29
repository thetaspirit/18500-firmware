#include "state-machine.h"

namespace states
{
  MainState main_state;
  MenuState menu_state;
  BluetoothState ble_state;
  GNSSstate gnss_state;
  TimeState time_state;
  TimezoneState timezone_state;
  NotifState notif_state;

  /**
   * These variables are true when a button has been CLICKED, and it needs to be
   * addressed.  Set these variables back to false after they've been addressed.
   */
  bool button_1;
  bool button_2;
  bool button_3;
  bool button_4;

  void clear_buttons()
  {
    button_1 = false;
    button_2 = false;
    button_3 = false;
    button_4 = false;
  }

  void IRAM_ATTR _falling1()
  {
    button_1 = true;
  }
  void IRAM_ATTR _falling2()
  {
    button_2 = true;
  }
  void IRAM_ATTR _falling3()
  {
    button_3 = true;
  }
  void IRAM_ATTR _falling4()
  {
    button_4 = true;
  }

  unsigned long BLE_CONNECT_TIMEOUT = 30000; // milliseconds

  void _handle_time() {}

  void _handle_timezone() {}

  /**
   * Handles all the stuff for when we are in the gps SUBMENU
   */
  void _handle_gnss()
  {
    switch (gnss_state)
    {
    case GNSSstate::TIME_HIGHLIGHTED:
      break;
    case GNSSstate::TIMEZONE_HIGHLIGHTED:
      break;
    case GNSSstate::TIME:
      break;
    case GNSSstate::TIMEZONE:
      break;
    default:
      break;
    }
  }

  uint8_t ble_receive_status = 0xFF;
  /**
   * Handles all the stuff for when we are in the bluetooth SUBMENU
   */
  void _handle_bluetooth()
  {
    switch (ble_state)
    {
    case BluetoothState::WAITING:
      // the only way the code reaches this point is AFTER
      // block_until_connected returns from when it was called
      // in _handle_menu.  This code specifically addresses what happens
      // when we are *done* waiting.
      if (!ble_schedule::ble_is_connected())
      {
        ble_state = BluetoothState::TIMEOUT;
        ble_schedule::stop();
        Serial.println("Bluetooth timed out.");
      }
      else
      {
        ble_state = BluetoothState::RECEIVING;
        ble_receive_status = ble_schedule::receive_schedule_data();
        Serial.println("Function receive_schedule_data ended.");
      }
      break;
    case BluetoothState::RECEIVING:
      // the only way the code reaches this point is AFTER receive_schedule_data returns
      // from when it was called above.  This code specifically addresses what happens
      // after that function returns.
      if (ble_receive_status == 0)
      {
        Serial.println("Schedule received successfully.");
        ble_schedule::save_schedule_to_sd(); // TODO add safety checks to this ig
        ble_state = BluetoothState::DONE;
      }
      else
      {
        Serial.println("There was a problem receiving the schedule.");
        ble_state = BluetoothState::TIMEOUT;
      }
      ble_schedule::stop();
      break;
    case BluetoothState::DONE:
      if (button_1)
      { // go back to previous menu
        button_1 = false;
        // set state back to READY before leaving!
        ble_state = BluetoothState::READY;
        menu_state = MenuState::BLUETOOTH_HIGHLIGHTED;
      }
      break;
    case BluetoothState::TIMEOUT:
      if (button_1)
      { // go back to previous menu
        button_1 = false;
        // set state back to READY before leaving!
        ble_state = BluetoothState::READY;
        Serial.println("Showing menu -> bluetooth");
        menu_state = MenuState::BLUETOOTH_HIGHLIGHTED;
      }
      else if (button_3)
      { // user wants to try again
        button_3 = false;
        ble_schedule::start();
        ble_state = BluetoothState::WAITING;
        ble_schedule::block_until_connected(BLE_CONNECT_TIMEOUT);
      }
      break;
    default:
      break;
    }
  }

  /**
   * Funtion that cases on all of the MenuStates
   */
  void _handle_menu()
  {
    switch (menu_state)
    {
    case MenuState::SOUND:
      if (button_2)
      {
        button_2 = false;
        utils::configs::toggle_sound();
      }
      else if (button_1)
      {
        button_1 = false;
        menu_state = MenuState::BRIGHTNESS;
        Serial.println("Showing menu -> brightness.");
      }
      else if (button_3)
      {
        button_3 = false;
        menu_state = MenuState::VIBRATION;
        Serial.println("Showing menu -> vibrate.");
      }
      break;
    case MenuState::VIBRATION:
      if (button_2)
      {
        button_2 = false;
        utils::configs::toggle_vibrate();
      }
      else if (button_1)
      {
        button_1 = false;
        menu_state = MenuState::SOUND;
        Serial.println("Showing menu -> sound.");
      }
      else if (button_3)
      {
        button_3 = false;
        menu_state = MenuState::BLUETOOTH_HIGHLIGHTED;
        Serial.println("Showing menu -> bluetooth.");
      }
      break;
    case MenuState::BRIGHTNESS:
      if (button_2)
      {
        button_2 = false;
        utils::configs::toggle_brightness();
      }
      else if (button_1)
      {
        button_1 = false;
        menu_state = MenuState::GNSS;
        Serial.println("Showing menu -> GPS.");
      }
      else if (button_3)
      {
        button_3 = false;
        menu_state = MenuState::SOUND;
        Serial.println("Showing menu -> sound.");
      }
      break;
    case MenuState::BLUETOOTH_HIGHLIGHTED:
      if (button_2)
      {
        button_2 = false;
        if (ble_state == BluetoothState::READY)
        {
          menu_state = MenuState::BLUETOOTH;
          Serial.println("Entering bluetooth submenu.");
          ble_schedule::start();
          ble_state = BluetoothState::WAITING;
          ble_schedule::block_until_connected(BLE_CONNECT_TIMEOUT);
        }
      }
      else if (button_1)
      {
        button_1 = false;
        menu_state = MenuState::VIBRATION;
        Serial.println("Showing menu -> vibrate.");
      }
      else if (button_3)
      {
        button_3 = false;
        menu_state = MenuState::GNSS;
        Serial.println("Showing menu -> GPS.");
      }
      break;
    case MenuState::BLUETOOTH:
      _handle_bluetooth();
      break;
    case MenuState::GNSS_HIGHLIGHTED:
      if (button_2)
      {
        button_2 = false;
        // Enter the gps submenu
        menu_state = MenuState::GNSS;
        gnss_state = GNSSstate::TIME_HIGHLIGHTED;
        Serial.println("Entering gps submenu.");
      }
      else if (button_1)
      {
        button_1 = false;
        menu_state = MenuState::BLUETOOTH_HIGHLIGHTED;
        Serial.println("Showing menu -> bluetooth.");
      }
      else if (button_3)
      {
        button_3 = false;
        menu_state = MenuState::BRIGHTNESS;
        Serial.println("Showing menu -> brightness.");
      }
      break;
    case MenuState::GNSS:
      _handle_gnss();
      break;
    default:
      break;
    }
  }

  void _handle_notif() {}

  void init()
  {
    // Default states
    main_state = MainState::HOME;
    menu_state = MenuState::SOUND;
    ble_state = BluetoothState::READY;
    gnss_state = GNSSstate::TIME;
    time_state = TimeState::READY;
    timezone_state = TimezoneState::DONE;
    notif_state = NotifState::ALARM;

    // Button interrupts
    attachInterrupt(digitalPinToInterrupt(BUTTON_1), _falling1, FALLING);
    attachInterrupt(digitalPinToInterrupt(BUTTON_2), _falling2, FALLING);
    attachInterrupt(digitalPinToInterrupt(BUTTON_3), _falling3, FALLING);
    attachInterrupt(digitalPinToInterrupt(BUTTON_4), _falling4, FALLING);
  }

  void background_task(void *parameters)
  {
    while (true)
    {
      switch (main_state)
      {
      case MainState::HOME:
        if (button_1)
        {
          button_1 = false;
          Serial.println("Entering the menu.");
          main_state = MainState::MENU;
        }
        else if (button_2)
        {
          button_2 = false;
          Serial.println("Entering the today.");
          main_state = MainState::TODAY;
        }
        break;
      case MainState::MENU:
        _handle_menu();
        break;
      case MainState::TODAY:
        if (button_1)
        {
          button_1 = false;
          Serial.println("Going back home.");
          main_state = MainState::HOME;
        }
        break;
      case MainState::NOTIF:
        _handle_notif();
        break;
      default:
        break;
      }
      vTaskDelay(pdMS_TO_TICKS(50));
    }
  }

  MainState get_main()
  {
    return main_state;
  }

  MenuState get_menu()
  {
    return menu_state;
  }

  BluetoothState get_bluetooth()
  {
    return ble_state;
  }

  GNSSstate get_gnss()
  {
    return gnss_state;
  }

  TimeState get_time()
  {
    return time_state;
  }

  TimezoneState get_timezone()
  {
    return timezone_state;
  }

  NotifState get_notif()
  {
    return notif_state;
  }
}