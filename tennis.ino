extern bool sound;

void playTennisGame() {
  const int screenW = SCREEN_WIDTH;
  const int screenH = SCREEN_HEIGHT;
  const int paddleH = 18;
  const int paddleW = 3;
  const int ballSize = 4;
  const int leftPaddleX = 2;
  const int rightPaddleX = screenW - paddleW - 3;
  const int centerX = screenW / 2;
  const int leftScoreX = centerX - 16;
  const int rightScoreX = centerX + 6;
  const int scoreY = 0;
  const int leftScoreW = 14;
  const int rightScoreW = 14;
  const int scoreH = 8;
  const uint16_t leftColor = ST77XX_BLUE;
  const uint16_t rightColor = ST77XX_RED;
  const uint16_t ballColor = ST77XX_GREEN;

  int leftY = screenH / 2 - paddleH / 2;
  int rightY = screenH / 2 - paddleH / 2;
  int ballX = screenW / 2;
  int ballY = screenH / 2;
  int prevScoreL = -1;
  int prevScoreR = -1;

  int ballDX = 2;
  int ballDY = 1;

  int scoreL = 0;
  int scoreR = 0;

  unsigned long lastUpdate = millis();
  const int frameDelay = 16;

  auto rectsOverlap = [](int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
  };

  auto drawCenterLine = [&]() {
    for (int y = 0; y < screenH; y += 4) {
      display.drawFastVLine(centerX, y, 2, SSD1306_WHITE);
    }
  };

  auto drawScore = [&](bool left) {
    display.setTextSize(1);
    if (left) {
      display.fillRect(leftScoreX, scoreY, leftScoreW, scoreH, SSD1306_BLACK);
      display.setTextColor(leftColor);
      display.setCursor(leftScoreX, scoreY);
      display.printf("%2d", scoreL);
      prevScoreL = scoreL;
    } else {
      display.fillRect(rightScoreX, scoreY, rightScoreW, scoreH, SSD1306_BLACK);
      display.setTextColor(rightColor);
      display.setCursor(rightScoreX, scoreY);
      display.printf("%d", scoreR);
      prevScoreR = scoreR;
    }
    display.setTextColor(SSD1306_WHITE);
  };

  auto drawPaddleDelta = [&](int x, int oldY, int newY, uint16_t color, bool forceFull) {
    if (forceFull) {
      int clearY = min(oldY, newY);
      int clearH = paddleH + abs(newY - oldY);
      display.fillRect(x, clearY, paddleW, clearH, SSD1306_BLACK);
      display.fillRect(x, newY, paddleW, paddleH, color);
      return;
    }
    if (newY == oldY) return;

    int delta = abs(newY - oldY);
    if (delta >= paddleH) {
      display.fillRect(x, oldY, paddleW, paddleH, SSD1306_BLACK);
      display.fillRect(x, newY, paddleW, paddleH, color);
      return;
    }

    if (newY > oldY) {
      display.fillRect(x, oldY, paddleW, delta, SSD1306_BLACK);
      display.fillRect(x, oldY + paddleH, paddleW, delta, color);
    } else {
      display.fillRect(x, oldY + paddleH - delta, paddleW, delta, SSD1306_BLACK);
      display.fillRect(x, newY, paddleW, delta, color);
    }
  };

  auto variedBallDy = [&](int paddleY, int direction) {
    int ballCenter = ballY + ballSize / 2;
    int paddleCenter = paddleY + paddleH / 2;
    int offset = ballCenter - paddleCenter;
    int dy = offset / 4 + random(-1, 2);
    if (dy == 0) dy = random(2) ? 1 : -1;
    dy = constrain(dy, -3, 3);
    return dy * direction;
  };

  auto drawStaticGame = [&]() {
    display.clearDisplay();
    drawCenterLine();
    display.fillRect(leftPaddleX, leftY, paddleW, paddleH, leftColor);
    display.fillRect(rightPaddleX, rightY, paddleW, paddleH, rightColor);
    drawScore(true);
    drawScore(false);
    display.display();
  };

  auto runCountdown = [&]() {
    const int boxW = 34;
    const int boxH = 22;
    const int boxX = (screenW - boxW) / 2;
    const int boxY = (screenH - boxH) / 2;
    for (int value = 3; value >= 1; value--) {
      drawStaticGame();
      display.fillRect(boxX, boxY, boxW, boxH, SSD1306_BLACK);
      display.drawRect(boxX, boxY, boxW, boxH, UI_SEPARATOR_COLOR);
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor((screenW - 12) / 2, boxY + 4);
      display.print(value);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.display();
      if (sound) tone(BUZZER_PIN, value == 1 ? 1400 : 900, 90);

      unsigned long start = millis();
      while (millis() - start < 650) {
        JoystickData countdownInput = readJoysticks();
        if (countdownInput.lb && countdownInput.rb) {
          noTone(BUZZER_PIN);
          display.clearDisplay();
          return true;
        }
        delay(20);
      }
    }
    drawStaticGame();
    return false;
  };

  randomSeed((uint32_t)micros() ^ analogRead(LX_PIN) ^ ((uint32_t)analogRead(RY_PIN) << 12));

  if (runCountdown()) return;
  lastUpdate = millis();
  display.fillRect(ballX, ballY, ballSize, ballSize, ballColor);
  display.display();

  while (true) {
    JoystickData input = readJoysticks();

    if (input.lb && input.rb) {
      noTone(BUZZER_PIN);
      display.clearDisplay();
      return;
    }

    if (millis() - lastUpdate >= frameDelay) {
      lastUpdate = millis();

      int oldLeftY = leftY;
      int oldRightY = rightY;
      int oldBallX = ballX;
      int oldBallY = ballY;

      leftY -= input.ly / 20;
      rightY -= input.ry / 20;

      if (leftY < 0) leftY = 0;
      if (leftY > screenH - paddleH) leftY = screenH - paddleH;
      if (rightY < 0) rightY = 0;
      if (rightY > screenH - paddleH) rightY = screenH - paddleH;

      ballX += ballDX;
      ballY += ballDY;

      if (ballY <= 0) {
        ballY = 0;
        ballDY = abs(ballDY);
        if (sound) tone(BUZZER_PIN, 1500, 50);
      } else if (ballY >= screenH - ballSize) {
        ballY = screenH - ballSize;
        ballDY = -abs(ballDY);
        if (sound) tone(BUZZER_PIN, 1500, 50);
      }

      if (ballX <= leftPaddleX + paddleW + 1 &&
          ballY + ballSize >= leftY &&
          ballY <= leftY + paddleH) {
        ballDX = abs(ballDX);
        ballDY = variedBallDy(leftY, 1);
        ballX = leftPaddleX + paddleW + 2;
        if (sound) tone(BUZZER_PIN, 800, 50);
      }

      if (ballX + ballSize >= rightPaddleX - 1 &&
          ballY + ballSize >= rightY &&
          ballY <= rightY + paddleH) {
        ballDX = -abs(ballDX);
        ballDY = variedBallDy(rightY, 1);
        ballX = rightPaddleX - ballSize - 2;
        if (sound) tone(BUZZER_PIN, 1000, 50);
      }

      if (ballX < 0) {
        scoreR++;
        if (sound) tone(BUZZER_PIN, 300, 300);
        delay(400);
        display.fillRect(oldBallX, oldBallY, ballSize, ballSize, SSD1306_BLACK);
        ballX = screenW / 2;
        ballY = screenH / 2;
        ballDX = 2;
        ballDY = random(2) ? 1 : -1;
        oldBallX = ballX;
        oldBallY = ballY;
      } else if (ballX > screenW) {
        scoreL++;
        if (sound) tone(BUZZER_PIN, 1200, 300);
        delay(400);
        display.fillRect(oldBallX, oldBallY, ballSize, ballSize, SSD1306_BLACK);
        ballX = screenW / 2;
        ballY = screenH / 2;
        ballDX = -2;
        ballDY = random(2) ? 1 : -1;
        oldBallX = ballX;
        oldBallY = ballY;
      }

      bool oldBallHitCenter = rectsOverlap(oldBallX, oldBallY, ballSize, ballSize, centerX, 0, 1, screenH);
      bool oldBallHitLeftScore = rectsOverlap(oldBallX, oldBallY, ballSize, ballSize, leftScoreX, scoreY, leftScoreW, scoreH);
      bool oldBallHitRightScore = rectsOverlap(oldBallX, oldBallY, ballSize, ballSize, rightScoreX, scoreY, rightScoreW, scoreH);
      bool newBallHitLeftScore = rectsOverlap(ballX, ballY, ballSize, ballSize, leftScoreX, scoreY, leftScoreW, scoreH);
      bool newBallHitRightScore = rectsOverlap(ballX, ballY, ballSize, ballSize, rightScoreX, scoreY, rightScoreW, scoreH);
      bool oldBallHitLeftPaddle = rectsOverlap(oldBallX, oldBallY, ballSize, ballSize, leftPaddleX, min(oldLeftY, leftY), paddleW, paddleH + abs(leftY - oldLeftY));
      bool oldBallHitRightPaddle = rectsOverlap(oldBallX, oldBallY, ballSize, ballSize, rightPaddleX, min(oldRightY, rightY), paddleW, paddleH + abs(rightY - oldRightY));

      display.fillRect(oldBallX, oldBallY, ballSize, ballSize, SSD1306_BLACK);

      if (oldBallHitCenter) drawCenterLine();
      drawPaddleDelta(leftPaddleX, oldLeftY, leftY, leftColor, oldBallHitLeftPaddle);
      drawPaddleDelta(rightPaddleX, oldRightY, rightY, rightColor, oldBallHitRightPaddle);

      display.fillRect(ballX, ballY, ballSize, ballSize, ballColor);

      bool scoreRedrawn = false;
      if (scoreL != prevScoreL || oldBallHitLeftScore || newBallHitLeftScore) {
        drawScore(true);
        scoreRedrawn = true;
      }
      if (scoreR != prevScoreR || oldBallHitRightScore || newBallHitRightScore) {
        drawScore(false);
        scoreRedrawn = true;
      }
      if (scoreRedrawn && oldBallHitCenter && !rectsOverlap(ballX, ballY, ballSize, ballSize, centerX, 0, 1, screenH)) {
        drawCenterLine();
      }

      display.display();
    }
  }
}
