
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
int mainMenuIndex = 0;
int gamesMenuIndex = 0;
int musicMenuIndex = 0;
int settingsMenuIndex = 0;

const int MENU_VISIBLE_ITEMS = 5;

// Верхняя и нижняя граница видимого окна (индексы элементов на экране)
int menuScrollTop = 0;
int menuScrollBottom = 0;

void drawMenu(const char* title, const char* items[], int itemCount, int index, int scrollTop) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(8, 3);
    display.println(title);
    display.drawLine(0, 13, 127, 13, SSD1306_WHITE);

    for (int i = 0; i < MENU_VISIBLE_ITEMS; i++) {
        int itemIdx = scrollTop + i;
        if (itemIdx >= itemCount) break;
        display.setCursor(0, 16 + i * 10);
        if (itemIdx == index) {
            display.print(">");
        }
        display.setCursor(8, 16 + i * 10);
        display.println(items[itemIdx]);
    }

    display.display();
}

// Обновляет menuScrollTop/menuScrollBottom: курсор не должен выходить за границы окна
void updateMenuScroll(int itemCount, int index) {
    if (itemCount <= MENU_VISIBLE_ITEMS) {
        menuScrollTop = 0;
        menuScrollBottom = itemCount - 1;
        return;
    }
    menuScrollBottom = min(itemCount - 1, menuScrollTop + MENU_VISIBLE_ITEMS - 1);
    if (index < menuScrollTop) {
        menuScrollTop = index;
        menuScrollBottom = min(itemCount - 1, menuScrollTop + MENU_VISIBLE_ITEMS - 1);
    } else if (index > menuScrollBottom) {
        menuScrollTop = max(0, index - MENU_VISIBLE_ITEMS + 1);
        menuScrollBottom = min(itemCount - 1, menuScrollTop + MENU_VISIBLE_ITEMS - 1);
    }
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
        }

        // === Обработка по текущему меню ===
        if (menuState == MENU_MAIN) {
            const char* items[] = { 
              "Car Control",
              "Games",
              "Music",
              "Status",
              "Settings"
            };
            int itemCount = 5;
            if (menuIndex < 0) menuIndex = itemCount - 1;
            if (menuIndex >= itemCount) menuIndex = 0;
            updateMenuScroll(itemCount, menuIndex);
            drawMenu("Menu", items, itemCount, menuIndex, menuScrollTop);

            if (enter) {
                if (sound) tone(BUZZER_PIN, OK_TONE, 50);
                mainMenuIndex = menuIndex;
                if (menuIndex == 0) runCarMode();
                else if (menuIndex == 1) { menuState = MENU_GAMES; menuIndex = gamesMenuIndex; }
                else if (menuIndex == 2) { menuState = MENU_MUSIC; menuIndex = musicMenuIndex; }
                else if (menuIndex == 3) printStatus();
                else if (menuIndex == 4) { menuState = MENU_SETTINGS; menuIndex = settingsMenuIndex; }
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
            if (menuIndex < 0) menuIndex = itemCount - 1;
            if (menuIndex >= itemCount) menuIndex = 0;
            updateMenuScroll(itemCount, menuIndex);
            drawMenu("Games", items, itemCount, menuIndex, menuScrollTop);

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
                gamesMenuIndex = menuIndex;
                menuState = MENU_MAIN; 
                menuIndex = mainMenuIndex;
                delay(200); 
            }
        }

        else if (menuState == MENU_MUSIC) {
            const char* melodyNames[] = { "Harry Potter", "Star Wars", "Pacman", "Super Mario", "Tetris" };
            int itemCount = 5;
            if (menuIndex < 0) menuIndex = itemCount - 1;
            if (menuIndex >= itemCount) menuIndex = 0;
            updateMenuScroll(itemCount, menuIndex);
            drawMenu("Music", melodyNames, itemCount, menuIndex, menuScrollTop);

            if (enter) {
                // if (sound) tone(BUZZER_PIN, OK_TONE, 50);
                // startupMelody = menuIndex;
                // saveMelodySetting();

                // Проигрываем выбранную мелодию
                playMusic(menuIndex);
            }
            if (back) {
                if (sound) tone(BUZZER_PIN, BACK_TONE, 50);
                musicMenuIndex = menuIndex;
                menuState = MENU_MAIN;
                menuIndex = mainMenuIndex;
                delay(200);
            }
        }

        else if (menuState == MENU_SETTINGS) {
            const char* melodyNames[] = { "OFF", "Mario", "Nokia" };
            static char melodyStr[20];
            static char turnStr[24];
            static char speedStr[24];
            static char deadzoneStr[24];
            static char joyDzStr[24];
            snprintf(melodyStr, sizeof(melodyStr), "Melody:%s", melodyNames[startupMelody]);
            snprintf(turnStr, sizeof(turnStr), "Turn:%.1f", (double)turnSpeed);
            snprintf(speedStr, sizeof(speedStr), "Speed:%.1f", (double)motorSpeed);
            snprintf(deadzoneStr, sizeof(deadzoneStr), "Motor DZ:%d", motorDeadzone);
            snprintf(joyDzStr, sizeof(joyDzStr), "Joystick DZ:%d", joystickDeadzone);
            const char* items[] = { 
              sound ? "Sound:ON" : "Sound:OFF",
              melodyStr,
              turnStr,
              speedStr,
              deadzoneStr,
              joyDzStr,
              debug ? "Debug:ON" : "Debug:OFF"
            };
            int itemCount = 7;
            if (menuIndex < 0) menuIndex = itemCount - 1;
            if (menuIndex >= itemCount) menuIndex = 0;
            updateMenuScroll(itemCount, menuIndex);
            drawMenu("Settings", items, itemCount, menuIndex, menuScrollTop);

            if (enter) {
                if (menuIndex == 0) {
                    sound = !sound;
                    saveSoundSetting();
                } else if (menuIndex == 1) {
                    startupMelody = (startupMelody + 1) % 3;
                    snprintf(melodyStr, sizeof(melodyStr), "Melody:%s", melodyNames[startupMelody]);
                    drawMenu("Settings", items, itemCount, menuIndex, menuScrollTop);
                    saveMelodySetting();
                    playMelody(startupMelody);
                } else if (menuIndex == 2) {
                    turnSpeed += 0.1f;
                    if (turnSpeed > 1.0f) turnSpeed = 0.2f;
                    snprintf(turnStr, sizeof(turnStr), "Turn:%.1f", (double)turnSpeed);
                    drawMenu("Settings", items, itemCount, menuIndex, menuScrollTop);
                    saveTurnSetting();
                } else if (menuIndex == 3) {
                    motorSpeed += 0.1f;
                    if (motorSpeed > 1.0f) motorSpeed = 0.3f;
                    snprintf(speedStr, sizeof(speedStr), "Speed:%.1f", (double)motorSpeed);
                    drawMenu("Settings", items, itemCount, menuIndex, menuScrollTop);
                    saveSpeedSetting();
                } else if (menuIndex == 4) {
                    motorDeadzone += 5;
                    if (motorDeadzone > 30) motorDeadzone = 0;
                    snprintf(deadzoneStr, sizeof(deadzoneStr), "Motor DZ:%d", motorDeadzone);
                    drawMenu("Settings", items, itemCount, menuIndex, menuScrollTop);
                    saveCarDeadzoneSetting();
                } else if (menuIndex == 5) {
                    joystickDeadzone += 10;
                    if (joystickDeadzone > 100) joystickDeadzone = 0;
                    snprintf(joyDzStr, sizeof(joyDzStr), "Joystick DZ:%d", joystickDeadzone);
                    drawMenu("Settings", items, itemCount, menuIndex, menuScrollTop);
                    saveJoystickDeadzoneSetting();
                } else if (menuIndex == 6) {
                    debug = !debug;
                    savedebugSetting();
                }
                delay(200);
            }
            if (back) {
                if (sound) tone(BUZZER_PIN, BACK_TONE, 50);
                settingsMenuIndex = menuIndex;
                menuState = MENU_MAIN;
                menuIndex = mainMenuIndex;
                delay(200);
            }
        }

        delay(50);
    }
}
  