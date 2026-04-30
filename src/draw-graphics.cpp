#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h> 
#include "graphics-files.h"
#include <string>
#include "state-machine.h"

TFT_eSPI tft = TFT_eSPI(); 
TFT_eSprite spr = TFT_eSprite(&tft);
SPIClass sdSPI(HSPI);

//Frame sizes for Remi characters
#define FRAME_W 180
#define FRAME_H 240

#define SCREEN_W 240 
#define SCREEN_H 320

#define NOTIF_X 18
#define NOTIF_Y 68
#define NOTIF_W 196
#define NOTIF_H 324

#define MENU_COUNT 5

#define FPS 6

uint16_t frameDelay = 1000 / FPS;


volatile bool needsRedraw = true;
volatile bool animTick = false;
char currAlarm[32];

states::BluetoothState BLEstate = states::get_bluetooth();
states::MenuState menuState;
states::GNSSstate GNSSstate = states::get_gnss();
states::TimeState timeState;

bool toggleSound = utils::configs::get_sound();
bool toggleVibration = utils::configs::get_vibrate();
uint8_t brightness = utils::configs::get_brightness();

/* TODO: ADD CODE FOR REMI SELECTION
   TODO: RECONFIGURE MAIN DRAWTASK (MAY NO LONGER BE NEEDED)
   TODO: WRITE CODE FOR TODAY ALARM LIST
   TODO: ARE YOU SURE POPUP
   TODO: FINIALIZE CODE */

/*Remi Character Defs*/

enum Character {
  RACCOON,
  TOT,
  CAT
};



const char* characterFolders[] = {
  "Raccoon",
  "Tot",
  "Cat"
};

struct RemiCharacter {
  Character type;

  const char* animName;

  uint8_t frame;
  uint8_t maxFrames;

  uint32_t lastFrameTime;
};

RemiCharacter character;

//volatile Screen currScreen = HOME;

uint16_t frameDelay = 1000 / FPS;



void drawWrappedText(TFT_eSprite &s, String text,
                     int x, int y,
                     int maxWidth, int maxHeight) {

  s.setTextDatum(TL_DATUM);

  int cursorY = y;
  int lineHeight = s.fontHeight();

  while (text.length() > 0) {

    int spaceIndex = text.indexOf(' ');
    String word;

    // get next word
    if (spaceIndex == -1) {
      word = text;
      text = "";
    } else {
      word = text.substring(0, spaceIndex);
      text = text.substring(spaceIndex + 1);
    }

    static String line = "";

    String testLine;
    if (line.length() == 0)
      testLine = word;
    else
      testLine = line + " " + word;

    // check if adding word exceeds width
    if (s.textWidth(testLine) > maxWidth) {

      // draw current line
      if (cursorY + lineHeight > y + maxHeight)
        break;

      s.drawString(line, x, cursorY);
      cursorY += lineHeight + 4;

      // start new line with current word
      line = word;
    }
    else {
      line = testLine;
    }

    if (text.length() == 0) {

      if (cursorY + lineHeight <= y + maxHeight) {
        s.drawString(line, x, cursorY);
      }

      line = "";
    }
  }
}

void drawReminder(){
  spr.fillSprite(TFT_WHITE);
  spr.pushImage(0, 0, 240, 320, notifScreen);
  spr.setViewport(NOTIF_X, NOTIF_Y, NOTIF_W, NOTIF_H);

  spr.setTextColor(TFT_BLACK);
  spr.setFreeFont(&ari_w9500_bold10pt7b);


  drawWrappedText(
    spr,
    currAlarm,
    0, 0,
    NOTIF_W,
    NOTIF_H
  );


  spr.resetViewport();

  spr.pushSprite(0, 0);
}

void drawMenu()
{
  spr.fillSprite(TFT_BLACK);

  switch (menuState) {

    case states::MenuState::SOUND:
      if (toggleSound)
        spr.pushImage(0, 0, spr.width(), spr.height(), soundOn);
      else
        spr.pushImage(0, 0, spr.width(), spr.height(), soundOff);
      break;


    case states::MenuState::VIBRATION:
      if (toggleVibration)
        spr.pushImage(0, 0, spr.width(), spr.height(), vibrationOn);
      else
        spr.pushImage(0, 0, spr.width(), spr.height(), vibrationOff);
      break;

    case states::MenuState::BLUETOOTH:
      drawBLE();
      break;

    case states::MenuState::BLUETOOTH_HIGHLIGHTED:
      spr.pushImage(0, 0, spr.width(), spr.height(), bluetooth);
      break;

    case states::MenuState::GNSS:
      drawGNSSMain();
      break;

    case states::MenuState::GNSS_HIGHLIGHTED:
      spr.pushImage(0, 0, spr.width(), spr.height(), gpsSel);
      break;

    case states::MenuState::BRIGHTNESS:
      if (brightness == 2)
        spr.pushImage(0, 0, spr.width(), spr.height(), brightness_high);
      else
        spr.pushImage(0, 0, spr.width(), spr.height(), brightness_low);
      break;
  }

  spr.pushSprite(0, 0);

  
}

