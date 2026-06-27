extern bool sound;

void printStatus() {
    bool raw = false;
    bool prev_lb = false;
    bool prev_rb = false;
    uint32_t rawWindowSum = 0;
    uint16_t rawWindowCount = 0;
    uint16_t rawWindowMin = 4095;
    uint16_t rawWindowMax = 0;
    uint16_t rawDisplayAverage = 0;
    uint16_t rawDisplaySpread = 0;
    bool rawWindowComplete = false;
    unsigned long rawWindowStartedAt = millis();
    float filteredBatteryRaw = -1.0f;
    bool statusUiInitialized = false;
    bool statusHeaderDirty = true;
    unsigned long lastBatteryDraw = 0;
    char statusRows[5][32] = {};

    const int textX = 16;
    const int rowY = 16;
    const int rowH = 10;

    auto drawStatusHeader = [&]() {
        display.fillRect(0, 0, SCREEN_WIDTH, 14, SSD1306_BLACK);
        display.setTextSize(1);
        display.setTextColor(UI_HEADER_COLOR);
        display.setCursor(8, 3);
        display.printf("Status %s", raw ? "(raw)" : "");
        display.drawLine(0, 13, SCREEN_WIDTH - 1, 13, UI_SEPARATOR_COLOR);
    };

    auto statusRowY = [&](int row) {
        if (row < 2) return rowY + row * rowH;
        if (row < 4) return rowY + row * rowH + 5;
        return SCREEN_HEIGHT - rowH;
    };

    auto drawStatusRow = [&](int row, const char* text, bool force) {
        if (!force && strcmp(statusRows[row], text) == 0) return false;
        int y = statusRowY(row);
        display.fillRect(0, y, SCREEN_WIDTH, rowH, SSD1306_BLACK);
        if (row == 4) {
            display.drawLine(0, y - 3, SCREEN_WIDTH - 1, y - 3, UI_SEPARATOR_COLOR);
        }
        display.setTextSize(1);
        display.setTextColor(row == 4 ? UI_FOOTER_COLOR : SSD1306_WHITE);
        display.setCursor(textX, y);
        display.print(text);
        strncpy(statusRows[row], text, sizeof(statusRows[row]) - 1);
        statusRows[row][sizeof(statusRows[row]) - 1] = '\0';
        return true;
    };

    while (true) {
        // === Считывание значений джойстика ===
        JoystickData input = readJoysticks();
        uint16_t batteryRaw = readBatteryAdcRaw();
        if (filteredBatteryRaw < 0.0f) {
            filteredBatteryRaw = batteryRaw;
        } else {
            constexpr float FILTER_ALPHA = 0.15f;
            filteredBatteryRaw += FILTER_ALPHA * (batteryRaw - filteredBatteryRaw);
        }

        unsigned long now = millis();
        if (now - rawWindowStartedAt >= 1000 && rawWindowCount > 0) {
            rawDisplayAverage = rawWindowSum / rawWindowCount;
            rawDisplaySpread = max(
                rawDisplayAverage - rawWindowMin,
                rawWindowMax - rawDisplayAverage
            );
            rawWindowComplete = true;
            rawWindowSum = 0;
            rawWindowCount = 0;
            rawWindowMin = 4095;
            rawWindowMax = 0;
            rawWindowStartedAt = now;
        }
        rawWindowSum += batteryRaw;
        rawWindowCount++;
        rawWindowMin = min(rawWindowMin, batteryRaw);
        rawWindowMax = max(rawWindowMax, batteryRaw);
    
        // === Обработка нажатий на кнопки ===
        // Если нажата правая кнопка - переключаем отображение raw значений
        if (!prev_rb && input.rb) {
            if (sound) tone(BUZZER_PIN, OK_TONE, 150);
            raw = !raw;
            statusHeaderDirty = true;
        }
    
        // Если нажата левая кнока
        if (!prev_lb && input.lb) {
            if (sound) tone(BUZZER_PIN, BACK_TONE, 150);
            return;
        }
    
        prev_rb = input.rb;
        prev_lb = input.lb;
    
        // === Вывод на экран ===
        bool force = !statusUiInitialized || statusHeaderDirty;
        bool changed = false;
        if (force) {
            display.clearDisplay();
            drawStatusHeader();
            changed = true;
            for (int i = 0; i < 5; i++) statusRows[i][0] = '\0';
            statusHeaderDirty = false;
        }

        char line[32];
        snprintf(line, sizeof(line), "LX:%4d %c  RX:%4d %c", raw ? input.lx_raw : input.lx, input.lhd, raw ? input.rx_raw : input.rx, input.rhd);
        changed |= drawStatusRow(0, line, force);

        snprintf(line, sizeof(line), "LY:%4d %c  RY:%4d %c", raw ? input.ly_raw : input.ly, input.lvd, raw ? input.ry_raw : input.ry, input.rvd);
        changed |= drawStatusRow(1, line, force);

        snprintf(line, sizeof(line), "LB:%s    RB:%s", input.lb ? "PRES" : "----", input.rb ? "PRES" : "----");
        changed |= drawStatusRow(2, line, force);

        if (raw) {
            uint16_t displayAverage = rawDisplayAverage;
            uint16_t displaySpread = rawDisplaySpread;
            if (!rawWindowComplete && rawWindowCount > 0) {
                displayAverage = rawWindowSum / rawWindowCount;
                displaySpread = max(
                    displayAverage - rawWindowMin,
                    rawWindowMax - displayAverage
                );
            }
            snprintf(line, sizeof(line), "GPIO2:%u%c%u", displayAverage, (char)0xF1, displaySpread);
        } else {
            snprintf(line, sizeof(line), "Battery: %.2f V", batteryVoltageFromRaw(filteredBatteryRaw));
        }
        changed |= drawStatusRow(3, line, force);

        changed |= drawStatusRow(4, "LB - back  RB - raw", force);

        if (force || now - lastBatteryDraw >= 1000) {
            drawBatteryIcon();
            lastBatteryDraw = now;
            changed = true;
        }

        if (changed) display.display();
        statusUiInitialized = true;
        delay(150);
    }
}
