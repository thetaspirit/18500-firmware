#include "rfid-remis.h"
#include "system-utils.h"

namespace rfid
{
  MFRC522 rfid(RFID_CS, RFID_RST, utils::shared_spi::shared_SPI);

  bool init()
  {
    pinMode(RFID_CS, OUTPUT);
    digitalWrite(RFID_CS, HIGH); // CS idle HIGH before init
    rfid.PCD_Init();

    rfid.PCD_DumpVersionToSerial();

    // Ensure RFID is deselected after init
    digitalWrite(RFID_CS, HIGH);

    return true;
  }
}