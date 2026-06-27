#include <WiFi.h>
#include <esp_now.h>

const uint8_t yellowCarMac[] = { 0x48, 0x55, 0x19, 0xC8, 0x72, 0x5F };

extern int yellowMaxSpeed;
extern int joystickDeadzone;
extern bool debug;
extern bool sound;

bool yellowEspNowReady = false;

const int YELLOW_CAR_UI_HEADER_HEIGHT = 14;
const int YELLOW_CAR_UI_ROW_Y = 16;
const int YELLOW_CAR_UI_ROW_HEIGHT = 10;
const int YELLOW_CAR_UI_ROW_GAP = 5;
const int YELLOW_CAR_UI_TEXT_X = 16;
const int YELLOW_CAR_UI_ROW_COUNT = 5;
const unsigned long YELLOW_CAR_UI_REFRESH_INTERVAL = 50;

bool yellowCarUiInitialized = false;
unsigned long yellowCarUiLastDisplayRedraw = 0;
unsigned long yellowCarUiLastBatteryRedraw = 0;
char yellowCarUiRows[YELLOW_CAR_UI_ROW_COUNT][28];

void initYellowCar() {
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, yellowCarMac, 6);
  peer.channel = 0;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("[YellowCar] ERROR: esp_now_add_peer() failed");
    yellowEspNowReady = false;
    return;
  }
  yellowEspNowReady = true;
  Serial.printf("[YellowCar] OK, peer %02X:%02X:%02X:%02X:%02X:%02X\n",
                yellowCarMac[0], yellowCarMac[1], yellowCarMac[2],
                yellowCarMac[3], yellowCarMac[4], yellowCarMac[5]);
}

void sendYellowCarCommand(const char* cmd) {
  if (!yellowEspNowReady || !cmd) return;
  size_t len = strlen(cmd);
  if (len > 64) return;
  esp_err_t result = esp_now_send(yellowCarMac, (const uint8_t*)cmd, len + 1);
  if (result == ESP_OK) {
    if (debug) Serial.printf("[YellowCar] sent: %s\n", cmd);
  } else {
    Serial.printf("[YellowCar] send failed: %d\n", result);
  }
}

void drawYellowCarHeader() {
  display.fillRect(0, 0, SCREEN_WIDTH, YELLOW_CAR_UI_HEADER_HEIGHT, SSD1306_BLACK);
  display.setTextSize(1);
  display.setTextColor(UI_HEADER_COLOR);
  display.setCursor(8, 3);
  display.print("Yellow Car");
  display.drawLine(0, 13, SCREEN_WIDTH - 1, 13, UI_SEPARATOR_COLOR);
}

void resetYellowCarUi() {
  yellowCarUiInitialized = false;
  yellowCarUiLastDisplayRedraw = 0;
  yellowCarUiLastBatteryRedraw = 0;
  for (int i = 0; i < YELLOW_CAR_UI_ROW_COUNT; i++) {
    yellowCarUiRows[i][0] = '\0';
  }
}

bool drawYellowCarRow(int row, const char* text, bool force) {
  if (!force && strcmp(yellowCarUiRows[row], text) == 0) return false;

  int y;
  if (row < 2) {
    y = YELLOW_CAR_UI_ROW_Y + row * YELLOW_CAR_UI_ROW_HEIGHT;
  } else if (row < 4) {
    y = YELLOW_CAR_UI_ROW_Y + row * YELLOW_CAR_UI_ROW_HEIGHT + YELLOW_CAR_UI_ROW_GAP;
  } else {
    y = SCREEN_HEIGHT - YELLOW_CAR_UI_ROW_HEIGHT;
  }

  display.fillRect(0, y, SCREEN_WIDTH, YELLOW_CAR_UI_ROW_HEIGHT, SSD1306_BLACK);
  if (row == YELLOW_CAR_UI_ROW_COUNT - 1) {
    display.drawLine(0, y - 3, SCREEN_WIDTH - 1, y - 3, UI_SEPARATOR_COLOR);
  }
  display.setTextSize(1);
  display.setTextColor(row == YELLOW_CAR_UI_ROW_COUNT - 1 ? UI_FOOTER_COLOR : SSD1306_WHITE);
  display.setCursor(YELLOW_CAR_UI_TEXT_X, y);
  display.print(text);

  strncpy(yellowCarUiRows[row], text, sizeof(yellowCarUiRows[row]) - 1);
  yellowCarUiRows[row][sizeof(yellowCarUiRows[row]) - 1] = '\0';
  return true;
}

