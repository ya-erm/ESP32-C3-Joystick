extern bool sound;

void playArkanoidGame() {
  const int screenW = SCREEN_WIDTH;
  const int screenH = SCREEN_HEIGHT;
  const int wallTop = 0;
  const int footerH = 11;

  const int brickRows = 4;
  const int brickCols = 10;
  const int brickW = 14;
  const int brickH = 6;
  const int brickGap = 1;
  const int brickStartX = (screenW - (brickCols * brickW + (brickCols - 1) * brickGap)) / 2;
  const int brickStartY = wallTop + 5;

  const int paddleW = 28;
  const int paddleH = 4;
  const int paddleY = screenH - 7;
  const int ballSize = 4;
  const int startLives = 4;
  const int maxHearts = 3;
  const int heartW = 5;
  const int heartGap = 2;
  const int overlayPadX = 2;
  const int overlayBoxH = 9;
  const int overlayBoxY = 0;
  const int heartsY = overlayBoxY + 2;
  const int scoreTextY = overlayBoxY + 1;

  const uint16_t wallColor = UI_SEPARATOR_COLOR;
  const uint16_t paddleColor = ST77XX_CYAN;
  const uint16_t ballColor = ST77XX_GREEN;
  const uint16_t scoreColor = ST77XX_YELLOW;
  const uint16_t brickColors[brickRows] = {
    ST77XX_RED,
    ST77XX_ORANGE,
    ST77XX_YELLOW,
    ST77XX_BLUE
  };

  bool bricks[brickRows][brickCols];
  int bricksLeft = 0;
  int score = 0;
  int lives = startLives;
  int paddleX = (screenW - paddleW) / 2;
  float ballX = screenW / 2 - ballSize / 2;
  float ballY = paddleY - ballSize - 1;
  float ballVX = 1.0f;
  float ballVY = -1.05f;
  bool paused = false;
  bool gameOver = false;
  bool gameWon = false;
  bool rbWasPressed = true;
  bool overlayDrawn = false;
  int lastDrawnLives = -1;
  int lastDrawnScore = -1;
  int lastHeartsBoxX = 0;
  int lastHeartsBoxW = 0;
  int lastScoreBoxX = 0;
  int lastScoreBoxW = 0;
  unsigned long lastUpdate = millis();
  const int frameDelay = 18;

  auto rectsOverlap = [](int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
  };

  auto brickX = [&](int col) {
    return brickStartX + col * (brickW + brickGap);
  };

  auto brickY = [&](int row) {
    return brickStartY + row * (brickH + brickGap);
  };

  auto drawHeart = [&](int x, int y) {
    display.drawPixel(x + 1, y, ST77XX_RED);
    display.drawPixel(x + 3, y, ST77XX_RED);
    display.drawFastHLine(x, y + 1, 5, ST77XX_RED);
    display.drawFastHLine(x, y + 2, 5, ST77XX_RED);
    display.drawFastHLine(x + 1, y + 3, 3, ST77XX_RED);
    display.drawPixel(x + 2, y + 4, ST77XX_RED);
  };

  auto drawLives = [&]() {
    int visibleHearts = constrain(lives - 1, 0, maxHearts);
    int heartsW = visibleHearts * heartW + max(0, visibleHearts - 1) * heartGap;
    int startX = (screenW - heartsW) / 2;
    for (int i = 0; i < visibleHearts; i++) {
      drawHeart(startX + i * (heartW + heartGap), heartsY);
    }
  };

  auto drawBrick = [&](int row, int col) {
    int x = brickX(col);
    int y = brickY(row);
    display.fillRect(x, y, brickW, brickH, SSD1306_BLACK);
    if (bricks[row][col]) {
      display.fillRect(x, y, brickW - 1, brickH - 1, brickColors[row]);
      display.drawFastHLine(x, y, brickW - 1, SSD1306_WHITE);
    }
  };

  auto drawBricksInRect = [&](int x, int y, int w, int h) {
    for (int row = 0; row < brickRows; row++) {
      for (int col = 0; col < brickCols; col++) {
        if (rectsOverlap(x, y, w, h, brickX(col), brickY(row), brickW, brickH)) {
          drawBrick(row, col);
        }
      }
    }
  };

  auto drawAllBricks = [&]() {
    for (int row = 0; row < brickRows; row++) {
      for (int col = 0; col < brickCols; col++) {
        drawBrick(row, col);
      }
    }
  };

  auto drawPaddle = [&]() {
    display.fillRect(paddleX, paddleY, paddleW, paddleH, paddleColor);
    display.drawFastHLine(paddleX + 2, paddleY, paddleW - 4, SSD1306_WHITE);
  };

  auto drawBall = [&]() {
    display.fillRect((int)round(ballX), (int)round(ballY), ballSize, ballSize, ballColor);
  };

  auto redrawStaticInRect = [&](int x, int y, int w, int h) {
    drawBricksInRect(x, y, w, h);
    if (rectsOverlap(x, y, w, h, paddleX, paddleY, paddleW, paddleH)) drawPaddle();
  };

  auto currentHeartsRect = [&](int& x, int& y, int& w, int& h) {
    int visibleHearts = constrain(lives - 1, 0, maxHearts);
    if (visibleHearts <= 0) {
      x = screenW / 2;
      y = overlayBoxY;
      w = 0;
      h = 0;
      return;
    }

    int heartsW = visibleHearts * heartW + (visibleHearts - 1) * heartGap;
    w = heartsW + overlayPadX * 2;
    h = overlayBoxH;
    x = (screenW - w) / 2;
    y = overlayBoxY;
  };

  auto currentScoreRect = [&](int& x, int& y, int& w, int& h) {
    char scoreText[10];
    snprintf(scoreText, sizeof(scoreText), "%d", score);
    w = strlen(scoreText) * 6 + overlayPadX * 2;
    h = overlayBoxH;
    x = screenW - w;
    y = overlayBoxY;
  };

  auto restoreOverlayArea = [&](int x, int y, int w, int h) {
    if (w <= 0 || h <= 0) return;
    display.fillRect(x, y, w, h, SSD1306_BLACK);
    redrawStaticInRect(x, y, w, h);
    if (!gameOver && !gameWon &&
        rectsOverlap(x, y, w, h, (int)round(ballX), (int)round(ballY), ballSize, ballSize)) {
      drawBall();
    }
  };

  auto drawLivesOverlay = [&](bool force = false) {
    int x;
    int y;
    int w;
    int h;
    currentHeartsRect(x, y, w, h);

    bool changed = lives != lastDrawnLives;
    if (!force && !changed) return;

    if (changed) {
      restoreOverlayArea(lastHeartsBoxX, overlayBoxY, lastHeartsBoxW, overlayBoxH);
    }
    if (w > 0) {
      display.fillRect(x, y, w, h, SSD1306_BLACK);
      drawLives();
    }

    lastDrawnLives = lives;
    lastHeartsBoxX = x;
    lastHeartsBoxW = w;
  };

  auto drawScoreOverlay = [&](bool force = false) {
    int x;
    int y;
    int w;
    int h;
    currentScoreRect(x, y, w, h);

    bool changed = score != lastDrawnScore;
    if (!force && !changed) return;

    if (changed) {
      restoreOverlayArea(lastScoreBoxX, overlayBoxY, lastScoreBoxW, overlayBoxH);
    }

    char scoreText[10];
    snprintf(scoreText, sizeof(scoreText), "%d", score);
    display.fillRect(x, y, w, h, SSD1306_BLACK);
    display.setTextSize(1);
    display.setTextColor(scoreColor);
    display.setCursor(x + overlayPadX, scoreTextY);
    display.print(scoreText);
    display.setTextColor(SSD1306_WHITE);

    lastDrawnScore = score;
    lastScoreBoxX = x;
    lastScoreBoxW = w;
  };

  auto drawStatusOverlays = [&](bool forceHearts = false, bool forceScore = false) {
    drawLivesOverlay(forceHearts);
    drawScoreOverlay(forceScore);
  };

  auto drawFullGame = [&]() {
    display.clearDisplay();
    drawAllBricks();
    drawPaddle();
    drawBall();
    drawStatusOverlays(true, true);
    display.display();
  };

  auto resetRound = [&]() {
    paddleX = (screenW - paddleW) / 2;
    ballX = screenW / 2 - ballSize / 2;
    ballY = paddleY - ballSize - 1;
    ballVX = random(2) ? 1.0f : -1.0f;
    ballVY = -1.05f;
    lastUpdate = millis();
  };

  auto drawFooter = [&](const char* leftText, const char* rightText) {
    int footerY = screenH - footerH;
    display.fillRect(0, footerY, screenW, footerH, SSD1306_BLACK);
    display.drawFastHLine(0, footerY, screenW, wallColor);
    display.setTextSize(1);
    display.setTextColor(UI_FOOTER_COLOR);
    display.setCursor(0, footerY + 2);
    display.print(leftText);
    int rightX = screenW - strlen(rightText) * 6;
    display.setCursor(max(0, rightX), footerY + 2);
    display.print(rightText);
    display.setTextColor(SSD1306_WHITE);
  };

  auto drawOverlay = [&](const char* title, const char* leftText, const char* rightText) {
    const int boxW = 72;
    const int boxH = 22;
    const int boxX = (screenW - boxW) / 2;
    const int boxY = (screenH - footerH - boxH) / 2;
    display.fillRect(boxX, boxY, boxW, boxH, SSD1306_BLACK);
    display.drawRect(boxX, boxY, boxW, boxH, wallColor);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(boxX + (boxW - strlen(title) * 6) / 2, boxY + 7);
    display.print(title);
    drawFooter(leftText, rightText);
    display.display();
  };

  auto resetGame = [&]() {
    for (int row = 0; row < brickRows; row++) {
      for (int col = 0; col < brickCols; col++) {
        bricks[row][col] = true;
      }
    }

    bricksLeft = brickRows * brickCols;
    score = 0;
    lives = startLives;
    resetRound();
    paused = false;
    gameOver = false;
    gameWon = false;
    overlayDrawn = false;
    drawFullGame();
  };

  auto capBallSpeed = [&]() {
    ballVX = constrain(ballVX, -1.85f, 1.85f);
    ballVY = constrain(ballVY, -1.85f, 1.85f);
    if (ballVX > -0.30f && ballVX < 0.30f) ballVX = ballVX < 0 ? -0.30f : 0.30f;
  };

  randomSeed((uint32_t)micros() ^ analogRead(LX_PIN) ^ ((uint32_t)analogRead(RY_PIN) << 12));
  resetGame();

  while (true) {
    JoystickData input = readJoysticks();

    if (input.lb && input.rb) {
      noTone(BUZZER_PIN);
      display.clearDisplay();
      return;
    }

    if (gameOver || gameWon) {
      if (!overlayDrawn) {
        drawFullGame();
        drawOverlay(gameWon ? "You Win" : "Game Over", "LB - exit", "RB - restart");
        overlayDrawn = true;
      }
      if (input.lb) {
        noTone(BUZZER_PIN);
        display.clearDisplay();
        return;
      }
      if (input.rb && !rbWasPressed) {
        if (sound) tone(BUZZER_PIN, OK_TONE, 50);
        resetGame();
        rbWasPressed = true;
        delay(160);
      } else {
        rbWasPressed = input.rb;
      }
      delay(40);
      continue;
    }

    if (paused) {
      if (input.lb) {
        noTone(BUZZER_PIN);
        display.clearDisplay();
        return;
      }

      if (input.rb && !rbWasPressed) {
        paused = false;
        rbWasPressed = true;
        overlayDrawn = false;
        lastUpdate = millis();
        drawFullGame();
        if (sound) tone(BUZZER_PIN, OK_TONE, 40);
        delay(120);
      } else {
        rbWasPressed = input.rb;
        delay(40);
      }
      continue;
    }

    if (input.rb && !rbWasPressed) {
      paused = true;
      rbWasPressed = true;
      drawOverlay("Pause", "LB - exit", "RB - resume");
      if (sound) tone(BUZZER_PIN, BACK_TONE, 40);
      delay(120);
      continue;
    }
    rbWasPressed = input.rb;

    if (millis() - lastUpdate < frameDelay) {
      delay(2);
      continue;
    }
    lastUpdate = millis();

    int oldPaddleX = paddleX;
    int oldBallX = (int)round(ballX);
    int oldBallY = (int)round(ballY);
    float prevBallX = ballX;
    float prevBallY = ballY;

    int axis = abs(input.lx) > abs(input.rx) ? input.lx : input.rx;
    if (axis > 20 || axis < -20) {
      paddleX += axis / 18;
      paddleX = constrain(paddleX, 2, screenW - paddleW - 2);
    }

    ballX += ballVX;
    ballY += ballVY;

    if (ballX <= 1) {
      ballX = 1;
      ballVX = abs(ballVX);
      if (sound) tone(BUZZER_PIN, 1300, 30);
    } else if (ballX >= screenW - ballSize - 1) {
      ballX = screenW - ballSize - 1;
      ballVX = -abs(ballVX);
      if (sound) tone(BUZZER_PIN, 1300, 30);
    }

    if (ballY <= wallTop + 1) {
      ballY = wallTop + 1;
      ballVY = abs(ballVY);
      if (sound) tone(BUZZER_PIN, 1500, 30);
    }

    if (ballVY > 0 &&
        rectsOverlap((int)round(ballX), (int)round(ballY), ballSize, ballSize,
                     paddleX, paddleY, paddleW, paddleH)) {
      float ballCenter = ballX + ballSize / 2.0f;
      float paddleCenter = paddleX + paddleW / 2.0f;
      float hit = (ballCenter - paddleCenter) / (paddleW / 2.0f);
      ballX += (paddleX - oldPaddleX) * 0.18f;
      ballY = paddleY - ballSize - 1;
      ballVX = hit * 1.55f;
      ballVY = -abs(ballVY);
      capBallSpeed();
      if (sound) tone(BUZZER_PIN, 900, 45);
    }

    bool hitBrick = false;
    bool heartsOverlayDirty = false;
    bool scoreOverlayDirty = false;
    for (int row = 0; row < brickRows && !hitBrick; row++) {
      for (int col = 0; col < brickCols; col++) {
        if (!bricks[row][col]) continue;
        int bx = brickX(col);
        int by = brickY(row);
        if (!rectsOverlap((int)round(ballX), (int)round(ballY), ballSize, ballSize,
                          bx, by, brickW, brickH)) {
          continue;
        }

        bricks[row][col] = false;
        bricksLeft--;
        score += 10;

        int heartsX;
        int heartsYRect;
        int heartsWRect;
        int heartsHRect;
        currentHeartsRect(heartsX, heartsYRect, heartsWRect, heartsHRect);
        if (rectsOverlap(bx, by, brickW, brickH, heartsX, heartsYRect, heartsWRect, heartsHRect)) {
          heartsOverlayDirty = true;
        }

        int scoreX;
        int scoreYRect;
        int scoreWRect;
        int scoreHRect;
        currentScoreRect(scoreX, scoreYRect, scoreWRect, scoreHRect);
        if (rectsOverlap(bx, by, brickW, brickH, scoreX, scoreYRect, scoreWRect, scoreHRect)) {
          scoreOverlayDirty = true;
        }
        scoreOverlayDirty = true;
        drawBrick(row, col);

        float prevLeft = prevBallX;
        float prevRight = prevBallX + ballSize;
        float prevTop = prevBallY;
        float prevBottom = prevBallY + ballSize;
        if (prevRight <= bx) {
          ballVX = -abs(ballVX);
        } else if (prevLeft >= bx + brickW) {
          ballVX = abs(ballVX);
        } else if (prevBottom <= by) {
          ballVY = -abs(ballVY);
        } else if (prevTop >= by + brickH) {
          ballVY = abs(ballVY);
        } else if (abs(ballVX) > abs(ballVY)) {
          ballVX = -ballVX;
        } else {
          ballVY = -ballVY;
        }

        ballVX *= 1.005f;
        ballVY *= 1.005f;
        capBallSpeed();
        hitBrick = true;
        if (sound) tone(BUZZER_PIN, 1200 + row * 120, 45);
        break;
      }
    }

    if (bricksLeft == 0) {
      gameWon = true;
      overlayDrawn = false;
      if (sound) tone(BUZZER_PIN, 1700, 180);
    } else if (ballY > screenH) {
      lives--;
      if (lives <= 0) {
        gameOver = true;
        overlayDrawn = false;
        if (sound) tone(BUZZER_PIN, 220, 300);
      } else {
        if (sound) tone(BUZZER_PIN, 360, 150);
        resetRound();
        drawFullGame();
        delay(350);
        continue;
      }
    }

    int clearBallX = min(oldBallX, (int)round(ballX)) - 1;
    int clearBallY = min(oldBallY, (int)round(ballY)) - 1;
    int clearBallW = ballSize + abs((int)round(ballX) - oldBallX) + 2;
    int clearBallH = ballSize + abs((int)round(ballY) - oldBallY) + 2;
    display.fillRect(clearBallX, clearBallY, clearBallW, clearBallH, SSD1306_BLACK);
    redrawStaticInRect(clearBallX, clearBallY, clearBallW, clearBallH);

    if (oldPaddleX != paddleX) {
      int clearPaddleX = min(oldPaddleX, paddleX);
      int clearPaddleW = paddleW + abs(paddleX - oldPaddleX);
      display.fillRect(clearPaddleX, paddleY, clearPaddleW, paddleH, SSD1306_BLACK);
      redrawStaticInRect(clearPaddleX, paddleY, clearPaddleW, paddleH);
      drawPaddle();
    }

    if (!gameOver && !gameWon) drawBall();
    int newBallX = (int)round(ballX);
    int newBallY = (int)round(ballY);

    int heartsX;
    int heartsYRect;
    int heartsWRect;
    int heartsHRect;
    currentHeartsRect(heartsX, heartsYRect, heartsWRect, heartsHRect);
    int scoreX;
    int scoreYRect;
    int scoreWRect;
    int scoreHRect;
    currentScoreRect(scoreX, scoreYRect, scoreWRect, scoreHRect);

    heartsOverlayDirty = heartsOverlayDirty ||
                         rectsOverlap(clearBallX, clearBallY, clearBallW, clearBallH,
                                      heartsX, heartsYRect, heartsWRect, heartsHRect) ||
                         rectsOverlap(newBallX, newBallY, ballSize, ballSize,
                                      heartsX, heartsYRect, heartsWRect, heartsHRect);
    scoreOverlayDirty = scoreOverlayDirty ||
                        rectsOverlap(clearBallX, clearBallY, clearBallW, clearBallH,
                                     scoreX, scoreYRect, scoreWRect, scoreHRect) ||
                        rectsOverlap(newBallX, newBallY, ballSize, ballSize,
                                     scoreX, scoreYRect, scoreWRect, scoreHRect);
    drawStatusOverlays(true, true);
    display.display();
  }
}
