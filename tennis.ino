extern bool sound;

void playTennisGame() {
  // === Параметры поля ===
  const int screenW = 128;
  const int screenH = 64;
  const int paddleH = 14;
  const int paddleW = 2;
  const int ballSize = 3;

  // === Начальные позиции ===
  int leftY = screenH / 2 - paddleH / 2;
  int rightY = screenH / 2 - paddleH / 2;
  int ballX = screenW / 2;
  int ballY = screenH / 2;

  // === Скорость мяча ===
  int ballDX = 2;
  int ballDY = 1;

  int scoreL = 0;
  int scoreR = 0;

  unsigned long lastUpdate = millis();
  const int frameDelay = 30; // скорость обновления (FPS ~33)

  while (true) {
    // --- Чтение джойстиков ---
    JoystickData input = readJoysticks();
    
    // --- Выход из игры при нажатии обеих кнопок ---
    if (input.lb && input.rb) {
      noTone(BUZZER_PIN);
      return;
    }

    if (millis() - lastUpdate > frameDelay) {
      lastUpdate = millis();

      // --- Обновление позиций платформ ---
      leftY -= input.ly / 20;
      rightY -= input.ry / 20;

      // --- Границы ---
      if (leftY < 0) leftY = 0;
      if (leftY > screenH - paddleH) leftY = screenH - paddleH;
      if (rightY < 0) rightY = 0;
      if (rightY > screenH - paddleH) rightY = screenH - paddleH;
      
      // --- Обновление позиции мяча ---
      ballX += ballDX;
      ballY += ballDY;

      // --- Отскоки от верхней/нижней границы ---
      if (ballY <= 0 || ballY >= screenH - ballSize) {
        ballDY = -ballDY;
        if (sound) tone(BUZZER_PIN, 1500, 50);
      }

      // --- Проверка столкновений с платформами ---
      if (ballX <= paddleW + 2 &&
          ballY + ballSize >= leftY &&
          ballY <= leftY + paddleH) {
        ballDX = -ballDX;
        ballX = paddleW + 3;
        if (sound) tone(BUZZER_PIN, 800, 50);
      }

      if (ballX + ballSize >= screenW - paddleW - 2 &&
          ballY + ballSize >= rightY &&
          ballY <= rightY + paddleH) {
        ballDX = -ballDX;
        ballX = screenW - paddleW - ballSize - 3;
        if (sound) tone(BUZZER_PIN, 1000, 50);
      }

      // --- Проверка на гол ---
      if (ballX < 0) {
        scoreR++;
        if (sound) tone(BUZZER_PIN, 300, 300);
        delay(400);
        ballX = screenW / 2;
        ballY = screenH / 2;
        ballDX = 2;
        ballDY = random(2) ? 1 : -1;
      } else if (ballX > screenW) {
        scoreL++;
        if (sound) tone(BUZZER_PIN, 1200, 300);
        delay(400);
        ballX = screenW / 2;
        ballY = screenH / 2;
        ballDX = -2;
        ballDY = random(2) ? 1 : -1;
      }

      // --- Рисуем всё ---
      display.clearDisplay();

      // Центр (разделительная линия)
      for (int y = 0; y < screenH; y += 4)
        display.drawFastVLine(screenW / 2, y, 2, SSD1306_WHITE);

      // Платформы
      display.fillRect(1, leftY, paddleW, paddleH, SSD1306_WHITE);
      display.fillRect(screenW - paddleW - 2, rightY, paddleW, paddleH, SSD1306_WHITE);

      // Мяч
      display.fillRect(ballX, ballY, ballSize, ballSize, SSD1306_WHITE);

      // Счёт
      display.setTextSize(1);
      display.setCursor((screenW / 2) - 2 - 12, 0);
      display.printf("%2d", scoreL);
      display.setCursor((screenW / 2) - 2 + 6, 0);
      display.printf("%d", scoreR);

      display.display();
    }
  }
}
