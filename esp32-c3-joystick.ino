#include <Wire.h>
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

// === I2C ===
#define SDA 8
#define SCL 9

// === Пин буззера ===
#define BUZZER_PIN 2

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

// === Структура игры тетрис ===
struct Piece {
  int type;      // тип фигуры (0-6)
  int rotation;  // поворот (0-3)
  int x;         // позиция X
  int y;         // позиция Y
};

// === Preferences для сохранения настроек ===
Preferences preferences;

bool sound = true;
int startupMelody = 1; // 0=OFF, 1-5 - melodies

void loadPreferences() {
  preferences.begin("settings", true);
  sound = preferences.getBool("sound", true);
  startupMelody = preferences.getInt("melody", 1);
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


void setup() {
  Wire.begin(SDA, SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  analogReadResolution(12);
  pinMode(LB_PIN, INPUT_PULLUP);
  pinMode(RB_PIN, INPUT_PULLUP);
  pinMode(LX_PIN, INPUT);
  pinMode(LY_PIN, INPUT);
  pinMode(RX_PIN, INPUT);
  pinMode(RY_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  display.setCursor(0, 4);
  display.print("   ESP32 Joystick   ");
  display.display();
  
  loadPreferences();
  playMelody(startupMelody);
  delay(500);
  runMenu();
}


void loop() {

}
