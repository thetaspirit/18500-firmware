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

    // rfid.PCD_DumpVersionToSerial();

    // Ensure RFID is deselected after init
    digitalWrite(RFID_CS, HIGH);

    return true;
  }

  uint32_t get_uid()
  {
    byte bufferATQA[2];
    byte bufferSize = sizeof(bufferATQA);

    // Send WUPA command to detect any card (new or existing)
    if (rfid.PICC_WakeupA(bufferATQA, &bufferSize) != MFRC522::STATUS_OK)
    {
      return 0;
    }

    // Select the card to get its UID
    if (rfid.PICC_Select(&rfid.uid, 0) != MFRC522::STATUS_OK)
    {
      return 0;
    }

    // Combine all 4 bytes of UID into uint32_t
    uint32_t uid_value = 0;

    for (byte i = 0; i < rfid.uid.size && i < 4; i++)
    {
      uid_value |= (uint32_t)rfid.uid.uidByte[i] << (8 * (3 - i));
    }

    // Halt the card
    rfid.PICC_HaltA();

    return uid_value;
  }

  void print_uid()
  {
    byte bufferATQA[2];
    byte bufferSize = sizeof(bufferATQA);

    // Send WUPA command to detect any card (new or existing)
    if (rfid.PICC_WakeupA(bufferATQA, &bufferSize) != MFRC522::STATUS_OK)
    {
      return;
    }

    // Select the card to get its UID
    if (rfid.PICC_Select(&rfid.uid, 0) != MFRC522::STATUS_OK)
    {
      return;
    }

    // Print the UID
    Serial.print("UID: ");
    for (byte i = 0; i < rfid.uid.size; i++)
    {
      Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(rfid.uid.uidByte[i], HEX);
    }
    Serial.println();

    // Halt the card
    rfid.PICC_HaltA();
  }
}