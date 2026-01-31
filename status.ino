extern bool sound;

void printStatus() {
    bool raw = false;
    bool prev_lb = false;
    bool prev_rb = false;

    while (true) {
        // === Считывание значений джойстика ===
        JoystickData input = readJoysticks();
    
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
        
        display.setCursor(0, l0+40);
        display.printf(" LB - back  RB - raw");
        display.display();
        delay(150);
    }
}
  