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
        }
    
        // Если нажата левая кнока
        if (!prev_lb && input.lb) {
            if (sound) tone(BUZZER_PIN, BACK_TONE, 150);
            return;
        }
    
        prev_rb = input.rb;
        prev_lb = input.lb;
    
        // === Вывод на экран ===
        display.clearDisplay();
        display.setCursor(8, 3);
        display.printf("Status %s", raw ? "(raw)" : "");
        display.drawLine(0, 13, 127, 13, SSD1306_WHITE);
        int l0 = 16;
        display.setCursor(0, l0);
        display.printf(" Lx:%4d %c  Rx:%4d %c", raw ? input.lx_raw : input.lx, input.lhd, raw ? input.rx_raw : input.rx, input.rhd);
        display.setCursor(0, l0+8);
        display.printf(" Ly:%4d %c  Ry:%4d %c", raw ? input.ly_raw : input.ly, input.lvd, raw ? input.ry_raw : input.ry, input.rvd);
        display.setCursor(0, l0+16);
        display.printf(" LB:%s    RB:%s", input.lb ? "PRES" : "----", input.rb ? "PRES" : "----");

        display.setCursor(0, l0+24);
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
            display.printf(" GPIO2:%u", displayAverage);
            display.write((uint8_t)0xF1); // символ ± в шрифте CP437
            display.print(displaySpread);
        } else {
            display.printf(" Battery: %.2f V", batteryVoltageFromRaw(filteredBatteryRaw));
        }
        
        display.setCursor(0, l0+40);
        display.printf(" LB - back  RB - raw");
        drawBatteryIcon();
        display.display();
        delay(150);
    }
}
