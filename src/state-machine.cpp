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

  unsigned long BLE_CONNECT_TIMEOUT = 30000; // milliseconds

  /**
   * These variables are true when a button has been CLICKED, and it needs to be
   * addressed.  Set these variables back to false after they've been addressed.
   */
  bool button_1;
  bool button_2;
  bool button_3;
  bool button_4;

  bool button1_ignore_falling;
  bool button2_ignore_falling;
  bool button3_ignore_falling;
  bool button4_ignore_falling;
  // Ignore the falling edge of the button when we call the long-click interrupt

  esp_timer_handle_t button1_timer;
  esp_timer_handle_t button2_timer;
  esp_timer_handle_t button3_timer;
  esp_timer_handle_t button4_timer;
  unsigned long LONG_CLICK_MS = 2000;
  void _reset_states(void);

  void clear_buttons()
  {
    button_1 = false;
    button_2 = false;
    button_3 = false;
    button_4 = false;
  }

  void button1_callback(void *args)
  {
    button1_ignore_falling = true;
    _reset_states();
  }
  void button2_callback(void *args) {}
  void button3_callback(void *args) {}
  void button4_callback(void *args) {}

  void IRAM_ATTR button1_isr()
  {
    if (gpio_get_level((gpio_num_t)BUTTON_1))
    {
      esp_timer_start_once(button1_timer, LONG_CLICK_MS * 1000);
    }
    else
    {
      if (!button1_ignore_falling)
      {
        esp_timer_stop(button1_timer);
        button_1 = true;
      }
      else
      {
        button1_ignore_falling = false;
      }
    }
  }
  void IRAM_ATTR button2_isr()
  {
    if (gpio_get_level((gpio_num_t)BUTTON_2))
    {
      esp_timer_start_once(button2_timer, LONG_CLICK_MS * 1000);
    }
    else
    {
      if (!button2_ignore_falling)
      {
        esp_timer_stop(button2_timer);
        button_2 = true;
      }
      else
      {
        button2_ignore_falling = false;
      }
    }
  }
  void IRAM_ATTR button3_isr()
  {
    if (gpio_get_level((gpio_num_t)BUTTON_3))
    {
      esp_timer_start_once(button3_timer, LONG_CLICK_MS * 1000);
    }
    else
    {
      if (!button3_ignore_falling)
      {
        esp_timer_stop(button3_timer);
        button_3 = true;
      }
      else
      {
        button3_ignore_falling = false;
      }
    }
  }
  void IRAM_ATTR button4_isr()
  {
    if (gpio_get_level((gpio_num_t)BUTTON_4))
    {
      esp_timer_start_once(button4_timer, LONG_CLICK_MS * 1000);
    }
    else
    {
      if (!button4_ignore_falling)
      {
        esp_timer_stop(button4_timer);
        button_4 = true;
      }
      else
      {
        button4_ignore_falling = false;
      }
    }
  }

  void _handle_time()
  {
  }

  void _handle_timezone() {}

  /**
   * Handles all the stuff for when we are in the gps SUBMENU
   */
  void _handle_gnss()
  {
    switch (gnss_state)
    {
    case GNSSstate::TIME_HIGHLIGHTED:
      if (button_1 || button_3)
      {
        clear_buttons();
        gnss_state = GNSSstate::TIMEZONE_HIGHLIGHTED;
        Serial.println("Showing gps submenu -> timezone.");
      }
      else if (button_2)
      {
        clear_buttons();
        gnss_state = GNSSstate::TIME;
        Serial.println("Entering gps->time sub-submenu.");
      }
      break;
    case GNSSstate::TIMEZONE_HIGHLIGHTED:
      if (button_1 || button_3)
      {
        clear_buttons();
        gnss_state = GNSSstate::TIME_HIGHLIGHTED;
        Serial.println("Showing gps submenu -> time.");
      }
      else if (button_2)
      {
        clear_buttons();
        gnss_state = GNSSstate::TIMEZONE;
        Serial.println("Entering gps->timezone sub-submenu.");
      }
      break;
    case GNSSstate::TIME:
      _handle_time();
      break;
    case GNSSstate::TIMEZONE:
      _handle_timezone();
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
        clear_buttons();
        // set state back to READY before leaving!
        ble_state = BluetoothState::READY;
        menu_state = MenuState::BLUETOOTH_HIGHLIGHTED;
      }
      break;
    case BluetoothState::TIMEOUT:
      if (button_1)
      { // go back to previous menu
        clear_buttons();
        // set state back to READY before leaving!
        ble_state = BluetoothState::READY;
        Serial.println("Showing menu -> bluetooth");
        menu_state = MenuState::BLUETOOTH_HIGHLIGHTED;
      }
      else if (button_3)
      { // user wants to try again
        clear_buttons();
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
        clear_buttons();
        utils::configs::toggle_sound();
      }
      else if (button_1)
      {
        clear_buttons();
        menu_state = MenuState::BRIGHTNESS;
        Serial.println("Showing menu -> brightness.");
      }
      else if (button_3)
      {
        clear_buttons();
        menu_state = MenuState::VIBRATION;
        Serial.println("Showing menu -> vibrate.");
      }
      break;
    case MenuState::VIBRATION:
      if (button_2)
      {
        clear_buttons();
        utils::configs::toggle_vibrate();
      }
      else if (button_1)
      {
        clear_buttons();
        menu_state = MenuState::SOUND;
        Serial.println("Showing menu -> sound.");
      }
      else if (button_3)
      {
        clear_buttons();
        menu_state = MenuState::BLUETOOTH_HIGHLIGHTED;
        Serial.println("Showing menu -> bluetooth.");
      }
      break;
    case MenuState::BRIGHTNESS:
      if (button_2)
      {
        clear_buttons();
        utils::configs::toggle_brightness();
      }
      else if (button_1)
      {
        clear_buttons();
        menu_state = MenuState::GNSS_HIGHLIGHTED;
        Serial.println("Showing menu -> GPS.");
      }
      else if (button_3)
      {
        clear_buttons();
        menu_state = MenuState::SOUND;
        Serial.println("Showing menu -> sound.");
      }
      break;
    case MenuState::BLUETOOTH_HIGHLIGHTED:
      if (button_2)
      {
        clear_buttons();
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
        clear_buttons();
        menu_state = MenuState::VIBRATION;
        Serial.println("Showing menu -> vibrate.");
      }
      else if (button_3)
      {
        clear_buttons();
        menu_state = MenuState::GNSS_HIGHLIGHTED;
        Serial.println("Showing menu -> GPS.");
      }
      break;
    case MenuState::BLUETOOTH:
      _handle_bluetooth();
      break;
    case MenuState::GNSS_HIGHLIGHTED:
      if (button_2)
      {
        clear_buttons();
        // Enter the gps submenu
        menu_state = MenuState::GNSS;
        gnss_state = GNSSstate::TIME_HIGHLIGHTED;
        Serial.println("Entering gps submenu.");
      }
      else if (button_1)
      {
        clear_buttons();
        menu_state = MenuState::BLUETOOTH_HIGHLIGHTED;
        Serial.println("Showing menu -> bluetooth.");
      }
      else if (button_3)
      {
        clear_buttons();
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

  void _reset_states()
  {
    // Default states
    main_state = MainState::HOME;
    menu_state = MenuState::SOUND;
    ble_state = BluetoothState::READY;
    gnss_state = GNSSstate::TIME;
    time_state = TimeState::SYNC;
    timezone_state = TimezoneState::DONE;
    notif_state = NotifState::ALARM;
  }

  void init()
  {
    _reset_states();

    // Button long-click timeres
    esp_timer_create_args_t args1 = {
        .callback = &button1_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "button1_timer"};
    esp_timer_create_args_t args2 = {
        .callback = &button2_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "button2_timer"};
    esp_timer_create_args_t args3 = {
        .callback = &button3_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "button3_timer"};
    esp_timer_create_args_t args4 = {
        .callback = &button4_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "button4_timer"};

    esp_timer_create(&args1, &button1_timer);
    esp_timer_create(&args2, &button2_timer);
    esp_timer_create(&args3, &button3_timer);
    esp_timer_create(&args4, &button4_timer);

    // Button interrupts
    attachInterrupt(digitalPinToInterrupt(BUTTON_1), button1_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BUTTON_2), button2_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BUTTON_3), button3_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BUTTON_4), button4_isr, CHANGE);
  }

  void background_task(void *parameters)
  {
    while (true)
    {
      digitalWrite(RED, LOW);
      digitalWrite(YELLOW, LOW);
      digitalWrite(GREEN, LOW);
      switch (main_state)
      {
      case MainState::HOME:
        digitalWrite(YELLOW, HIGH);
        if (button_1)
        {
          clear_buttons();
          Serial.println("Entering the menu.");
          main_state = MainState::MENU;
        }
        else if (button_2)
        {
          clear_buttons();
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
          clear_buttons();
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