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
}