void drawYellowCarUi(const JoystickData& input, int motorVal, int servoVal) {
  bool force = !yellowCarUiInitialized;
  bool changed = false;

  if (force) {
    display.clearDisplay();
    drawYellowCarHeader();
    changed = true;
  }

  char line[28];
  snprintf(line, sizeof(line), "LX:%4d %c  RX:%4d %c", input.lx, input.lhd, input.rx, input.rhd);
  changed |= drawYellowCarRow(0, line, force);

  snprintf(line, sizeof(line), "LY:%4d %c  RY:%4d %c", input.ly, input.lvd, input.ry, input.rvd);
  changed |= drawYellowCarRow(1, line, force);

  snprintf(line, sizeof(line), "Motor:%4d", motorVal);
  changed |= drawYellowCarRow(2, line, force);

  snprintf(line, sizeof(line), "Servo:%4d", servoVal);
  changed |= drawYellowCarRow(3, line, force);

  snprintf(line, sizeof(line), "LB - exit");
  changed |= drawYellowCarRow(4, line, force);

  unsigned long now = millis();
  if (force || now - yellowCarUiLastBatteryRedraw >= 1000) {
    drawBatteryIcon();
    yellowCarUiLastBatteryRedraw = now;
    changed = true;
  }

  if (changed) display.display();
  yellowCarUiInitialized = true;
  yellowCarUiLastDisplayRedraw = now;
}

void runYellowCarMode() {
  Serial.println("[YellowCar] enter");
  if (!yellowEspNowReady) {
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("ESP-NOW init fail");
    display.display();
    delay(2000);
    return;
  }

  unsigned long lastSend = 0;
  const unsigned long sendInterval = 100;
  int lastServo = 0;
  resetYellowCarUi();

  while (true) {
    JoystickData input = readJoysticks();

    if (input.lb) {
      sendYellowCarCommand("M1 0");
      if (sound) tone(BUZZER_PIN, BACK_TONE, 50);
      delay(200);
      return;
    }

    // RY → мотор (вперёд/назад)
    int ry = constrain(input.ry, -100, 100);
    if (abs(ry) < joystickDeadzone) ry = 0;
    int motorVal = map(ry, -100, 100, -yellowMaxSpeed, yellowMaxSpeed);

    // LX → сервопривод руля (-45..45)
    int lx = constrain(input.lx, -100, 100);
    if (abs(lx) < joystickDeadzone) lx = 0;
    int servoVal = map(lx, -100, 100, -45, 45);

    if (millis() - lastSend >= sendInterval) {
      char cmd[16];
      snprintf(cmd, sizeof(cmd), "M1 %d", motorVal);
      sendYellowCarCommand(cmd);
      if (servoVal != lastServo) {
        snprintf(cmd, sizeof(cmd), "S1 %d", servoVal);
        sendYellowCarCommand(cmd);
        lastServo = servoVal;
      }
      lastSend = millis();
    }

    unsigned long now = millis();
    if (!yellowCarUiInitialized || now - yellowCarUiLastDisplayRedraw >= YELLOW_CAR_UI_REFRESH_INTERVAL) {
      drawYellowCarUi(input, motorVal, servoVal);
    } else if (now - yellowCarUiLastBatteryRedraw >= 1000) {
      drawBatteryIcon();
      display.display();
      yellowCarUiLastBatteryRedraw = now;
    }

    delay(20);
  }
}
