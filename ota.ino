#include <ArduinoOTA.h>
#include <WiFi.h>
#include <esp_now.h>
#include "wifi_config.h"

// Create a wifi_config.h file with the following content:
//
// #ifndef WIFI_SSID
// #define WIFI_SSID "YOUR_WIFI_SSID"
// #endif
// #ifndef WIFI_PASSWORD
// #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
// #endif

extern bool carEspNowReady;
extern bool yellowEspNowReady;

static const char* OTA_HOSTNAME = "esp32-c3-joystick";
static const char* OTA_SSID = WIFI_SSID;
static const char* OTA_PASSWORD = WIFI_PASSWORD;

static bool otaReady = false;
static bool otaUpdateInProgress = false;
static int otaProgress = -1;
static char otaStatus[32] = "Starting";
static bool otaUiInitialized = false;
static unsigned long otaUiLastBatteryDraw = 0;
static char otaUiRows[5][32];

static bool isOtaWiFiConfigured() {
  return strlen(OTA_SSID) > 0;
}

static const char* getOtaWiFiPassword() {
  return OTA_PASSWORD;
}

static void setOtaStatus(const char* status) {
  snprintf(otaStatus, sizeof(otaStatus), "%s", status);
}

static const char* getOtaWiFiStatus() {
  if (!isOtaWiFiConfigured()) return "set SSID";

  switch (WiFi.status()) {
    case WL_CONNECTED:
      return "connected";
    case WL_NO_SSID_AVAIL:
      return "no SSID";
    case WL_CONNECT_FAILED:
      return "failed";
    case WL_CONNECTION_LOST:
      return "lost";
    case WL_DISCONNECTED:
      return "connecting";
    default:
      return "waiting";
  }
}

static void stopEspNowForOta() {
  esp_now_deinit();
  carEspNowReady = false;
  yellowEspNowReady = false;
}

static void restoreEspNowAfterOta() {
  if (otaReady) {
    ArduinoOTA.end();
  }
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  initCar();
  initYellowCar();
}

static void resetOtaPageUi() {
  otaUiInitialized = false;
  otaUiLastBatteryDraw = 0;
  for (int i = 0; i < 5; i++) {
    otaUiRows[i][0] = '\0';
  }
}

static int otaPageRowY(int row) {
  const int rowY = 16;
  const int rowH = 10;
  if (row < 2) return rowY + row * rowH;
  if (row < 4) return rowY + row * rowH + 5;
  return SCREEN_HEIGHT - rowH;
}

static bool drawOtaPageRow(int row, const char* text, bool force) {
  if (!force && strcmp(otaUiRows[row], text) == 0) return false;

  const int textX = 16;
  const int rowH = 10;
  int y = otaPageRowY(row);
  display.fillRect(0, y, SCREEN_WIDTH, rowH, SSD1306_BLACK);
  if (row == 4) {
    display.drawLine(0, y - 3, SCREEN_WIDTH - 1, y - 3, UI_SEPARATOR_COLOR);
  }
  display.setTextSize(1);
  display.setTextColor(row == 4 ? UI_FOOTER_COLOR : SSD1306_WHITE);
  display.setCursor(textX, y);
  display.print(text);

  strncpy(otaUiRows[row], text, sizeof(otaUiRows[row]) - 1);
  otaUiRows[row][sizeof(otaUiRows[row]) - 1] = '\0';
  return true;
}