void setCharacterAnim(RemiCharacter &c, const char* anim, uint8_t frames) {

  c.animName = anim;
  c.maxFrames = frames;
  c.frame = 0;
  c.lastFrameTime = millis();

  needsRedraw = true;
}

void updateAnimation(RemiCharacter &c) {

  if (millis() - c.lastFrameTime >= frameDelay) {

    c.lastFrameTime = millis();
    c.frame++;

    if (c.frame >= c.maxFrames) c.frame = 0;

    animTick = true;
  }
}

void drawRLEFrame(const char* path, int x, int y) {

  File f = SD.open(path);
  if (!f) return;

  uint16_t buffer[FRAME_W];

  uint16_t px = 0;
  uint16_t py = 0;

  while (f.available() >= 4 && py < FRAME_H) {

    uint16_t color = f.read() | (f.read() << 8);
    uint16_t run   = f.read() | (f.read() << 8);

    while (run--) {

      buffer[px++] = color;

      if (px >= FRAME_W) {

        spr.pushImage(x, y + py, FRAME_W, 1, buffer);

        px = 0;
        py++;

        yield(); 
      }
    }
  }

  f.close();
}

void drawHome() {
  

  spr.fillSprite(TFT_WHITE);

  char path[128];


  snprintf(path, sizeof(path),
           "/anim/%s/%s/%02d.rle",
           characterFolders[character.type],
           character.animName,
           character.frame);

  Serial.println(path);
  if (!SD.exists(path)) {
    Serial.println("FRAME MISSING");
  }

  drawRLEFrame(path, 30, 40);

  spr.pushImage(10, 10, 40, 40, battNeedCharge);
  spr.pushImage(10, 270, 112, 32, healthBar[6]);

  spr.pushSprite(0, 0);
}




void drawGNSSMain(){
  spr.fillSprite(TFT_WHITE);

  switch(GNSSstate) {
    case states::GNSSstate::TIME :
      spr.pushImage(0, 0, spr.width(), spr.height(), GNSSTimeSync);
      break;
    
    case states::GNSSstate::TIME_HIGHLIGHTED :
      spr.pushImage(0, 0, spr.width(), spr.height(), TimeSyncHighlight);
      break;
    
    case states::GNSSstate::TIMEZONE :
      spr.pushImage(0, 0, spr.width(), spr.height(), GNSSTimeZone);
      drawTimeZone();
      break;

    case states::GNSSstate::TIMEZONE_HIGHLIGHTED:
      spr.pushImage(0, 0, spr.width(), spr.height(), TimeZoneHighlight);
      break;
  }

  spr.pushSprite(0, 0);


}

//redraw should probably be asserted
void drawTimeZone() {
  spr.fillSprite(TFT_WHITE);
  spr.pushImage(0, 0, spr.width(), spr.height(), GNSSTimeZone);

  spr.setTextColor(TFT_BLACK);
  spr.setFreeFont(&ari_w9500_bold10pt7b);

  spr.drawString("UTC", 120, 72);
  //Write code for incrementing and decrementing UTC

  spr.pushSprite(0,0);

}

void drawTimeSync() {
  switch(timeState){
    case states::TimeState::SYNC:
      spr.pushImage(0, 0, spr.width(), spr.height(), GNSSTimeSync);
      break;
    
    case states::TimeState::TIMEOUT:
      spr.pushImage(0, 0, spr.width(), spr.height(), GNSSTimeSyncFail);
      break;

    case states::TimeState::DONE:
      needsRedraw = true;
      drawMenu(); 
  }
}



void drawBLE(){

  switch(BLEstate) {
    case states::BluetoothState::WAITING:
      spr.pushImage(0, 0, spr.width(), spr.height(), BLEWaiting);
      break;
    
    case states::BluetoothState::RECEIVING:
      spr.pushImage(0, 0, spr.width(), spr.height(), BLESync);
      break;
    
    case states::BluetoothState::TIMEOUT:
      spr.pushImage(0, 0, spr.width(), spr.height(), BLESyncFail);
      break;

    case states::BluetoothState::DONE:
      needsRedraw = true;
      drawMenu();
      break;
  }


}

void drawTask(void *param) {

  for (;;) {

    if (currScreen == HOME) {

      if (reminderAlert) {

        if (needsRedraw) {
          drawReminder();
          needsRedraw = false;
        }
      }

      else{
        updateAnimation(character);

        if (animTick) {
          drawHome();
          animTick = false;
        }

      }

      
    }

    else if (currScreen == MENU) {

      if (needsRedraw) {
        drawMenu();
        needsRedraw = false;
      }
    }

    vTaskDelay(1);
  }
}