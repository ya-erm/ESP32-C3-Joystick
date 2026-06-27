#include <WiFi.h>
#include <esp_now.h>

// MAC машинки для ESP-NOW
const uint8_t carMac[] = { 0x0C, 0x4E, 0xA0, 0x5F, 0xEF, 0x7C };

// Эти значения задаются в настройках
extern float turnSpeed;
extern float motorSpeed;
extern int motorDeadzone;
extern bool debug;

bool carEspNowReady = false;

const int CAR_UI_HEADER_HEIGHT = 14;
const int CAR_UI_ROW_Y = 16;
const int CAR_UI_ROW_HEIGHT = 10;
const int CAR_UI_ROW_GAP = 5;
const int CAR_UI_TEXT_X = 16;
const int CAR_UI_ROW_COUNT = 5;
const unsigned long CAR_UI_REFRESH_INTERVAL = 50;

bool carUiInitialized = false;
unsigned long carUiLastDisplayRedraw = 0;
unsigned long carUiLastBatteryRedraw = 0;
char carUiRows[CAR_UI_ROW_COUNT][28];

void initCar() {
  Serial.println("[Car] init...");
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("[Car] ERROR: esp_now_init() failed");
    carEspNowReady = false;
    return;
  }
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, carMac, 6);
  peer.channel = 0;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("[Car] ERROR: esp_now_add_peer() failed");
    carEspNowReady = false;
    return;
  }
  carEspNowReady = true;
  Serial.printf("[Car] OK, peer %02X:%02X:%02X:%02X:%02X:%02X\n",
                carMac[0], carMac[1], carMac[2], carMac[3], carMac[4], carMac[5]);
}

void sendCarCommand(const char* cmd) {
  if (!carEspNowReady) {
    Serial.println("[Car] sendCarCommand: ESP-NOW not ready, exit");
    return;
  };
  if (!cmd) {
    Serial.println("[Car] sendCarCommand: cmd is NULL");
    return;
  }
  size_t len = strlen(cmd);
  if (len > 64) {
    Serial.printf("[Car] sendCarCommand: cmd too long (%u)\n", (unsigned)len);
    return;
  }
  esp_err_t result = esp_now_send(carMac, (const uint8_t*)cmd, len + 1);
  if (result == ESP_OK) {
    if (debug) Serial.printf("[Car] sendCarCommand: OK, sent %s\n", cmd);
  } else {
    Serial.printf("[Car] sendCarCommand: Send failed: %d\n", result);
  }
}

void drawCarControlHeader() {
  display.fillRect(0, 0, SCREEN_WIDTH, CAR_UI_HEADER_HEIGHT, SSD1306_BLACK);
  display.setTextSize(1);
  display.setTextColor(UI_HEADER_COLOR);
  display.setCursor(8, 3);
  display.print("Car Control");
  display.drawLine(0, 13, SCREEN_WIDTH - 1, 13, UI_SEPARATOR_COLOR);
}

void resetCarControlUi() {
  carUiInitialized = false;
  carUiLastDisplayRedraw = 0;
  carUiLastBatteryRedraw = 0;
  for (int i = 0; i < CAR_UI_ROW_COUNT; i++) {
    carUiRows[i][0] = '\0';
  }
}

bool drawCarControlRow(int row, const char* text, bool force) {
  if (!force && strcmp(carUiRows[row], text) == 0) return false;

  int y;
  if (row < 2) {
    y = CAR_UI_ROW_Y + row * CAR_UI_ROW_HEIGHT;
  } else if (row < 4) {
    y = CAR_UI_ROW_Y + row * CAR_UI_ROW_HEIGHT + CAR_UI_ROW_GAP;
  } else {
    y = SCREEN_HEIGHT - CAR_UI_ROW_HEIGHT;
  }

  display.fillRect(0, y, SCREEN_WIDTH, CAR_UI_ROW_HEIGHT, SSD1306_BLACK);
  if (row == CAR_UI_ROW_COUNT - 1) {
    display.drawLine(0, y - 3, SCREEN_WIDTH - 1, y - 3, UI_SEPARATOR_COLOR);
  }
  display.setTextSize(1);
  display.setTextColor(row == CAR_UI_ROW_COUNT - 1 ? UI_FOOTER_COLOR : SSD1306_WHITE);
  display.setCursor(CAR_UI_TEXT_X, y);
  display.print(text);

  strncpy(carUiRows[row], text, sizeof(carUiRows[row]) - 1);
  carUiRows[row][sizeof(carUiRows[row]) - 1] = '\0';
  return true;
}

