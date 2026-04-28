#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "system-utils.h"

namespace rfid
{
  /**
   * Important: utils::shared_spi::init() must be called BEFORE this function!
   */
  bool init(void);

  /**
   * @return UID of the card detected.  0 = no card found, or there was some other problem.
   */
  uint32_t get_uid(void);

  void print_uid(void);
}