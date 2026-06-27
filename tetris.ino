extern bool sound;

// === Размеры игрового поля ===
// Экран в портретной ориентации ST7735: 80x160. HUD сверху, поле занимает всю ширину.
const int FIELD_WIDTH = 20;
const int FIELD_HEIGHT = 37;
const int CELL_SIZE = 4;
const int FIELD_X = 0;
const int FIELD_Y = 11;
const int TETRIS_HUD_HEIGHT = 10;
const int TETRIS_GHOST_CELL = 8;

// === Фигуры тетромино (7 типов) ===
const int TETROMINOES[7][4][4] = {
  // I
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  // O
  {{0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}},
  // T
  {{0,0,0,0}, {0,1,0,0}, {1,1,1,0}, {0,0,0,0}},
  // S
  {{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
  // Z
  {{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
  // J
  {{0,0,0,0}, {1,0,0,0}, {1,1,1,0}, {0,0,0,0}},
  // L
  {{0,0,0,0}, {0,0,1,0}, {1,1,1,0}, {0,0,0,0}}
};

// === Игровое поле (0 = пусто, 1-7 = цвет фигуры) ===
int field[FIELD_HEIGHT][FIELD_WIDTH];
int renderedField[FIELD_HEIGHT][FIELD_WIDTH];
bool tetrisRenderInitialized = false;
int tetrisLastHudScore = -1;
int tetrisLastHudLevel = -1;
int tetrisLastHudLines = -1;
int tetrisLastHudNextType = -1;
bool tetrisLastHudGameOver = false;

Piece currentPiece;
Piece nextPiece;

int score = 0;
int level = 1;
int linesCleared = 0;

// === Поворот фигуры на 90 градусов по часовой стрелке ===
void rotatePiece(Piece* piece) {
  piece->rotation = (piece->rotation + 1) % 4;
}

// === Получить блок фигуры с учетом поворота ===
bool getBlock(int type, int rotation, int x, int y) {
  int temp;
  switch(rotation) {
    case 0: return TETROMINOES[type][y][x] != 0;
    case 1: // поворот на 90°
      temp = x;
      x = 3 - y;
      y = temp;
      return TETROMINOES[type][y][x] != 0;
    case 2: // поворот на 180°
      x = 3 - x;
      y = 3 - y;
      return TETROMINOES[type][y][x] != 0;
    case 3: // поворот на 270°
      temp = x;
      x = y;
      y = 3 - temp;
      return TETROMINOES[type][y][x] != 0;
  }
  return false;
}

// === Проверка коллизии фигуры с полем ===
bool checkCollision(Piece piece) {
  for (int py = 0; py < 4; py++) {
    for (int px = 0; px < 4; px++) {
      if (getBlock(piece.type, piece.rotation, px, py)) {
        int fx = piece.x + px;
        int fy = piece.y + py;
        
        // Проверка границ
        if (fx < 0 || fx >= FIELD_WIDTH || fy < 0 || fy >= FIELD_HEIGHT) {
          return true;
        }
        
        // Проверка столкновения с установленными блоками
        if (field[fy][fx] != 0) {
          return true;
        }
      }
    }
  }
  return false;
}

// === Закрепление фигуры на поле ===
void lockPiece(Piece piece) {
  for (int py = 0; py < 4; py++) {
    for (int px = 0; px < 4; px++) {
      if (getBlock(piece.type, piece.rotation, px, py)) {
        int fx = piece.x + px;
        int fy = piece.y + py;
        if (fx >= 0 && fx < FIELD_WIDTH && fy >= 0 && fy < FIELD_HEIGHT) {
          field[fy][fx] = piece.type + 1;
        }
      }
    }
  }
}

// === Удаление заполненных линий ===
int clearLines() {
  int linesRemoved = 0;
  int y = FIELD_HEIGHT - 1;
  while (y >= 0) {
    bool fullLine = true;
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (field[y][x] == 0) {
        fullLine = false;
        break;
      }
    }
    
    if (fullLine) {
      // Удаляем линию: сдвигаем все линии выше вниз
      for (int moveY = y; moveY > 0; moveY--) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
          field[moveY][x] = field[moveY - 1][x];
        }
      }
      // Очищаем верхнюю линию
      for (int x = 0; x < FIELD_WIDTH; x++) {
        field[0][x] = 0;
      }
      linesRemoved++;
      // Проверяем эту же позицию снова
    } else {
      y--;
    }
  }
  return linesRemoved;
}

// === Создание новой фигуры ===
Piece createNewPiece() {
  Piece p;
  p.type = random(7);
  p.rotation = 0;
  p.x = FIELD_WIDTH / 2 - 2;  // центр по ширине
  p.y = 0;                     // начинаем сверху
  return p;
}

uint16_t tetrisPieceColor(int value) {
  switch (value) {
    case 1: return ST77XX_CYAN;
    case 2: return ST77XX_YELLOW;
    case 3: return ST77XX_MAGENTA;
    case 4: return ST77XX_GREEN;
    case 5: return ST77XX_RED;
    case 6: return ST77XX_BLUE;
    case 7: return ST77XX_ORANGE;
    default: return SSD1306_WHITE;
  }
}

// === Отрисовка блока ===
void drawTetrisCell(int x, int y, int value) {
  int px = FIELD_X + x * CELL_SIZE;
  int py = FIELD_Y + y * CELL_SIZE;
  display.fillRect(px, py, CELL_SIZE, CELL_SIZE, SSD1306_BLACK);

  if (value >= 1 && value <= 7) {
    display.fillRect(px, py, CELL_SIZE - 1, CELL_SIZE - 1, tetrisPieceColor(value));
  } else if (value == TETRIS_GHOST_CELL) {
    display.drawPixel(px + CELL_SIZE / 2 - 1, py + CELL_SIZE / 2 - 1, UI_SEPARATOR_COLOR);
  }
}

void drawBlock(int x, int y, bool filled, uint16_t color = SSD1306_WHITE) {
  int px = FIELD_X + x * CELL_SIZE;
  int py = FIELD_Y + y * CELL_SIZE;
  if (filled) {
    display.fillRect(px, py, CELL_SIZE - 1, CELL_SIZE - 1, color);
  } else {
    display.drawPixel(px + CELL_SIZE / 2 - 1, py + CELL_SIZE / 2 - 1, color);
  }
}

void resetTetrisRenderCache() {
  tetrisRenderInitialized = false;
  tetrisLastHudScore = -1;
  tetrisLastHudLevel = -1;
  tetrisLastHudLines = -1;
  tetrisLastHudNextType = -1;
  tetrisLastHudGameOver = false;
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      renderedField[y][x] = -1;
    }
  }
}

