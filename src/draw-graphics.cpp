#include "draw-graphics.h"
#include "graphics-files.h"

#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <string>
#include "state-machine.h"

// ================= TFT =================
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
// ================= SCREEN =================
#define SCREEN_W 240
#define SCREEN_H 320

#define FRAME_W 180
#define FRAME_H 240


// ================= MENU / UI =================
#define FPS 6
uint16_t frameDelay = 1000 / FPS;

volatile bool renderingFrame = false;
portMUX_TYPE renderMux = portMUX_INITIALIZER_UNLOCKED;

//volatile bool needsRedraw = true;
volatile bool animTick = false;
//char currAlarm[32];

// ================= STATES =================
states::BluetoothState BLEstate = states::get_bluetooth();
states::MenuState menuState = states::get_menu();
states::GNSSstate GNSSstate = states::get_gnss();
states::TimeState timeState = states::get_time();
states::MainState mainState = states::get_main();
states::NotifState alarmState = states::get_notif();
states::MainState prevMainState;
states::MenuState prevMenuState;

bool toggleSound = utils::configs::get_sound();
bool toggleVibration = utils::configs::get_vibrate();
uint8_t brightness = utils::configs::get_brightness();

enum Character { RACCOON, TOT, CAT };

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

struct AnimInfo {
  const char* name;
  uint8_t frames;
};

char animationBase[16] = "idle";  
uint8_t frameIndex = 0;
uint8_t totalFrames = 10;

AnimInfo raccoonAnims[] = {
  {"idleRaccoon", 5},
  {"sadRaccoon", 17}
};

AnimInfo catAnims[] = {
  {"idleCat", 10},
  {"sadCat", 6}
};

AnimInfo totAnims[] = {
  {"idleTot", 10},
  {"sadTot", 11}
};

enum RenderMode {
  RENDER_HOME,
  RENDER_MENU,
  RENDER_GNSS,
  RENDER_BLUETOOTH,
  RENDER_TODAY,
  RENDER_NOTIF
};



// ================= MENU PATH =================
const char* getMenuScreenPath() {
  switch (menuState) {

    case states::MenuState::SOUND:
      return toggleSound ? "/Screens/soundOn.rle" : "/Screens/soundOff.rle";

    case states::MenuState::VIBRATION:
      return toggleVibration ? "/Screens/vibrationOn.rle" : "/Screens/vibrationOff.rle";

    case states::MenuState::BRIGHTNESS:
      return (brightness == 2) ? "/Screens/BrightnessHigh.rle"
                               : "/Screens/BrightnessLow.rle";

    case states::MenuState::BLUETOOTH:
      return "/Screens/BLEwaiting.rle";

    case states::MenuState::BLUETOOTH_HIGHLIGHTED:
      return "/Screens/BLEsel.rle";

    case states::MenuState::GNSS:
      return "/Screens/TimeSyncHiglight.rle";

    case states::MenuState::GNSS_HIGHLIGHTED:
      return "/Screens/GNSSselect.rle";

    default:
      return nullptr;
  }
}

void updateAnimation(RemiCharacter &c) {

  if (millis() - c.lastFrameTime >= frameDelay) {

    c.lastFrameTime = millis();

    c.frame++;

    if (c.frame >= c.maxFrames) {
      c.frame = 0;
    }

    animTick = true;
  }
}

void drawRLEFrame(const char* path, int x, int y, int w, int h)
{
  File f = SD.open(path);
  if (!f) return;

  static uint16_t buffer[SCREEN_W];

  int px = 0;
  int py = 0;

  memset(buffer, 0, sizeof(buffer));

  while (f.available() >= 4 && py < h)
  {
    uint16_t color = f.read() | (f.read() << 8);
    uint16_t run   = f.read() | (f.read() << 8);

    if (run == 0 || run > 5000)
      break;

    while (run > 0)
    {
      buffer[px++] = color;
      run--;

      // ✅ HARD ROW BOUNDARY
      if (px == w)
      {
        spr.pushImage(x, y + py, w, 1, buffer);

        px = 0;
        py++;

        // clear row completely (important for no ghosting)
        memset(buffer, 0, sizeof(buffer));

        if (py >= h)
        {
          f.close();
          return;
        }
      }
    }
  }

  f.close();
}

const char* drawGNSSMain(){
  //spr.fillSprite(TFT_WHITE);

  switch(GNSSstate) {
    case states::GNSSstate::TIME :
      return "/Screens/GNSSTimeSync.rle";
    
    case states::GNSSstate::TIME_HIGHLIGHTED :
      return "/Screens/TimeSyncHighlight.rle";
    
    case states::GNSSstate::TIMEZONE :
      return "/Screens/GNSSTimeZone.rle";
      //drawTimeZone();

    case states::GNSSstate::TIMEZONE_HIGHLIGHTED:
      return "/Screens/TimeZoneHighlight.rle";
    
    default:
      return nullptr;
  }

}

RenderMode getRenderMode()
{
  switch (mainState)
  {
    case states::MainState::HOME:
      return RENDER_HOME;

    case states::MainState::MENU:
    {
      if (menuState == states::MenuState::GNSS ||
          menuState == states::MenuState::GNSS_HIGHLIGHTED)
        return RENDER_GNSS;

      if (menuState == states::MenuState::BLUETOOTH ||
          menuState == states::MenuState::BLUETOOTH_HIGHLIGHTED)
        return RENDER_BLUETOOTH;

      return RENDER_MENU;
    }

    case states::MainState::TODAY:
      return RENDER_TODAY;

    case states::MainState::NOTIF:
      return RENDER_NOTIF;

    default:
      return RENDER_MENU;
  }
}


