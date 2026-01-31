// === Калибровка центров (примерные значения, можно измерить и подставить) ===
#define LX_CENTER 680
#define LY_CENTER 680
#define RX_CENTER 680
#define RY_CENTER 680

// === Инверсия осей (если направление перепутано — ставим true) ===
#define LX_INVERT true
#define LY_INVERT false
#define RX_INVERT false
#define RY_INVERT true

// Порог мёртвой зоны задаётся в настройках (joystickDeadzone)
extern int joystickDeadzone;

// === Обработка значения оси ===
int processAxis(int raw, int center, bool invert, int amplitude = 100) {
  int value = raw - center;
  if (invert) value = -value;
  if (abs(value) < joystickDeadzone) value = 0; // подавляем шум
  int scaledValue = map(value, -center, center, -amplitude, amplitude);
  return scaledValue;
}

char getHDir(int x) {
  if (abs(x) < joystickDeadzone) return '-';
  return x > 0 ? 'R' : 'L';
}

char getVDir(int y) {
  if (abs(y) < joystickDeadzone) return '-';
  return y > 0 ? 'U' : 'D';
}

// === Считывание всех значений джойстика ===
JoystickData readJoysticks() {
  JoystickData data;
  // считываем сырые значения
  data.lx_raw = analogRead(LX_PIN);
  data.ly_raw = analogRead(LY_PIN);
  data.rx_raw = analogRead(RX_PIN);
  data.ry_raw = analogRead(RY_PIN);
  data.lb = digitalRead(LB_PIN) == LOW;
  data.rb = digitalRead(RB_PIN) == LOW;
  // обрабатываем значения
  data.lx = processAxis(data.lx_raw, LX_CENTER, LX_INVERT);
  data.ly = processAxis(data.ly_raw, LY_CENTER, LY_INVERT);
  data.rx = processAxis(data.rx_raw, RX_CENTER, RX_INVERT);
  data.ry = processAxis(data.ry_raw, RY_CENTER, RY_INVERT);
  // вычисляем направления
  data.lhd = getHDir(data.lx);
  data.lvd = getVDir(data.ly);
  data.rhd = getHDir(data.rx);
  data.rvd = getVDir(data.ry);
  return data;
}

bool readLeftButton() {
  return digitalRead(LB_PIN) == LOW;
}

bool readRightButton() {
  return digitalRead(RB_PIN) == LOW;
}