void drawTetrisFrame() {
  display.drawFastHLine(0, TETRIS_HUD_HEIGHT, display.width(), UI_SEPARATOR_COLOR);
}

void drawNextPiecePreview(Piece piece, int x, int y) {
  const int nextCellSize = 3;
  for (int py = 0; py < 4; py++) {
    for (int px = 0; px < 4; px++) {
      if (getBlock(piece.type, piece.rotation, px, py)) {
        display.fillRect(x + px * 4, y + py * 4, nextCellSize, nextCellSize, tetrisPieceColor(piece.type + 1));
      }
    }
  }
}

void drawTetrisHud(bool gameOver, bool force = false) {
  bool changed = force ||
                 score != tetrisLastHudScore ||
                 level != tetrisLastHudLevel ||
                 linesCleared != tetrisLastHudLines ||
                 nextPiece.type != tetrisLastHudNextType ||
                 gameOver != tetrisLastHudGameOver;
  if (!changed) return;

  display.fillRect(0, 0, display.width(), TETRIS_HUD_HEIGHT, SSD1306_BLACK);
  display.setTextSize(1);
  display.setTextColor(UI_HEADER_COLOR);
  display.setCursor(0, 1);
  display.print("L");
  display.print(level);

  if (!gameOver) {
    drawNextPiecePreview(nextPiece, (display.width() - 16) / 2, -2);
  }

  char scoreText[9];
  snprintf(scoreText, sizeof(scoreText), "%d", score);
  int scoreX = display.width() - strlen(scoreText) * 6;
  if (scoreX < 42) scoreX = 42;
  display.setCursor(scoreX, 1);
  display.print(scoreText);
  display.setTextColor(SSD1306_WHITE);

  drawTetrisFrame();

  tetrisLastHudScore = score;
  tetrisLastHudLevel = level;
  tetrisLastHudLines = linesCleared;
  tetrisLastHudNextType = nextPiece.type;
  tetrisLastHudGameOver = gameOver;
}