void drawCarControlUi(
  const JoystickData& input,
  int m1,
  int m2,
  int m3,
  int m4
) {
  bool force = !carUiInitialized;
  bool changed = false;

  if (force) {
    display.clearDisplay();
    drawCarControlHeader();
    changed = true;
  }

  char line[28];
  snprintf(line, sizeof(line), "LX:%4d %c  RX:%4d %c", input.lx, input.lhd, input.rx, input.rhd);
  changed |= drawCarControlRow(0, line, force);

  snprintf(line, sizeof(line), "LY:%4d %c  RY:%4d %c", input.ly, input.lvd, input.ry, input.rvd);
  changed |= drawCarControlRow(1, line, force);

  snprintf(line, sizeof(line), "M1:%4d    M2:%4d", m1, m2);
  changed |= drawCarControlRow(2, line, force);

  snprintf(line, sizeof(line), "M3:%4d    M4:%4d", m3, m4);
  changed |= drawCarControlRow(3, line, force);

  snprintf(line, sizeof(line), "LB - exit");
  changed |= drawCarControlRow(4, line, force);

  unsigned long now = millis();
  if (force || now - carUiLastBatteryRedraw >= 1000) {
    drawBatteryIcon();
    carUiLastBatteryRedraw = now;
    changed = true;
  }

  if (changed) display.display();
  carUiInitialized = true;
  carUiLastDisplayRedraw = now;
}

void runCarMode() {
  Serial.println("[Car] runCarMode enter");
  if (!carEspNowReady) {
    Serial.println("[Car] runCarMode: ESP-NOW not ready, exit");
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("ESP-NOW init fail");
    display.display();
    delay(2000);
    return;
  }

  unsigned long lastSend = 0;
  const unsigned long sendInterval = 50;
  resetCarControlUi();

  while (true) {
    JoystickData input = readJoysticks();

    // Выход: левая кнопка
    if (input.lb) {
      Serial.println("[Car] runCarMode exit (LB), send S");
      sendCarCommand("S");
      if (sound) tone(BUZZER_PIN, BACK_TONE, 50);
      delay(200);
      return;
    }

    // Правый стик: только RY — вперёд/назад (оба борта одинаково), коэффициент из настроек
    int ry = constrain(input.ry, -100, 100);
    if (abs(ry) < joystickDeadzone) ry = 0;
    int limit = (int)(100.0f * motorSpeed);
    int left  = map(ry, -100, 100, -limit, limit);
    int right = map(ry, -100, 100, -limit, limit);  
    // Левый стик LX: разворот на месте
    int turnRaw = constrain(input.lx, -100, 100);
    if (abs(turnRaw) < joystickDeadzone) turnRaw = 0;
    int turn = (int)(turnRaw * turnSpeed);
    left  += turn;
    right -= turn;
    left  = constrain(left,  -100, 100);
    right = constrain(right, -100, 100);

    int ml = left == 0 
        ? 0 
        : left > 0 
            ? map(left, 0, 100, motorDeadzone, 100) 
            : map(left, -100, 0, -100, -motorDeadzone);
    int mr = right == 0 
        ? 0 
        : right > 0 
            ? map(right, 0, 100, motorDeadzone, 100) 
            : map(right, -100, 0, -100, -motorDeadzone);

    int m1 = map(ml, -100, 100, -255, 255);  // задний левый
    int m2 = map(mr, -100, 100, -255, 255);  // задний правый
    int m3 = map(ml, -100, 100, -255, 255);  // передний левый
    int m4 = map(mr, -100, 100, -255, 255);  // передний правый

    if (m1 == 0 && m2 == 0 && m3 == 0 && m4 == 0) {
      sendCarCommand("S");
    } else if (millis() - lastSend >= sendInterval) {
      char cmd[32];
      snprintf(cmd, sizeof(cmd), "V:%d:%d:%d:%d", m1, m2, m3, m4);
      sendCarCommand(cmd);
      lastSend = millis();
    }

    unsigned long now = millis();
    if (!carUiInitialized || now - carUiLastDisplayRedraw >= CAR_UI_REFRESH_INTERVAL) {
      drawCarControlUi(input, m1, m2, m3, m4);
    } else if (now - carUiLastBatteryRedraw >= 1000) {
      drawBatteryIcon();
      display.display();
      carUiLastBatteryRedraw = now;
    }

    delay(20);
  }
}
