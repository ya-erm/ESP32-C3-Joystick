extern MenuDef* currentMenu; 

// --- Главное меню ---
const MenuItemDef mainItems[] = {
    { "Car Control",  runCarMode, nullptr },
    { "Games",        runGamesMenu, nullptr },
    { "Music",        runMusicMenu, nullptr },
    { "Status",       printStatus, nullptr },
    { "Settings",     runSettingsMenu, nullptr }
};
MenuDef mainMenu = { "Menu", mainItems, 5, nullptr, 0 };

void initMenu() {
    currentMenu = &mainMenu;
}

// --- Игры ---
const MenuItemDef gamesItems[] = {
    { "Tennis",       playTennisGame, nullptr },
    { "Snake",        playSnakeGame, nullptr },
    { "Tetris",       playTetrisGame, nullptr }
};
MenuDef gamesMenu = { "Games", gamesItems, 3, &mainMenu, 0 };

// --- Музыка ---
const MenuItemDef musicItems[] = {
    { "Harry Potter", playHarryPotterMelody, nullptr },
    { "Star Wars",    playStarWarsMelody, nullptr },
    { "Pacman",       playPacmanMelody, nullptr },
    { "Super Mario",  playMarioMelody, nullptr },
    { "Tetris",       playTetrisMelody, nullptr }
};
MenuDef musicMenu = { "Music", musicItems, 5, &mainMenu, 0 };

// --- Настройки ---
const MenuItemDef settingsItems[] = {
    { "Wi-Fi",        runWiFiSettingsMenu, nullptr },
    { "Sound",        runSoundSettingsMenu, nullptr },
    { "Motor",        runMotorSettingsMenu, nullptr },
    { "Joystick DZ",  changeJoystickDeadzone, getJoyDzValue },
    { "Debug",        toggleDebugLogs, getDebugValue }
};
MenuDef settingsMenu = { "Settings", settingsItems, 5, &mainMenu, 0 };

// --- Настройки Wi-Fi ---
const MenuItemDef wifiSettingsItems[] = {
    { "Mode",         toggleWiFiMode, getWiFiModeValue },
    { "ID",           doNothing, getMacValue },
    { "SSID",         doNothing, getSSIDValue },
    { "IP",           doNothing, getIPValue },
};
MenuDef wifiSettingsMenu = { "Wi-Fi (in progress)", wifiSettingsItems, 4, &settingsMenu, 0 };

// --- Настройки звука---
const MenuItemDef soundSettingsItems[] = {
    { "Sound",        toggleSound, getSoundValue },
    { "Melody",       changeMelody, getMelodyValue },
};
MenuDef soundSettingsMenu = { "Sound Settings", soundSettingsItems, 2, &settingsMenu, 0 };

// --- Настройки моторов ---
const MenuItemDef motorSettingsItems[] = {
    { "Turn",         changeTurnSpeed, getTurnValue },
    { "Speed",        changeMotorSpeed, getSpeedValue },
    { "Motor DZ",     changeMotorDeadzone, getMotorDzValue }
};
MenuDef motorSettingsMenu = { "Motor Settings", motorSettingsItems, 3, &settingsMenu, 0 };

void doNothing() {}

// --- Функции для переключения меню ---
void runGamesMenu() {
    switchMenu(&gamesMenu);
}

void runMusicMenu() {
    switchMenu(&musicMenu);
}

void runSettingsMenu() {
    switchMenu(&settingsMenu);
}

void runWiFiSettingsMenu() {
    switchMenu(&wifiSettingsMenu);
}

void runSoundSettingsMenu() {
    switchMenu(&soundSettingsMenu);
}

void runMotorSettingsMenu() {
    switchMenu(&motorSettingsMenu);
}

// --- Звук ---
static const char* getSoundValue() {
    return sound ? "ON" : "OFF";
}
static void toggleSound() {
    sound = !sound;
    saveSoundSetting();
}

// --- Мелодия при запуске ---
static const char* getMelodyValue() {
   switch (startupMelody) {
    case 0: return "OFF";
    case 1: return "Mario";
    case 2: return "Nokia";
   }
}
static void changeMelody() {
    startupMelody = (startupMelody + 1) % 3;
    drawMenu(currentMenu);
    saveMelodySetting();
    playMelody(startupMelody);
}

// --- Скорость поворота ---
static const char* getTurnValue() {
    static char str[4];
    snprintf(str, sizeof(str), "%.1f", (double)turnSpeed);
    return str;
}
static void changeTurnSpeed() {
    turnSpeed += 0.1f;
    if (turnSpeed > 1.0f) turnSpeed = 0.2f;
    saveTurnSetting();
}

// --- Скорость моторов ---
static const char* getSpeedValue() {
    static char str[4];
    snprintf(str, sizeof(str), "%.1f", (double)motorSpeed);
    return str;
}
static void changeMotorSpeed() {
    motorSpeed += 0.1f;
    if (motorSpeed > 1.0f) motorSpeed = 0.3f;
    saveSpeedSetting();
}

// --- Мёртвая зона моторов ---
static const char* getMotorDzValue() {
    static char motorDzStr[4];
    snprintf(motorDzStr, sizeof(motorDzStr), "%d", motorDeadzone);
    return motorDzStr;
}
static void changeMotorDeadzone() {
    motorDeadzone += 5;
    if (motorDeadzone > 30) motorDeadzone = 0;
    saveCarDeadzoneSetting();
}

// --- Мёртвая зона джойстика ---
static const char* getJoyDzValue() {
    static char joyDzStr[4];
    snprintf(joyDzStr, sizeof(joyDzStr), "%d", joystickDeadzone);
    return joyDzStr;
}
static void changeJoystickDeadzone() {
    joystickDeadzone += 10;
    if (joystickDeadzone > 100) joystickDeadzone = 0;
    saveJoystickDeadzoneSetting();
}

// --- Логирование отладочной информации ---
static const char* getDebugValue() {
    return debug ? "ON" : "OFF";
}
static void toggleDebugLogs() {
    debug = !debug;
    savedebugSetting();
}

// --- Режим Wi-Fi ---
static const char* getWiFiModeValue() {
    return WiFi.getMode() == WIFI_MODE_STA ? "STA" : "AP";
}
static void toggleWiFiMode() {
    WiFi.mode(WiFi.getMode() == WIFI_MODE_STA ? WIFI_MODE_AP : WIFI_MODE_STA);
}

// --- MAC Wi-Fi ---
static const char* getMacValue() {
    return WiFi.macAddress().c_str();
}

// --- SSID Wi-Fi ---
static const char* getSSIDValue() {
    return WiFi.SSID().c_str();
}

// --- IP Wi-Fi ---
static const char* getIPValue() {
    return WiFi.localIP().toString().c_str();
}