int tetrisVisibleCell(int x, int y, Piece ghost, bool gameOver) {
  int value = field[y][x];
  if (!gameOver) {
    for (int py = 0; py < 4; py++) {
      for (int px = 0; px < 4; px++) {
        if (getBlock(currentPiece.type, currentPiece.rotation, px, py)) {
          int fx = currentPiece.x + px;
          int fy = currentPiece.y + py;
          if (fx == x && fy == y) {
            return currentPiece.type + 1;
          }
        }
      }
    }
    if (value == 0) {
      for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
          if (getBlock(ghost.type, ghost.rotation, px, py)) {
            int fx = ghost.x + px;
            int fy = ghost.y + py;
            if (fx == x && fy == y) {
              return TETRIS_GHOST_CELL;
            }
          }
        }
      }
    }
  }
  return value;
}

void drawTetrisBoard(bool gameOver, bool force = false) {
  if (!tetrisRenderInitialized || force) {
    display.clearDisplay();
    drawTetrisFrame();
    for (int y = 0; y < FIELD_HEIGHT; y++) {
      for (int x = 0; x < FIELD_WIDTH; x++) {
        renderedField[y][x] = -1;
      }
    }
    tetrisRenderInitialized = true;
    drawTetrisHud(gameOver, true);
  }

  Piece ghost = currentPiece;
  if (!gameOver) {
    ghost = getGhostPiece(currentPiece);
  }

  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      int value = tetrisVisibleCell(x, y, ghost, gameOver);
      if (value != renderedField[y][x]) {
        drawTetrisCell(x, y, value);
        renderedField[y][x] = value;
      }
    }
  }

  drawTetrisHud(gameOver);
}

// Старые обертки оставлены для локальной совместимости с игровыми хелперами.
void drawField() {
  drawTetrisBoard(false, true);
}

void drawPiece(Piece piece) {
  for (int py = 0; py < 4; py++) {
    for (int px = 0; px < 4; px++) {
      if (getBlock(piece.type, piece.rotation, px, py)) {
        int fx = piece.x + px;
        int fy = piece.y + py;
        if (fy >= 0 && fx >= 0 && fx < FIELD_WIDTH) {
          drawBlock(fx, fy, true, tetrisPieceColor(piece.type + 1));
        }
      }
    }
  }
}

void drawNextPiece(Piece piece, int x, int y) {
  drawNextPiecePreview(piece, x, y);
}

void drawTetrisPauseBox(int x, int y, int w, const char* text, uint16_t textColor) {
  const int h = 12;
  display.fillRect(x, y, w, h, SSD1306_BLACK);
  display.drawRect(x, y, w, h, UI_SEPARATOR_COLOR);
  display.setTextSize(1);
  display.setTextColor(textColor);
  display.setCursor(x + (w - strlen(text) * 6) / 2, y + 2);
  display.print(text);
  display.setTextColor(SSD1306_WHITE);
}

void drawTetrisPauseOverlay() {
  const int boxW = 78;
  const int boxH = 12;
  const int gap = 4;
  const int x = (display.width() - boxW) / 2;
  const int y = FIELD_Y + FIELD_HEIGHT * CELL_SIZE / 2 - (boxH * 3 + gap * 2) / 2;
  drawTetrisPauseBox(x, y, boxW, "LB - resume", UI_FOOTER_COLOR);
  drawTetrisPauseBox(x, y + boxH + gap, boxW, "Pause", SSD1306_WHITE);
  drawTetrisPauseBox(x, y + (boxH + gap) * 2, boxW, "LB+RB - exit", UI_FOOTER_COLOR);
}

// === Вычисление позиции призрака (куда упадет фигура) ===
Piece getGhostPiece(Piece piece) {
  Piece ghost = piece;
  // Опускаем фигуру вниз пока не будет коллизии
  while (true) {
    Piece testPiece = ghost;
    testPiece.y++;
    if (checkCollision(testPiece)) {
      break; // Достигли дна или столкновения
    }
    ghost.y++;
  }
  return ghost;
}

