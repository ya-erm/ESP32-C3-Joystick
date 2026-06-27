extern bool menuNeedsRedraw;

static const uint16_t DISPLAY_TEST_COLORS[] = {
  ST77XX_RED,
  ST77XX_GREEN,
  ST77XX_BLUE,
  ST77XX_WHITE,
  ST77XX_BLACK,
};

static const char* DISPLAY_TEST_COLOR_NAMES[] = {
  "RED",
  "GREEN",
  "BLUE",
  "WHITE",
  "BLACK",
};

static const uint8_t DISPLAY_TEST_COLOR_COUNT =
  sizeof(DISPLAY_TEST_COLORS) / sizeof(DISPLAY_TEST_COLORS[0]);

static uint16_t displayContrastColor(uint16_t color) {
  return color == ST77XX_BLACK ? ST77XX_WHITE : ST77XX_BLACK;
}

static void drawDisplaySolidColor(uint8_t colorIndex, bool autoMode) {
  colorIndex %= DISPLAY_TEST_COLOR_COUNT;
  uint16_t color = DISPLAY_TEST_COLORS[colorIndex];
  uint16_t textColor = displayContrastColor(color);

  display.fillScreen(color);
  display.drawRect(0, 0, display.width(), display.height(), textColor);
  display.setTextSize(1);
  display.setTextColor(textColor, color);
  display.setCursor(6, 6);
  display.print(DISPLAY_TEST_COLOR_NAMES[colorIndex]);
  display.setCursor(6, 18);
  display.print(autoMode ? "AUTO ON" : "AUTO OFF");
  display.setCursor(6, display.height() - 18);
  display.print("R-stick color");
  display.setCursor(6, display.height() - 9);
  display.print("LB - exit, RB - auto");
}

void runDisplayTest() {
  uint8_t colorIndex = 0;
  bool autoMode = true;
  bool prevLb = true;
  bool prevRb = true;
  bool stickReady = true;
  unsigned long lastAutoChange = 0;

  display.setRotation(TFT_MENU_ROTATION);
  drawDisplaySolidColor(colorIndex, autoMode);
  lastAutoChange = millis();

  while (true) {
    JoystickData input = readJoysticks();
    bool lbPressed = input.lb && !prevLb;
    bool rbPressed = input.rb && !prevRb;
    unsigned long now = millis();

    if (lbPressed) {
      display.setRotation(TFT_MENU_ROTATION);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      menuNeedsRedraw = true;
      return;
    }

    if (rbPressed) {
      autoMode = !autoMode;
      drawDisplaySolidColor(colorIndex, autoMode);
      lastAutoChange = now;
      if (sound) tone(BUZZER_PIN, OK_TONE, 50);
    }

    int stickMove = 0;
    if (input.rx > 50 || input.ry > 50) stickMove = 1;
    else if (input.rx < -50 || input.ry < -50) stickMove = -1;

    if (stickMove == 0) {
      stickReady = true;
    } else if (stickReady) {
      stickReady = false;
      colorIndex = (colorIndex + DISPLAY_TEST_COLOR_COUNT + stickMove) % DISPLAY_TEST_COLOR_COUNT;
      drawDisplaySolidColor(colorIndex, autoMode);
      lastAutoChange = now;
      if (sound) tone(BUZZER_PIN, MOVE_TONE, 35);
    }

    if (autoMode && now - lastAutoChange > 1800) {
      colorIndex = (colorIndex + 1) % DISPLAY_TEST_COLOR_COUNT;
      drawDisplaySolidColor(colorIndex, autoMode);
      lastAutoChange = now;
    }

    prevLb = input.lb;
    prevRb = input.rb;
    delay(40);
  }
}
