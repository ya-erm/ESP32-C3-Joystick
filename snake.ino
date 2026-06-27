extern bool sound;

void playSnakeGame() {
  const int screenW = SCREEN_WIDTH;
  const int screenH = SCREEN_HEIGHT;
  const int borderWidth = 1;
  const int cellSize = 5;
  const int segmentSize = 4;
  const int gameAreaX = 0;
  const int gameAreaY = 0;
  const int gameAreaW = screenW;
  const int gameAreaH = screenH;
  const int playX = gameAreaX + borderWidth;
  const int playY = gameAreaY + borderWidth;
  const int cellsX = (gameAreaW - 2 * borderWidth) / cellSize;
  const int cellsY = (gameAreaH - 2 * borderWidth) / cellSize;
  const int scoreY = 4;
  const int scoreClearW = 30;
  const int scoreClearH = 8;
  const int scoreClearX = (screenW - scoreClearW) / 2;
  const uint16_t borderColor = UI_SEPARATOR_COLOR;
  const uint16_t scoreColor = ST77XX_YELLOW;
  const uint16_t snakeColor = ST77XX_GREEN;
  const uint16_t foodColor = ST77XX_RED;

  int snakeX[500];
  int snakeY[500];
  int snakeLength = 4;
  int dirX = 1;
  int dirY = 0;

  int foodX = 0;
  int foodY = 0;
  int score = 0;
  unsigned long lastMove = 0;
  int speed = 200;
  bool gameOver = false;

  auto rectsOverlap = [](int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
  };

  auto cellPixelX = [&](int cellX) { return playX + cellX * cellSize; };
  auto cellPixelY = [&](int cellY) { return playY + cellY * cellSize; };

  auto clearCell = [&](int cellX, int cellY) {
    display.fillRect(cellPixelX(cellX), cellPixelY(cellY), segmentSize, segmentSize, SSD1306_BLACK);
  };

  auto drawFood = [&]() {
    display.fillRect(cellPixelX(foodX), cellPixelY(foodY), segmentSize, segmentSize, foodColor);
  };

  auto drawSnakeSegment = [&](int index) {
    display.fillRect(cellPixelX(snakeX[index]), cellPixelY(snakeY[index]), segmentSize, segmentSize, snakeColor);
  };

  auto drawScore = [&]() {
    char scoreText[8];
    snprintf(scoreText, sizeof(scoreText), "%d", score);
    int scoreX = (screenW - strlen(scoreText) * 6) / 2;

    display.fillRect(scoreClearX, scoreY, scoreClearW, scoreClearH, SSD1306_BLACK);
    if (rectsOverlap(cellPixelX(foodX), cellPixelY(foodY), segmentSize, segmentSize,
                     scoreClearX, scoreY, scoreClearW, scoreClearH)) {
      drawFood();
    }
    for (int i = snakeLength - 1; i >= 0; i--) {
      if (rectsOverlap(cellPixelX(snakeX[i]), cellPixelY(snakeY[i]), segmentSize, segmentSize,
                       scoreClearX, scoreY, scoreClearW, scoreClearH)) {
        drawSnakeSegment(i);
      }
    }

    display.setTextSize(1);
    display.setTextColor(scoreColor);
    display.setCursor(scoreX, scoreY);
    display.print(scoreText);
    display.setTextColor(SSD1306_WHITE);
  };

  auto spawnFood = [&]() {
    bool onSnake;
    do {
      onSnake = false;
      foodX = random(cellsX);
      foodY = random(cellsY);
      for (int i = 0; i < snakeLength; i++) {
        if (foodX == snakeX[i] && foodY == snakeY[i]) {
          onSnake = true;
          break;
        }
      }
    } while (onSnake);
  };

  auto drawFullGame = [&]() {
    display.clearDisplay();
    display.drawRect(gameAreaX, gameAreaY, gameAreaW, gameAreaH, borderColor);
    drawFood();
    for (int i = snakeLength - 1; i >= 0; i--) {
      drawSnakeSegment(i);
    }
    drawScore();
    display.display();
  };

  auto drawPause = [&]() {
    const int boxW = 78;
    const int boxH = 22;
    const int boxX = (screenW - boxW) / 2;
    const int boxY = (screenH - boxH) / 2;
    display.fillRect(boxX, boxY, boxW, boxH, SSD1306_BLACK);
    display.drawRect(boxX, boxY, boxW, boxH, borderColor);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor((screenW - 30) / 2, (screenH - 8) / 2);
    display.print("Pause");
    display.setTextColor(SSD1306_WHITE);
    display.display();
  };

  auto waitForRbRelease = [&]() {
    while (readJoysticks().rb) {
      delay(20);
    }
  };

  auto runPause = [&]() {
    drawPause();
    waitForRbRelease();
    while (true) {
      JoystickData pauseInput = readJoysticks();
      if (pauseInput.lb) {
        noTone(BUZZER_PIN);
        display.clearDisplay();
        return true;
      }
      if (pauseInput.rb) {
        waitForRbRelease();
        drawFullGame();
        lastMove = millis();
        return false;
      }
      delay(50);
    }
  };

  auto resetGame = [&]() {
    snakeLength = 4;
    dirX = 1;
    dirY = 0;
    for (int i = 0; i < snakeLength; i++) {
      snakeX[i] = cellsX / 2 - i;
      snakeY[i] = cellsY / 2;
    }
    score = 0;
    speed = 200;
    lastMove = millis();
    gameOver = false;
    spawnFood();

    display.clearDisplay();
    display.drawRect(gameAreaX, gameAreaY, gameAreaW, gameAreaH, borderColor);
    drawFood();
    for (int i = snakeLength - 1; i >= 0; i--) {
      drawSnakeSegment(i);
    }
    drawScore();
    display.display();
  };

  auto drawGameOver = [&]() {
    display.fillRect((screenW - 78) / 2, (screenH - 22) / 2, 78, 22, SSD1306_BLACK);
    display.drawRect((screenW - 78) / 2, (screenH - 22) / 2, 78, 22, SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor((screenW - 54) / 2, (screenH - 8) / 2);
    display.print("Game Over");
    drawScore();
    display.display();
  };

  resetGame();

  while (true) {
    if (gameOver) {
      drawGameOver();
      while (true) {
        JoystickData input = readJoysticks();
        if (input.rb) {
          resetGame();
          break;
        }
        if (input.lb) {
          noTone(BUZZER_PIN);
          display.clearDisplay();
          return;
        }
        delay(50);
      }
      continue;
    }

    JoystickData input = readJoysticks();

    if (input.lb && input.rb) {
      noTone(BUZZER_PIN);
      display.clearDisplay();
      return;
    }
    if (input.rb) {
      if (runPause()) return;
      continue;
    }

    int moveX = 0, moveY = 0;
    if (input.lx > 50 || input.rx > 50) moveX = 1;
    else if (input.lx < -50 || input.rx < -50) moveX = -1;
    else if (input.ly > 50 || input.ry > 50) moveY = -1;
    else if (input.ly < -50 || input.ry < -50) moveY = 1;

    if (moveX != 0 && dirX == 0) { dirX = moveX; dirY = 0; }
    else if (moveY != 0 && dirY == 0) { dirY = moveY; dirX = 0; }

    if (millis() - lastMove > speed) {
      lastMove = millis();

      int oldTailX = snakeX[snakeLength - 1];
      int oldTailY = snakeY[snakeLength - 1];

      for (int i = snakeLength - 1; i > 0; i--) {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
      }

      snakeX[0] += dirX;
      snakeY[0] += dirY;

      if (snakeX[0] < 0 || snakeX[0] >= cellsX ||
          snakeY[0] < 0 || snakeY[0] >= cellsY) {
        if (sound) tone(BUZZER_PIN, 200, 400);
        gameOver = true;
        continue;
      }

      bool hitSelf = false;
      for (int i = 1; i < snakeLength; i++) {
        if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
          hitSelf = true;
          break;
        }
      }
      if (hitSelf) {
        if (sound) tone(BUZZER_PIN, 200, 400);
        gameOver = true;
        continue;
      }

      bool ateFood = snakeX[0] == foodX && snakeY[0] == foodY;
      if (ateFood) {
        if (sound) tone(BUZZER_PIN, 1000, 80);
        if (snakeLength < 500) {
          snakeX[snakeLength] = oldTailX;
          snakeY[snakeLength] = oldTailY;
          snakeLength++;
        }
        score++;
        if (speed > 60) speed -= 5;
        spawnFood();
      } else {
        clearCell(oldTailX, oldTailY);
      }

      drawFood();
      drawSnakeSegment(0);
      drawScore();
      display.drawRect(gameAreaX, gameAreaY, gameAreaW, gameAreaH, borderColor);
      display.display();
    }
  }
}
