
enum MenuState {
    MENU_MAIN,
    MENU_GAMES,
    MENU_MUSIC,
    MENU_SETTINGS,
    MENU_STATUS,
    MENU_NONE
};
  
MenuState menuState = MENU_MAIN;

int menuIndex = 0;

void drawMenu(const char* title, const char* items[], int itemCount, int index) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(8, 3);
    display.println(title);
    display.drawLine(0, 13, 127, 13, SSD1306_WHITE);

    for (int i = 0; i < itemCount; i++) {
        display.setCursor(0, 16 + i * 10);
        if (i == index) { 
          display.print(">"); 
        }
        display.setCursor(8, 16 + i * 10);
        display.println(items[i]);
    }

    display.display();
}

void runMenu() {
    unsigned long lastInput = millis();
    menuState = MENU_MAIN;
    menuIndex = 0;

    while (true) {
        // === Чтение джойстиков ===
        JoystickData input = readJoysticks();

        int moveY = 0;
        if (input.ly > 50 || input.ry > 50) moveY = -1;
        else if (input.ly < -50 || input.ry < -50) moveY = 1;
        bool enter = input.rb || input.rx > 50;
        bool back = input.lb || input.rx < -50;
        
        // === Навигация ===
        static unsigned long lastMove = 0;
        if (millis() - lastMove > 250 && moveY != 0) {
            if (sound) tone(BUZZER_PIN, MOVE_TONE, 50);
            lastMove = millis();
            menuIndex += moveY;
            if (menuIndex < 0) menuIndex = 0;
        }

        // === Обработка по текущему меню ===
        if (menuState == MENU_MAIN) {
            const char* items[] = { 
              "Status", 
              "Games",
              "Music",
              "Settings"
            };
            int itemCount = 4;
            if (menuIndex >= itemCount) { menuIndex = itemCount - 1; }
            drawMenu("Menu", items, itemCount, menuIndex);

            if (enter) {
                if (sound) tone(BUZZER_PIN, OK_TONE, 50);
                if (menuIndex == 0) { //menuState = MENU_STATUS;
                    printStatus();
                }
                else if (menuIndex == 1) menuState = MENU_GAMES;
                else if (menuIndex == 2) menuState = MENU_MUSIC;
                else if (menuIndex == 3) menuState = MENU_SETTINGS;
                menuIndex = 0;
                delay(200);
            }
        }

        else if (menuState == MENU_STATUS) {
        }

        else if (menuState == MENU_GAMES) {
            const char* items[] = { 
              "Tennis",
              "Snake",
              "Tetris"
            };
            int itemCount = 3;
            if (menuIndex >= itemCount) menuIndex = itemCount - 1;
            drawMenu("Games", items, itemCount, menuIndex);

            if (enter) {
                if (sound) tone(BUZZER_PIN, OK_TONE, 50);
                if (menuIndex == 0) {
                    playTennisGame();
                } else if (menuIndex == 1) {
                    playSnakeGame();
                } else if (menuIndex == 2) {
                    playTetrisGame();
                }
                delay(200);
            }

            if (back) { 
                if (sound) tone(BUZZER_PIN, BACK_TONE, 50);
                menuState = MENU_MAIN; 
                menuIndex = 0;
                delay(200); 
            }
        }

        else if (menuState == MENU_MUSIC) {
            const char* melodyNames[] = { "Harry Potter", "Star Wars", "Pacman", "Super Mario", "Tetris" };
            int itemCount = 5;
            if (menuIndex >= itemCount) menuIndex = 0;
            drawMenu("Music", melodyNames, itemCount, menuIndex);

            if (enter) {
                // if (sound) tone(BUZZER_PIN, OK_TONE, 50);
                // startupMelody = menuIndex;
                // saveMelodySetting();

                // Проигрываем выбранную мелодию
                playMusic(menuIndex);
            }
            if (back) {
                if (sound) tone(BUZZER_PIN, BACK_TONE, 50);
                menuState = MENU_MAIN;
                menuIndex = 0;
                delay(200);
            }
        }

        else if (menuState == MENU_SETTINGS) {
            const char* melodyNames[] = { "OFF", "Mario", "Nokia" };
            static char melodyStr[20];
            snprintf(melodyStr, sizeof(melodyStr), "Melody:%s", melodyNames[startupMelody]);
            const char* items[] = { 
              sound ? "Sound:ON" : "Sound:OFF",
              melodyStr
            };
            int itemCount = 2;
            if (menuIndex >= itemCount) menuIndex = 0;
            drawMenu("Settings", items, itemCount, menuIndex);

            if (enter) {
                if (menuIndex == 0) {
                    sound = !sound;
                    saveSoundSetting();
                } else if (menuIndex == 1) {
                    startupMelody = (startupMelody + 1) % 3;
                    snprintf(melodyStr, sizeof(melodyStr), "Melody:%s", melodyNames[startupMelody]);
                    drawMenu("Settings", items, itemCount, menuIndex);
                    saveMelodySetting();
                    playMelody(startupMelody);
                }
                delay(200);
            }
            if (back) {
                if (sound) tone(BUZZER_PIN, BACK_TONE, 50);
                menuState = MENU_MAIN;
                menuIndex = 0;
                delay(200);
            }
        }

        delay(50);
    }
}
  