#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Preferences.h>


// === TFT ST7735 80x160 ===
// Подключение по SPI. При необходимости поменяй GPIO здесь.
#define TFT_SCLK 9
#define TFT_MOSI 8
#define TFT_CS   20
#define TFT_DC   5
#define TFT_RST  -1

#define TFT_DIAGNOSTIC false

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 80
#define TFT_MENU_ROTATION 3
#define TFT_TETRIS_ROTATION 2
#define TFT_INIT_OPTIONS INITR_MINI160x80
#define TFT_COL_OFFSET 24
#define TFT_ROW_OFFSET 0

// Панель работает с BGR-порядком каналов при корректной MINI160x80-геометрии.
// Оставляем имена цветов семантическими для всего скетча.
#undef ST77XX_RED
#undef ST77XX_BLUE
#undef ST77XX_YELLOW
#undef ST77XX_CYAN
#undef ST77XX_ORANGE
#define ST77XX_RED 0x001F
#define ST77XX_BLUE 0xF800
#define ST77XX_YELLOW 0x07FF
#define ST77XX_CYAN 0xFFE0
#define ST77XX_ORANGE 0x053F

#define SSD1306_BLACK ST77XX_BLACK
#define SSD1306_WHITE ST77XX_WHITE
#define UI_HEADER_COLOR ST77XX_YELLOW
#define UI_FOOTER_COLOR 0x8410
#define UI_SEPARATOR_COLOR UI_FOOTER_COLOR

class JoystickDisplay : public Adafruit_ST7735 {
public:
  JoystickDisplay(SPIClass* spiClass, int8_t cs, int8_t dc, int8_t rst)
    : Adafruit_ST7735(spiClass, cs, dc, rst) {}

  JoystickDisplay(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst)
    : Adafruit_ST7735(cs, dc, mosi, sclk, rst) {}

  void clearDisplay() {
    fillScreen(SSD1306_BLACK);
  }

  void setPanelOffset(int8_t col, int8_t row) {
    setColRowStart(col, row);
  }

  void display() {
    // ST7735 рисует сразу в видеопамять дисплея, отдельной отправки буфера нет.
  }
};

JoystickDisplay display(&SPI, TFT_CS, TFT_DC, TFT_RST);

// === Пины джойстиков ===
#define LX_PIN 0
#define LY_PIN 1
#define LB_PIN 7

#define RX_PIN 4
#define RY_PIN 3
#define RB_PIN 10

// === Измерение напряжения аккумулятора ===
#define BATTERY_ADC_PIN 2

// === Пин буззера ===
#define BUZZER_PIN 6

#define OK_TONE 1568    // G6 – Яркий подтверждающий звук
#define BACK_TONE 784   // G5 – Более мягкий для "назад"
#define MOVE_TONE 1175  // D6 – Средний по высоте для перемещения

// === Структура для данных джойстика ===
struct JoystickData {
  // сырые значения
  int lx_raw;
  int ly_raw;
  int rx_raw;
  int ry_raw;
  // обработанные значения
  int lx;
  int ly;
  int rx;
  int ry;
  // направления
  char lhd;
  char lvd;
  char rhd;
  char rvd;
  // нажатия на кнопки
  bool lb;
  bool rb;
};

struct MenuItemDef {
  const char* label;
  void (*action)();
  const char* (*getValue)();
};

struct MenuDef {
  const char* title;
  const MenuItemDef* items;
  int itemCount;
  MenuDef* parent;
  int currentItemIndex;
};

// === Структура игры тетрис ===
struct Piece {
  int type;      // тип фигуры (0-6)
  int rotation;  // поворот (0-3)
  int x;         // позиция X
  int y;         // позиция Y
};

void drawDiagnosticFrame(JoystickDisplay& tft, const char* label) {
  tft.fillScreen(ST77XX_RED);
  delay(350);
  tft.fillScreen(ST77XX_GREEN);
  delay(350);
  tft.fillScreen(ST77XX_BLUE);
  delay(350);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 0);
  tft.println(label);
  tft.drawRect(0, 0, tft.width(), tft.height(), ST77XX_WHITE);
  tft.drawLine(0, 0, tft.width() - 1, tft.height() - 1, ST77XX_YELLOW);
  tft.drawLine(tft.width() - 1, 0, 0, tft.height() - 1, ST77XX_CYAN);
  delay(1200);
}

