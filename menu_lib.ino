extern MenuDef* currentMenu;

int menuIndex = 0;
bool menuNeedsRedraw = true;
bool menuNeedsFullRedraw = true;

void switchMenu(MenuDef* menuDef) {
    // Запоминаем текущее состояние меню
    currentMenu->currentItemIndex = menuIndex;
    // Переключаемся на новое меню
    currentMenu = menuDef;
    menuIndex = 0;
    menuNeedsRedraw = true;
    menuNeedsFullRedraw = true;
}

// =============================================================================
// ОТРИСОВКА И СКРОЛЛ
// =============================================================================

const int MENU_VISIBLE_ITEMS = 6;
const int MENU_HEADER_HEIGHT = 14;
const int MENU_ITEM_Y = 16;
const int MENU_ITEM_HEIGHT = 10;

int menuScrollTop = 0;
int menuScrollBottom = 0;
const MenuDef* lastDrawnMenu = nullptr;
int lastDrawnMenuIndex = -1;
int lastDrawnScrollTop = -1;
int lastDrawnScrollBottom = -1;

void drawBatteryIcon() {
    constexpr int x = SCREEN_WIDTH - 18;
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

void drawMenuHeader(const MenuDef* menuDef) {
    display.fillRect(0, 0, SCREEN_WIDTH, MENU_HEADER_HEIGHT, SSD1306_BLACK);
    display.setTextSize(1);
    display.setTextColor(UI_HEADER_COLOR);
    display.setCursor(8, 3);
    display.println(menuDef->title);
    display.drawLine(0, 13, SCREEN_WIDTH - 1, 13, UI_SEPARATOR_COLOR);
}

void drawMenuRow(const MenuDef* menuDef, int visibleSlot) {
    int y = MENU_ITEM_Y + visibleSlot * MENU_ITEM_HEIGHT;
    int itemIdx = menuScrollTop + visibleSlot;

    display.fillRect(0, y, SCREEN_WIDTH, MENU_ITEM_HEIGHT, SSD1306_BLACK);
    if (itemIdx >= menuDef->itemCount) return;

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, y);
    if (itemIdx == menuIndex) display.print(">");
    display.setCursor(8, y);

    const MenuItemDef* item = &menuDef->items[itemIdx];
    display.print(item->label);
    if (item->getValue) {
        display.print(":");
        display.print(item->getValue());
    }
}

void drawVisibleMenuRows(const MenuDef* menuDef) {
    for (int i = 0; i < MENU_VISIBLE_ITEMS; i++) {
        drawMenuRow(menuDef, i);
    }
}

void drawMenu(const MenuDef* menuDef) {
    bool scrollChanged = menuScrollTop != lastDrawnScrollTop
        || menuScrollBottom != lastDrawnScrollBottom;
    bool fullRedraw = menuNeedsFullRedraw
        || menuDef != lastDrawnMenu
        || scrollChanged;

    if (fullRedraw) {
        drawMenuHeader(menuDef);
        display.fillRect(0, MENU_HEADER_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - MENU_HEADER_HEIGHT, SSD1306_BLACK);
        drawVisibleMenuRows(menuDef);
    } else {
        int previousSlot = lastDrawnMenuIndex - menuScrollTop;
        if (previousSlot >= 0 && previousSlot < MENU_VISIBLE_ITEMS) {
            drawMenuRow(menuDef, previousSlot);
        }

        int currentSlot = menuIndex - menuScrollTop;
        if (currentSlot >= 0 && currentSlot < MENU_VISIBLE_ITEMS && currentSlot != previousSlot) {
            drawMenuRow(menuDef, currentSlot);
        }
    }

    drawBatteryIcon();
    display.display();

    lastDrawnMenu = menuDef;
    lastDrawnMenuIndex = menuIndex;
    lastDrawnScrollTop = menuScrollTop;
    lastDrawnScrollBottom = menuScrollBottom;
    menuNeedsFullRedraw = false;
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
    unsigned long lastBatteryRedraw = 0;

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
            menuNeedsRedraw = true;
        }

        int itemCount = currentMenu->itemCount;
        if (menuIndex < 0) menuIndex = itemCount - 1;
        if (menuIndex >= itemCount) menuIndex = 0;
        updateMenuScroll(itemCount, menuIndex);

        unsigned long now = millis();
        if (menuNeedsRedraw) {
            drawMenu(currentMenu);
            menuNeedsRedraw = false;
            lastBatteryRedraw = now;
        } else if (now - lastBatteryRedraw > 1000) {
            drawBatteryIcon();
            display.display();
            lastBatteryRedraw = now;
        }

        if (enter) {
            if (sound) tone(BUZZER_PIN, OK_TONE, 50);
            const MenuItemDef* item = &(currentMenu->items[menuIndex]);
            if (item->action) item->action();
            menuNeedsRedraw = true;
            menuNeedsFullRedraw = true;
            delay(200);
        }

        // --- Back ---
        if (back && currentMenu->parent) {
            if (sound) tone(BUZZER_PIN, BACK_TONE, 50);
            currentMenu->currentItemIndex = 0;
            menuIndex = currentMenu->parent->currentItemIndex;
            currentMenu = currentMenu->parent;
            menuNeedsRedraw = true;
            menuNeedsFullRedraw = true;
            delay(200);
        }

        delay(50);
    }
}
