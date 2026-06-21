#include <WiFi.h>
#include <esp_now.h>

const uint8_t yellowCarMac[] = { 0x48, 0x55, 0x19, 0xC8, 0x72, 0x5F };

extern int yellowMaxSpeed;
extern int joystickDeadzone;
extern bool debug;
extern bool sound;

bool yellowEspNowReady = false;

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

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(8, 3);
    display.print("Yellow Car");
    display.drawLine(0, 13, 127, 13, SSD1306_WHITE);
    int l0 = 16;
    display.setCursor(0, l0);
    display.printf(" Lx:%4d %c  Rx:%4d %c", input.lx, input.lhd, input.rx, input.rhd);
    display.setCursor(0, l0 + 8);
    display.printf(" Ly:%4d %c  Ry:%4d %c", input.ly, input.lvd, input.ry, input.rvd);
    display.setCursor(0, l0 + 20);
    display.printf(" Motor: %4d", motorVal);
    display.setCursor(0, l0 + 28);
    display.printf(" Servo: %4d", servoVal);
    display.setCursor(0, l0 + 40);
    display.printf(" LB - exit");
    drawBatteryIcon();
    display.display();

    delay(20);
  }
}