void runDisplayDiagnostic() {
  struct PinTest {
    const char* label;
    int8_t cs;
    int8_t dc;
    int8_t mosi;
    int8_t sclk;
  };
  struct InitTest {
    const char* label;
    uint8_t option;
  };

  const PinTest pinTests[] = {
    {"CS20 DC5 SDA8 SCL9", 20, 5, 8, 9},
    {"CS5 DC20 SDA8 SCL9", 5, 20, 8, 9},
    {"CS20 DC5 SDA9 SCL8", 20, 5, 9, 8},
    {"CS5 DC20 SDA9 SCL8", 5, 20, 9, 8},
  };
  const InitTest initTests[] = {
    {"MINI_PLUGIN", INITR_MINI160x80_PLUGIN},
    {"MINI", INITR_MINI160x80},
    {"BLACKTAB", INITR_BLACKTAB},
    {"GREENTAB", INITR_GREENTAB},
    {"REDTAB", INITR_REDTAB},
  };

  while (true) {
    for (uint8_t p = 0; p < sizeof(pinTests) / sizeof(pinTests[0]); p++) {
      for (uint8_t i = 0; i < sizeof(initTests) / sizeof(initTests[0]); i++) {
        char label[36];
        snprintf(label, sizeof(label), "%s\n%s", pinTests[p].label, initTests[i].label);
        Serial.printf("[TFT] %s / %s\n", pinTests[p].label, initTests[i].label);
        JoystickDisplay testDisplay(
          pinTests[p].cs,
          pinTests[p].dc,
          pinTests[p].mosi,
          pinTests[p].sclk,
          TFT_RST
        );
        testDisplay.initR(initTests[i].option);
        testDisplay.setRotation(1);
        drawDiagnosticFrame(testDisplay, label);
      }
    }
  }
}

int centeredTextX(const char* text, uint8_t textSize = 1) {
  int textWidth = strlen(text) * 6 * textSize;
  return max(0, (SCREEN_WIDTH - textWidth) / 2);
}

void drawCenteredText(const char* text, int y, uint8_t textSize, uint16_t color) {
  display.setTextSize(textSize);
  display.setTextColor(color);
  display.setCursor(centeredTextX(text, textSize), y);
  display.print(text);
}

