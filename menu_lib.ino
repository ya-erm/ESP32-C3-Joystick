extern MenuDef* currentMenu;

int menuIndex = 0;

void switchMenu(MenuDef* menuDef) {
    // Запоминаем текущее состояние меню
    currentMenu->currentItemIndex = menuIndex;
    // Переключаемся на новое меню
    currentMenu = menuDef;
    menuIndex = 0;
}

// =============================================================================
// ОТРИСОВКА И СКРОЛЛ
// =============================================================================

const int MENU_VISIBLE_ITEMS = 5;

int menuScrollTop = 0;
int menuScrollBottom = 0;

void drawBatteryIcon() {
    constexpr int x = 111;
    constexpr int y = 2;

    // Освобождаем правую часть заголовка под индикатор.
    display.fillRect(x - 1, 0, SCREEN_WIDTH - x + 1, 13, SSD1306_BLACK);

    // Корпус 15x9 и положительный контакт справа.
    display.drawRect(x, y, 15, 9, SSD1306_WHITE);
    display.drawFastVLine(x + 15, y + 2, 5, SSD1306_WHITE);

    uint8_t segments = readBatterySegments();
    for (uint8_t i = 0; i < segments; i++) {
        display.fillRect(x + 2 + i * 3, y + 2, 2, 5, SSD1306_WHITE);
    }
}

void drawMenu(const MenuDef* menuDef) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(8, 3);
    display.println(menuDef->title);
    display.drawLine(0, 13, 127, 13, SSD1306_WHITE);

    for (int i = 0; i < MENU_VISIBLE_ITEMS; i++) {
        int itemIdx = menuScrollTop + i;
        if (itemIdx >= menuDef->itemCount) break;
        display.setCursor(0, 16 + i * 10);
        if (itemIdx == menuIndex) display.print(">");
        display.setCursor(8, 16 + i * 10);
        const MenuItemDef* item = &menuDef->items[itemIdx];
        display.print(item->label);
        if (item->getValue) {
            display.print(":");
            display.println(item->getValue());
        } else {
            display.println();
        }
    }
    drawBatteryIcon();
    display.display();
}

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


// =============================================================================
// ОТОБРАЖЕНИЕ И НАВИГАЦИЯ ПО МЕНЮ
// =============================================================================

void runMenu() {
    unsigned long lastInput = millis();

    while (true) {
        JoystickData input = readJoysticks();
        int moveY = 0;
        if (input.ly > 50 || input.ry > 50) moveY = -1;
        else if (input.ly < -50 || input.ry < -50) moveY = 1;
        bool enter = input.rb || input.rx > 50;
        bool back = input.lb || input.rx < -50;

        static unsigned long lastMove = 0;
        if (millis() - lastMove > 250 && moveY != 0) {
            if (sound) tone(BUZZER_PIN, MOVE_TONE, 50);
            lastMove = millis();
            menuIndex += moveY;
        }

        int itemCount = currentMenu->itemCount;
        if (menuIndex < 0) menuIndex = itemCount - 1;
        if (menuIndex >= itemCount) menuIndex = 0;
        updateMenuScroll(itemCount, menuIndex);

        drawMenu(currentMenu);

        if (enter) {
            if (sound) tone(BUZZER_PIN, OK_TONE, 50);
            const MenuItemDef* item = &(currentMenu->items[menuIndex]);
            if (item->action) item->action();
            delay(200);
        }

        // --- Back ---
        if (back && currentMenu->parent) {
            if (sound) tone(BUZZER_PIN, BACK_TONE, 50);
            currentMenu->currentItemIndex = 0;
            menuIndex = currentMenu->parent->currentItemIndex;
            currentMenu = currentMenu->parent;
            delay(200);
        }

        delay(50);
    }
}
