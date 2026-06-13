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

static void drawOtaPage() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(8, 3);
  display.print("OTA Update");
  display.drawLine(0, 13, 127, 13, SSD1306_WHITE);

  display.setCursor(0, 16);
  display.print("WiFi:");
  display.print(getOtaWiFiStatus());

  display.setCursor(0, 26);
  display.print("SSID:");
  for (int i = 0; i < 13 && OTA_SSID[i] != '\0'; i++) {
    display.print(OTA_SSID[i]);
  }
  if (strlen(OTA_SSID) > 13) {
    display.print("...");
  }

  display.setCursor(0, 36);
  if (WiFi.status() == WL_CONNECTED) {
    display.print("IP:");
    display.print(WiFi.localIP());
  } else {
    display.print("IP:--");
  }

  display.setCursor(0, 46);
  display.print("OTA:");
  if (otaUpdateInProgress) {
    display.print("updating...");
  } else if (otaProgress >= 0) {
    display.print(otaProgress);
    display.print("%");
  } else {
    display.print(otaStatus);
  }

  display.setCursor(0, 56);
  display.print(otaUpdateInProgress ? "Updating..." : "LB - exit");
  display.display();
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