void drawStartupSplash() {
  const int bodyX = 37;
  const int bodyY = 10;
  const int bodyW = 86;
  const int bodyH = 42;
  const int screenX = 64;
  const int screenY = 16;
  const int screenW = 32;
  const int screenH = 22;
  const uint16_t shellColor = 0x4A69;

  display.clearDisplay();

  display.drawRoundRect(bodyX, bodyY, bodyW, bodyH, 8, ST77XX_CYAN);
  display.fillRoundRect(bodyX + 2, bodyY + 2, bodyW - 4, bodyH - 4, 6, shellColor);

  display.drawRoundRect(screenX, screenY, screenW, screenH, 3, SSD1306_WHITE);
  display.fillRect(screenX + 3, screenY + 3, screenW - 6, screenH - 6, SSD1306_BLACK);
  display.drawFastHLine(screenX + 7, screenY + 9, screenW - 14, ST77XX_CYAN);
  display.drawFastHLine(screenX + 10, screenY + 14, screenW - 20, UI_FOOTER_COLOR);

  display.drawCircle(bodyX + 12, bodyY + 14, 8, SSD1306_WHITE);
  display.fillCircle(bodyX + 12, bodyY + 14, 4, SSD1306_BLACK);
  display.fillCircle(bodyX + 14, bodyY + 12, 2, ST77XX_CYAN);

  display.drawCircle(bodyX + bodyW - 12, bodyY + 14, 8, SSD1306_WHITE);
  display.fillCircle(bodyX + bodyW - 12, bodyY + 14, 4, SSD1306_BLACK);
  display.fillCircle(bodyX + bodyW - 10, bodyY + 12, 2, ST77XX_CYAN);

  display.drawRoundRect(screenX + 10, screenY + screenH + 2, 12, 5, 2, SSD1306_WHITE);
  display.fillRect(screenX + 12, screenY + screenH + 4, 4, 1, ST77XX_RED);
  display.fillRect(screenX + 17, screenY + screenH + 4, 3, 1, UI_FOOTER_COLOR);

  drawCenteredText("ESP32 Joystick", 61, 1, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();
}

MenuDef* currentMenu = nullptr;

// === Preferences для сохранения настроек ===
Preferences preferences;

bool sound = true;  // звуки
bool debug = false; // логирование отладочной информации

int startupMelody = 1;     // 0=OFF, 1-5 - melodies
int joystickDeadzone = 30; // мёртвая зона осей джойстика (0–100)
int motorDeadzone = 10;    // мёртвая зона моторов (0–30)
float motorSpeed = 0.8f;   // коэффициент скорости моторов (0.3–1.0)
float turnSpeed = 0.6f;    // коэффициент поворота (0.2–1.0)
int yellowMaxSpeed = 80;   // макс. скорость жёлтой машинки (10–100)

void loadPreferences() {
  preferences.begin("settings", true);
  sound = preferences.getBool("sound", true);
  startupMelody = preferences.getInt("melody", 1);
  turnSpeed = preferences.getFloat("turn", 0.6f);
  turnSpeed = constrain(turnSpeed, 0.2f, 1.0f);
  motorSpeed = preferences.getFloat("speed", 0.8f);
  motorSpeed = constrain(motorSpeed, 0.3f, 1.0f);
  debug = preferences.getBool("debug", false);
  motorDeadzone = preferences.getInt("carDz", 10);
  motorDeadzone = constrain(motorDeadzone, 0, 30);
  joystickDeadzone = preferences.getInt("joyDz", 30);
  joystickDeadzone = constrain(joystickDeadzone, 0, 100);
  yellowMaxSpeed = preferences.getInt("ycSpeed", 80);
  yellowMaxSpeed = constrain(yellowMaxSpeed, 10, 100);
  preferences.end();
}

void saveSoundSetting() {
  preferences.begin("settings", false);
  preferences.putBool("sound", sound);
  preferences.end();
}

void saveMelodySetting() {
  preferences.begin("settings", false);
  preferences.putInt("melody", startupMelody);
  preferences.end();
}

void saveTurnSetting() {
  preferences.begin("settings", false);
  preferences.putFloat("turn", turnSpeed);
  preferences.end();
}

void saveSpeedSetting() {
  preferences.begin("settings", false);
  preferences.putFloat("speed", motorSpeed);
  preferences.end();
}

void savedebugSetting() {
  preferences.begin("settings", false);
  preferences.putBool("debug", debug);
  preferences.end();
}

void saveCarDeadzoneSetting() {
  preferences.begin("settings", false);
  preferences.putInt("carDz", motorDeadzone);
  preferences.end();
}

void saveJoystickDeadzoneSetting() {
  preferences.begin("settings", false);
  preferences.putInt("joyDz", joystickDeadzone);
  preferences.end();
}

void saveYellowMaxSpeedSetting() {
  preferences.begin("settings", false);
  preferences.putInt("ycSpeed", yellowMaxSpeed);
  preferences.end();
}

void setup() {
  Serial.begin(115200);
  if (TFT_DIAGNOSTIC) {
    runDisplayDiagnostic();
    return;
  }

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  display.initR(TFT_INIT_OPTIONS);
  display.setPanelOffset(TFT_COL_OFFSET, TFT_ROW_OFFSET);
  display.setSPISpeed(40000000);
  display.setRotation(TFT_MENU_ROTATION);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  analogReadResolution(12);
  pinMode(LB_PIN, INPUT_PULLUP);
  pinMode(RB_PIN, INPUT_PULLUP);
  pinMode(LX_PIN, INPUT);
  pinMode(LY_PIN, INPUT);
  pinMode(RX_PIN, INPUT);
  pinMode(RY_PIN, INPUT);
  pinMode(BATTERY_ADC_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  drawStartupSplash();
  
  loadPreferences();
  playMelody(startupMelody);
  initCar();
  initYellowCar();
  initMenu();
  uint8_t mac[6];
  WiFi.macAddress(mac);
  Serial.printf("[Joystick] MAC %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  delay(500);
  runMenu();
}


void loop() {

}