static void drawOtaPage() {
  bool force = !otaUiInitialized;
  bool changed = false;

  if (force) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(UI_HEADER_COLOR);
    display.setCursor(8, 3);
    display.print("OTA Update");
    display.drawLine(0, 13, SCREEN_WIDTH - 1, 13, UI_SEPARATOR_COLOR);
    changed = true;
  }

  char line[32];
  snprintf(line, sizeof(line), "WiFi:%s", getOtaWiFiStatus());
  changed |= drawOtaPageRow(0, line, force);

  char ssid[18];
  snprintf(ssid, sizeof(ssid), "%s", OTA_SSID);
  if (strlen(OTA_SSID) > 17) {
    ssid[14] = '.';
    ssid[15] = '.';
    ssid[16] = '.';
    ssid[17] = '\0';
  }
  snprintf(line, sizeof(line), "SSID:%s", ssid);
  changed |= drawOtaPageRow(1, line, force);

  if (WiFi.status() == WL_CONNECTED) {
    snprintf(line, sizeof(line), "IP:%s", WiFi.localIP().toString().c_str());
  } else {
    snprintf(line, sizeof(line), "IP:--");
  }
  changed |= drawOtaPageRow(2, line, force);

  if (otaUpdateInProgress) {
    snprintf(line, sizeof(line), "OTA:updating...");
  } else if (otaProgress >= 0) {
    snprintf(line, sizeof(line), "OTA:%d%%", otaProgress);
  } else {
    snprintf(line, sizeof(line), "OTA:%s", otaStatus);
  }
  changed |= drawOtaPageRow(3, line, force);

  changed |= drawOtaPageRow(4, otaUpdateInProgress ? "Updating..." : "LB - exit", force);

  unsigned long now = millis();
  if (force || now - otaUiLastBatteryDraw >= 1000) {
    drawBatteryIcon();
    otaUiLastBatteryDraw = now;
    changed = true;
  }

  if (changed) display.display();
  otaUiInitialized = true;
}

static void beginOtaService() {
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.onStart([]() {
    otaUpdateInProgress = true;
    otaProgress = 0;
    setOtaStatus("Receiving");
    drawOtaPage();
  });
  ArduinoOTA.onEnd([]() {
    otaProgress = 100;
    otaUpdateInProgress = false;
    setOtaStatus("Done");
    drawOtaPage();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    otaProgress = total == 0 ? 0 : (progress * 100) / total;
  });
  ArduinoOTA.onError([](ota_error_t error) {
    otaUpdateInProgress = false;
    otaProgress = -1;
    switch (error) {
      case OTA_AUTH_ERROR:
        setOtaStatus("Auth error");
        break;
      case OTA_BEGIN_ERROR:
        setOtaStatus("Begin error");
        break;
      case OTA_CONNECT_ERROR:
        setOtaStatus("Connect error");
        break;
      case OTA_RECEIVE_ERROR:
        setOtaStatus("Receive error");
        break;
      case OTA_END_ERROR:
        setOtaStatus("End error");
        break;
      default:
        setOtaStatus("Error");
        break;
    }
    drawOtaPage();
  });
  ArduinoOTA.begin();
  otaReady = true;
  setOtaStatus("Ready");
}

void runOtaMode() {
  Serial.println("[OTA] enter");
  otaReady = false;
  otaUpdateInProgress = false;
  otaProgress = -1;
  setOtaStatus("Starting");
  resetOtaPageUi();

  stopEspNowForOta();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setHostname(OTA_HOSTNAME);

  if (isOtaWiFiConfigured()) {
    WiFi.begin(OTA_SSID, getOtaWiFiPassword());
    setOtaStatus("Connecting");
  } else {
    setOtaStatus("No SSID");
  }

  unsigned long lastDraw = 0;

  while (true) {
    if (isOtaWiFiConfigured() && WiFi.status() == WL_CONNECTED && !otaReady) {
      beginOtaService();
      Serial.printf("[OTA] ready on %s\n", WiFi.localIP().toString().c_str());
    }

    if (otaReady) {
      ArduinoOTA.handle();
    }

    if (millis() - lastDraw > 500) {
      drawOtaPage();
      lastDraw = millis();
    }

    if (readLeftButton() && !otaUpdateInProgress) {
      if (sound) tone(BUZZER_PIN, BACK_TONE, 50);
      delay(200);
      restoreEspNowAfterOta();
      Serial.println("[OTA] exit, ESP-NOW restored");
      return;
    }

    delay(otaUpdateInProgress ? 1 : 20);
  }
}
