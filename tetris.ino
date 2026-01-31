extern bool sound;

// === Размеры игрового поля ===
// Экран повернут на 90°: 64x128 (ширина x высота)
// Поле занимает всю высоту экрана
const int FIELD_WIDTH = 11;   // ширина поля (количество клеток) = 11*4=44px
const int FIELD_HEIGHT = 30;  // высота поля (количество клеток) = 30*4=120px
const int CELL_SIZE = 4;      // размер клетки в пикселях
const int FIELD_X = 2;        // начинаем с X=1
const int FIELD_Y = 2;        // начинаем с Y=0

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
// field[y][x] где y - строка (0-19), x - столбец (0-9)
int field[FIELD_HEIGHT][FIELD_WIDTH];

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

// === Отрисовка блока ===
void drawBlock(int x, int y, bool filled) {
  int px = FIELD_X + x * CELL_SIZE;
  int py = FIELD_Y + y * CELL_SIZE;
  if (filled) {
    display.fillRect(px, py, CELL_SIZE - 1, CELL_SIZE - 1, SSD1306_WHITE);
  } else {
    display.drawRect(px, py, CELL_SIZE - 1, CELL_SIZE - 1, SSD1306_WHITE);
  }
}

// === Отрисовка игрового поля ===
void drawField() {
  // Рамка поля
  display.drawRect(FIELD_X - 1, FIELD_Y -1, 
                   FIELD_WIDTH * CELL_SIZE + 1, 
                   FIELD_HEIGHT * CELL_SIZE + 1, 
                   SSD1306_WHITE);
  
  // Установленные блоки
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (field[y][x] != 0) {
        drawBlock(x, y, true);
      }
    }
  }
}

// === Отрисовка текущей фигуры ===
void drawPiece(Piece piece) {
  for (int py = 0; py < 4; py++) {
    for (int px = 0; px < 4; px++) {
      if (getBlock(piece.type, piece.rotation, px, py)) {
        int fx = piece.x + px;
        int fy = piece.y + py;
        if (fy >= 0 && fx >= 0 && fx < FIELD_WIDTH) {
          drawBlock(fx, fy, true);
        }
      }
    }
  }
}

// === Отрисовка следующей фигуры (в тех же размерах что и в игровом поле) ===
void drawNextPiece(Piece piece, int x, int y) {
  const int nextCellSize = CELL_SIZE - 1;  // размер клетки 3x3 (как в игровом поле)
  for (int py = 0; py < 4; py++) {
    for (int px = 0; px < 4; px++) {
      if (getBlock(piece.type, piece.rotation, px, py)) {
        // Промежуток в 1 пиксель между блоками
        int px_screen = x + px * (nextCellSize + 1);
        int py_screen = y + py * (nextCellSize + 1);
        display.fillRect(px_screen, py_screen, nextCellSize, nextCellSize, SSD1306_WHITE);
      }
    }
  }
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
void drawGhostPiece(Piece piece) {
  for (int py = 0; py < 4; py++) {
    for (int px = 0; px < 4; px++) {
      if (getBlock(piece.type, piece.rotation, px, py)) {
        int fx = piece.x + px;
        int fy = piece.y + py;
        if (fy >= 0 && fx >= 0 && fx < FIELD_WIDTH) {
          // Рисуем точку в центре каждого блока (блок 3x3, центр на позиции 1)
          int blockSize = CELL_SIZE - 1;  // фактический размер блока 3x3
          int px_screen = FIELD_X + fx * CELL_SIZE + blockSize / 2;
          int py_screen = FIELD_Y + fy * CELL_SIZE + blockSize / 2;
          display.drawPixel(px_screen, py_screen, SSD1306_WHITE);
        }
      }
    }
  }
}

void playTetrisGame() {
  // === Поворот дисплея на 90 градусов ===
  display.setRotation(3);
  
  // === Инициализация ===
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
  int fallDelay = 500; // начальная скорость падения (мс)
  
  bool gameOver = false;
  bool canHardDrop = true;  // флаг разрешения hard drop (нужен возврат джойстика в нейтральное положение)
  
  while (true) {
    // --- Чтение джойстиков ---
    JoystickData input = readJoysticks();
    
    // --- Выход из игры при нажатии обеих кнопок ---
    if (input.lb && input.rb) {
      noTone(BUZZER_PIN);
      display.setRotation(0);  // возвращаем нормальную ориентацию
      return;
    }
    
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
      fallDelay = 500;
      
      gameOver = false;
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
    display.clearDisplay();
    
    // Игровое поле
    drawField();
    
    // Призрак фигуры (показываем куда упадет) - рисуем перед основной фигурой
    if (!gameOver) {
      Piece ghost = getGhostPiece(currentPiece);
      drawGhostPiece(ghost);
    }
    
    // Текущая фигура
    if (!gameOver) {
      drawPiece(currentPiece);
    }
    
    // Информация справа от поля
    display.setTextSize(1);
    
    // Уровень в виде L1, L2
    display.setCursor(50, 4);
    display.print("L");
    display.print(level);
    
    // Количество линий
    display.setCursor(50, 128-14);
    display.printf("%2d", linesCleared);
    
    // Следующая фигура
    if (!gameOver) {
      drawNextPiece(nextPiece, 48, 20);
    }
    
    if (gameOver) {
      display.setTextSize(1);
      
      // Координаты для текста (центр поля)
      int textX = FIELD_X + FIELD_WIDTH * CELL_SIZE / 2 - 12;  // центр по X
      int textY = FIELD_Y + FIELD_HEIGHT * CELL_SIZE / 2 - 8;  // центр по Y
      
      // Размеры прямоугольника
      int rectWidth = 32;
      int rectHeight = 26;
      int rectX = textX - 4;
      int rectY = textY - 4;
      
      // Очищаем заливку внутри прямоугольника (черный цвет)
      display.fillRect(rectX, rectY, rectWidth, rectHeight, SSD1306_BLACK);
      
      // Рисуем рамку прямоугольника (белый цвет)
      display.drawRect(rectX, rectY, rectWidth, rectHeight, SSD1306_WHITE);
      
      // Текст в две строки
      display.setCursor(textX, textY);
      display.print("GAME");
      display.setCursor(textX, textY + 10);
      display.print("OVER");
    }
    
    display.display();
  }
}