void renderScreen()
{

  spr.fillSprite(TFT_WHITE);

  RenderMode mode = getRenderMode();

  switch (mode)
  {
    case RENDER_HOME:
    {

      updateAnimation(character);

      char path[128];
      snprintf(path, sizeof(path),
               "/anim/%s/%s/%02d.rle",
               characterFolders[character.type],
               character.animName,
               character.frame);

      if (SD.exists(path)) {
        drawRLEFrame(path, 30, 40, 180, 240);
      }
      break;
    }

    case RENDER_MENU:
    {
      const char* path = getMenuScreenPath();
      if (path) drawRLEFrame(path, 0, 0, 240, 320);
      break;
    }

    case RENDER_GNSS:
    {
      const char* path = drawGNSSMain();  // your function
      if (path && SD.exists(path))
        drawRLEFrame(path, 0, 0, 240, 320);
      break;
    }

    case RENDER_BLUETOOTH:
    {
      const char* path = getMenuScreenPath(); // you define this
      if (path)
        drawRLEFrame(path, 0, 0, 240, 320);
      break;
    }

    case RENDER_TODAY:
    {
      const char* bg = "/Screens/remindersToday.rle";
      if (bg) drawRLEFrame(bg, 0, 0, 240, 320);

      // overlay text (example)
      // drawWrappedText(...)
      break;
    }

    case RENDER_NOTIF:
    {
      const char* bg = "/Screens/notifScreen.rle";
      if (bg) drawRLEFrame(bg, 0, 0, 240, 320);

      // overlay alarm text
      break;
    }
  }

  spr.pushSprite(0, 0);

}





const char* getScreenPath() {

  if (mainState == states::MainState::HOME)
    return nullptr;

  if (mainState == states::MainState::MENU)
    return getMenuScreenPath();

  if (mainState == states::MainState::NOTIF)
    return "/Screens/notifScreen.rle";

  if (mainState == states::MainState::TODAY)
    return "/Screens/remindersToday.rle";

  return nullptr;
}

// ================= TEXT WRAP =================
void drawWrappedText(TFT_eSprite &s, String text,
                     int x, int y,
                     int maxWidth, int maxHeight) {

  s.setTextDatum(TL_DATUM);

  int cursorY = y;
  int lineHeight = s.fontHeight();

  static String line = "";

  while (text.length() > 0) {

    int spaceIndex = text.indexOf(' ');
    String word;

    if (spaceIndex == -1) {
      word = text;
      text = "";
    } else {
      word = text.substring(0, spaceIndex);
      text = text.substring(spaceIndex + 1);
    }

    String testLine = (line.length() == 0) ? word : (line + " " + word);

    if (s.textWidth(testLine) > maxWidth) {

      if (cursorY + lineHeight > y + maxHeight)
        break;

      s.drawString(line, x, cursorY);
      cursorY += lineHeight + 4;
      line = word;

    } else {
      line = testLine;
    }

    if (text.length() == 0) {
      s.drawString(line, x, cursorY);
      line = "";
    }
  }
}


// ================= ANIMATION =================
void setCharacterAnim(RemiCharacter &c, const char* anim, uint8_t frames) {
  c.animName = anim;
  c.maxFrames = frames;
  c.frame = 0;
  c.lastFrameTime = millis();
}



// ================= RLE RENDER =================


void drawHome() {

  //spr.fillSprite(TFT_WHITE);

  char path[128];

  snprintf(path, sizeof(path),
           "/anim/%s/%s/%02d.rle",
           characterFolders[character.type],
           character.animName,
           character.frame);

  if (SD.exists(path)) {
    drawRLEFrame(path, 30, 40, 180, 240);
  }
}


void drawMenu() {

  //spr.fillSprite(TFT_BLACK);

  const char* path = getMenuScreenPath();

  if (path && SD.exists(path)) {
    drawRLEFrame(path, 0, 0, SCREEN_W, SCREEN_H);
  }
}



void drawTask(void *param)
{
  static states::MainState lastMain = states::MainState::HOME;
  static states::MenuState lastMenu = states::MenuState::SOUND;

  for (;;)
  {
    mainState = states::get_main();
    menuState = states::get_menu();
    GNSSstate = states::get_gnss();

    bool stateChanged =
      (mainState != lastMain || menuState != lastMenu);

    // HOME = animation-driven
    if (mainState == states::MainState::HOME)
    {
      updateAnimation(character);

      if (animTick || stateChanged)
      {
        renderScreen();
        animTick = false;
      }
    }
    else
    {
      // EVERYTHING ELSE = state-driven ONLY
      if (stateChanged)
      {
        renderScreen();
      }
    }

    lastMain = mainState;
    lastMenu = menuState;

    vTaskDelay(pdMS_TO_TICKS(16));
  }
}

void drawSetup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(0);

  spr.createSprite(SCREEN_W, SCREEN_H);
  spr.setSwapBytes(true);

  

  if (!utils::shared_spi::init()) {
    Serial.println("❌ SHARED SPI FAILED");
    while(1);
  }
  if (!utils::sd_card::init()) {
    Serial.println("❌ SD INIT FAILED (WRAPPER)");
    while (1);
  }
  delay(200);

  Serial.println("✅ SD INIT OK");
  character.type = RACCOON;
  mainState = states::MainState::HOME;
  Serial.println("✅ SD READY");

  character.type = RACCOON;
  setCharacterAnim(character, "idleRaccoon", 5);

  mainState = states::MainState::HOME;
  //needsRedraw = true;

  renderScreen();
  spr.pushSprite(0,0);
}