// === Отрисовка призрака фигуры (точки) ===
void playTetrisGame() {
  // === Поворот дисплея в портретный режим 80x160 ===
  display.setRotation(TFT_TETRIS_ROTATION);
  
  // === Инициализация ===
  display.clearDisplay();
  resetTetrisRenderCache();
  // Очистка поля
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      field[y][x] = 0;
    }
  }
  
  score = 0;
  level = 1;
  linesCleared = 0;
  
  currentPiece = createNewPiece();
  nextPiece = createNewPiece();
  
  unsigned long lastFall = millis();
  unsigned long lastInput = millis();
  unsigned long lastRotate = 0;  // время последнего поворота
  unsigned long lastRender = 0;
  const unsigned long renderInterval = 16;
  int fallDelay = 500; // начальная скорость падения (мс)
  
  bool gameOver = false;
  bool paused = false;
  bool lbWasPressed = true;
  bool rbWasPressed = true;
  bool canHardDrop = true;  // флаг разрешения hard drop (нужен возврат джойстика в нейтральное положение)
  
  while (true) {
    // --- Чтение джойстиков ---
    JoystickData input = readJoysticks();
    
    // --- Выход из игры при нажатии обеих кнопок ---
    if (input.lb && input.rb) {
      noTone(BUZZER_PIN);
      display.setRotation(TFT_MENU_ROTATION);  // возвращаем нормальную ориентацию меню
      display.clearDisplay();
      return;
    }

    if (paused) {
      if ((input.lb && !lbWasPressed) || (input.rb && !rbWasPressed)) {
        paused = false;
        lbWasPressed = input.lb;
        rbWasPressed = input.rb;
        lastFall = millis();
        lastInput = millis();
        lastRender = millis();
        resetTetrisRenderCache();
        drawTetrisBoard(gameOver, true);
        display.display();
        if (sound) tone(BUZZER_PIN, OK_TONE, 50);
        delay(120);
        continue;
      }

      lbWasPressed = input.lb;
      rbWasPressed = input.rb;
      delay(40);
      continue;
    }

    if (!gameOver && input.lb && !lbWasPressed) {
      paused = true;
      lbWasPressed = true;
      rbWasPressed = input.rb;
      drawTetrisBoard(gameOver);
      drawTetrisPauseOverlay();
      display.display();
      if (sound) tone(BUZZER_PIN, BACK_TONE, 50);
      continue;
    }
    lbWasPressed = input.lb;
    rbWasPressed = input.rb;
    
    // --- Перезапуск игры после game over при нажатии RB ---
    if (gameOver && input.rb) {
      // Сброс игры
      for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
          field[y][x] = 0;
        }
      }
      
      score = 0;
      level = 1;
      linesCleared = 0;
      
      currentPiece = createNewPiece();
      nextPiece = createNewPiece();
      
      lastFall = millis();
      lastInput = millis();
      lastRotate = 0;
      lastRender = 0;
      fallDelay = 500;
      resetTetrisRenderCache();
      
      gameOver = false;
      paused = false;
      lbWasPressed = input.lb;
      rbWasPressed = input.rb;
      canHardDrop = true;
      
      if (sound) tone(BUZZER_PIN, OK_TONE, 50);
      delay(200);
      continue;  // переходим к следующей итерации цикла
    }
    
    if (!gameOver) {
      // --- Обработка ввода ---
      if (millis() - lastInput > 100) {
        lastInput = millis();
        
        // Движение влево/вправо (используем ly/ry)
        if (input.ly < -50 || input.ry < -50) {
          Piece testPiece = currentPiece;
          testPiece.x--;
          if (!checkCollision(testPiece)) {
            currentPiece.x--;
            if (sound) tone(BUZZER_PIN, 300, 30);
          }
        } else if (input.ly > 50 || input.ry > 50) {
          Piece testPiece = currentPiece;
          testPiece.x++;
          if (!checkCollision(testPiece)) {
            currentPiece.x++;
            if (sound) tone(BUZZER_PIN, 300, 30);
          }
        }
        
        // Поворот - нажатие кнопки RB (с wall kick - автоматическим смещением)
        if (input.rb && millis() - lastRotate > 250) {
          lastRotate = millis();
          Piece testPiece = currentPiece;
          rotatePiece(&testPiece);
          
          // Пробуем повернуть без смещения
          if (!checkCollision(testPiece)) {
            rotatePiece(&currentPiece);
            if (sound) tone(BUZZER_PIN, 400, 30);
          } else {
            // Если не получилось, пробуем сместить влево/вправо (wall kick)
            bool rotated = false;
            int offsets[] = {-2, -1, 1, 2};  // пробуем смещения: -2, -1, +1, +2
            for (int i = 0; i < 4 && !rotated; i++) {
              Piece kickPiece = currentPiece;
              rotatePiece(&kickPiece);
              kickPiece.x += offsets[i];
              if (!checkCollision(kickPiece)) {
                currentPiece = kickPiece;
                rotated = true;
                if (sound) tone(BUZZER_PIN, 400, 30);
              }
            }
          }
        }
        
        // Hard drop (бросок до конца) - движение джойстика lx или rx < -50
        // Проверяем, что джойстик вернулся в нейтральное положение после предыдущего броска
        if (input.lx > -50 && input.lx < 50 && input.rx > -50 && input.rx < 50) {
          canHardDrop = true;  // джойстик в нейтральном положении, можно делать hard drop
        }
        
        if ((input.lx < -50 || input.rx < -50) && canHardDrop) {
          Piece ghost = getGhostPiece(currentPiece);
          if (ghost.y != currentPiece.y) {
            canHardDrop = false;  // запрещаем повторный hard drop до возврата джойстика в нейтральное положение
            
            currentPiece = ghost;
            lockPiece(currentPiece);
            
            // Удаление заполненных линий
            int cleared = clearLines();
            if (cleared > 0) {
              linesCleared += cleared;
              int points[] = {0, 100, 300, 500, 800};
              score += points[cleared] * level;
              level = (linesCleared / 10) + 1;
              fallDelay = max(50, 500 - (level - 1) * 50);
              // Веселая мелодия при удалении линии
              if (sound) {
                tone(BUZZER_PIN, 523, 50);  // C5
                delay(60);
                tone(BUZZER_PIN, 659, 50);  // E5
                delay(60);
                tone(BUZZER_PIN, 784, 50);  // G5
                delay(60);
                noTone(BUZZER_PIN);
              }
            }
            
            // Новая фигура
            currentPiece = nextPiece;
            nextPiece = createNewPiece();
            
            // Проверка на проигрыш
            if (checkCollision(currentPiece)) {
              gameOver = true;
              if (sound) tone(BUZZER_PIN, 150, 500);
              delay(500);
            }
            
            lastFall = millis(); // сброс таймера падения
            if (sound) tone(BUZZER_PIN, 500, 50);
          }
        }
        
        // Ускоренное падение (движение вниз) - в два раза быстрее
        if (input.lx > 50 || input.rx > 50) {
          Piece testPiece = currentPiece;
          testPiece.y += 2;  // падаем на 2 клетки вместо 1
          if (!checkCollision(testPiece)) {
            currentPiece.y += 2;
            if (sound) tone(BUZZER_PIN, 200, 20);
          } else {
            // Если не можем упасть на 2, пробуем на 1
            testPiece = currentPiece;
            testPiece.y++;
            if (!checkCollision(testPiece)) {
              currentPiece.y++;
              if (sound) tone(BUZZER_PIN, 200, 20);
            }
          }
        }
      }
      
      // --- Падение фигуры ---
      if (millis() - lastFall > fallDelay) {
        lastFall = millis();
        
        Piece testPiece = currentPiece;
        testPiece.y++;
        
        if (checkCollision(testPiece)) {
          // Фигура достигла дна или столкнулась
          lockPiece(currentPiece);
          
          // Проверка на проигрыш
          if (currentPiece.y <= 0) {
            gameOver = true;
            if (sound) tone(BUZZER_PIN, 150, 500);
            delay(500);
          } else {
            // Удаление заполненных линий
            int cleared = clearLines();
            if (cleared > 0) {
              linesCleared += cleared;
              // Очки: 100 за 1 линию, 300 за 2, 500 за 3, 800 за 4
              int points[] = {0, 100, 300, 500, 800};
              score += points[cleared] * level;
              
              // Увеличение уровня каждые 10 линий
              level = (linesCleared / 10) + 1;
              fallDelay = max(50, 500 - (level - 1) * 50);
              
              if (sound) tone(BUZZER_PIN, 600, 100);
            }
            
            // Новая фигура
            currentPiece = nextPiece;
            nextPiece = createNewPiece();
            
            // Проверка на проигрыш при появлении новой фигуры
            if (checkCollision(currentPiece)) {
              gameOver = true;
              if (sound) tone(BUZZER_PIN, 150, 500);
              delay(500);
            }
          }
        } else {
          currentPiece.y++;
        }
      }
    }
    
    // === Отрисовка ===
    unsigned long renderNow = millis();
    if (renderNow - lastRender >= renderInterval) {
      lastRender = renderNow;
      drawTetrisBoard(gameOver);

      if (gameOver) {
        display.setTextSize(1);
        int rectWidth = 42;
        int rectHeight = 26;
        int rectX = (display.width() - rectWidth) / 2;
        int rectY = FIELD_Y + FIELD_HEIGHT * CELL_SIZE / 2 - rectHeight / 2;
        int textX = rectX + 8;
        int textY = rectY + 5;

        display.fillRect(rectX, rectY, rectWidth, rectHeight, SSD1306_BLACK);
        display.drawRect(rectX, rectY, rectWidth, rectHeight, UI_SEPARATOR_COLOR);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(textX, textY);
        display.print("GAME");
        display.setCursor(textX, textY + 10);
        display.print("OVER");
      }

      display.display();
    }
  }
}
