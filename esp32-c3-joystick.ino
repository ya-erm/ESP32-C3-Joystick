#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// === Пины джойстиков ===
#define LX_PIN 0
#define LY_PIN 1
#define LB_PIN 7

#define RX_PIN 4
#define RY_PIN 3
#define RB_PIN 10

// === Измерение напряжения аккумулятора ===
#define BATTERY_ADC_PIN 2

// === I2C ===
#define SDA 8
#define SCL 9

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
  Wire.begin(SDA, SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
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

  display.setCursor(0, 4);
  display.print("   ESP32 Joystick   ");
  display.display();
  
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
