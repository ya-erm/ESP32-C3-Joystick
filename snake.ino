extern bool sound;

void playSnakeGame() {
    const int screenW = 128;
    const int screenH = 64;
    const int topOffset = 16;  // верхние 16 пикселей (желтые)
    const int borderWidth = 1;  // ширина рамки
    const int cellSize = 4;  // размер "клетки"
    
    // Доступное пространство для игры (с учетом рамки)
    const int gameAreaW = screenW - 2 * borderWidth;  // 126
    const int gameAreaH = screenH - topOffset - 2 * borderWidth;  // 46
    const int gameAreaX = borderWidth;  // 1
    const int gameAreaY = topOffset + borderWidth;  // 17
    
    const int cellsX = gameAreaW / cellSize;  // 31
    const int cellsY = gameAreaH / cellSize;  // 11
  
    // === Змейка ===
    int snakeX[200];
    int snakeY[200];
    int snakeLength = 4;
    int dirX = 1;
    int dirY = 0;
  
    // начальная позиция
    for (int i = 0; i < snakeLength; i++) {
      snakeX[i] = cellsX / 2 - i;
      snakeY[i] = cellsY / 2;
    }
  
    // === Еда ===
    int foodX = random(cellsX);
    int foodY = random(cellsY);
  
    int score = 0;
    unsigned long lastMove = 0;
    int speed = 200; // миллисекунд между шагами
    bool gameOver = false;
  
    while (true) {
      // === Экран Game Over ===
      if (gameOver) {
        display.clearDisplay();
        
        // Отрисовка игрового поля (змейка, еда, рамка, очки)
        // Рамка игрового поля
        display.drawRect(gameAreaX, gameAreaY, gameAreaW, gameAreaH, SSD1306_WHITE);
        
        // Еда (пустая рамка 3x3)
        int foodPixelX = gameAreaX + borderWidth + foodX * cellSize;
        int foodPixelY = gameAreaY + borderWidth + foodY * cellSize;
        display.drawRect(foodPixelX, foodPixelY, 3, 3, SSD1306_WHITE);
        
        // Змейка
        for (int i = 0; i < snakeLength; i++) {
          int pixelX = gameAreaX + borderWidth + snakeX[i] * cellSize;
          int pixelY = gameAreaY + borderWidth + snakeY[i] * cellSize;
          
          if (i == 0) {
            // Голова - заполненная
            display.fillRect(pixelX, pixelY, 3, 3, SSD1306_WHITE);
          } else {
            // Тело - пустая рамка
            display.drawRect(pixelX, pixelY, 3, 3, SSD1306_WHITE);
          }
        }
        
        // Счёт (в верхней части, но не в игровом поле)
        display.setTextSize(1);
        display.setCursor((screenW / 2) - 3, 4);
        display.printf("%d", score);
        
        // Рамка для Game Over (пустая заливка) поверх игрового поля
        const int gameOverW = 72;
        const int gameOverH = 20;
        const int gameOverX = (screenW - gameOverW) / 2;
        const int gameOverY = (screenH - gameOverH + topOffset) / 2;
        display.drawRect(gameOverX, gameOverY, gameOverW, gameOverH, SSD1306_WHITE);
        
        // Текст "Game Over"
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        int textX = gameOverX + (gameOverW - 9 * 6) / 2; // "Game Over" = 9 символов * 6 пикселей
        int textY = gameOverY + (gameOverH - 8) / 2; // высота текста = 8 пикселей
        display.setCursor(textX, textY);
        display.print("Game Over");
        
        display.display();
        
        // Обработка кнопок в состоянии Game Over
        while (true) {
          JoystickData input = readJoysticks();
          
          if (input.rb) {
            // Перезапуск игры
            gameOver = false;
            // Сброс всех переменных
            snakeLength = 4;
            dirX = 1;
            dirY = 0;
            for (int i = 0; i < snakeLength; i++) {
              snakeX[i] = cellsX / 2 - i;
              snakeY[i] = cellsY / 2;
            }
            foodX = random(cellsX);
            foodY = random(cellsY);
            score = 0;
            speed = 200;
            lastMove = millis();
            break;
          }
          
          if (input.lb) {
            // Выход из игры
            noTone(BUZZER_PIN);
            return;
          }
          
          delay(50); // Небольшая задержка для снижения нагрузки
        }
        continue; // Продолжаем игровой цикл после перезапуска
      }
      // --- Чтение джойстиков ---
      JoystickData input = readJoysticks();
      
      // --- Выход из игры при нажатии обеих кнопок ---
      if (input.lb && input.rb) {
        noTone(BUZZER_PIN);
        return;
      }
  
      // --- Определение направления ---
      int moveX = 0, moveY = 0;
      if (input.lx > 50 || input.rx > 50) moveX = 1;
      else if (input.lx < -50 || input.rx < -50) moveX = -1;
      else if (input.ly > 50 || input.ry > 50) moveY = -1;
      else if (input.ly < -50 || input.ry < -50) moveY = 1;
      
      // --- Запрет на обратный ход ---
      if (moveX != 0 && dirX == 0) { dirX = moveX; dirY = 0; }
      else if (moveY != 0 && dirY == 0) { dirY = moveY; dirX = 0; }
  
      // --- Движение змейки ---
      if (millis() - lastMove > speed) {
        lastMove = millis();
  
        // Сдвигаем тело
        for (int i = snakeLength - 1; i > 0; i--) {
          snakeX[i] = snakeX[i - 1];
          snakeY[i] = snakeY[i - 1];
        }
  
        // Голова
        snakeX[0] += dirX;
        snakeY[0] += dirY;
  
        // Проверка выхода за пределы — смерть
        if (snakeX[0] < 0 || snakeX[0] >= cellsX ||
            snakeY[0] < 0 || snakeY[0] >= cellsY) {
          if (sound) tone(BUZZER_PIN, 200, 400);
          gameOver = true;
          continue;
        }
  
        // Проверка самоудара
        for (int i = 1; i < snakeLength; i++) {
          if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
            if (sound) tone(BUZZER_PIN, 200, 400);
            gameOver = true;
            continue;
          }
        }
  
        // Проверка еды
        if (snakeX[0] == foodX && snakeY[0] == foodY) {
          if (sound) tone(BUZZER_PIN, 1000, 80);
          snakeLength++;
          score++;
          if (speed > 60) speed -= 5;  // ускоряем игру
          foodX = random(cellsX);
          foodY = random(cellsY);
        }
  
        // === Отрисовка ===
        display.clearDisplay();
  
        // Рамка игрового поля
        display.drawRect(gameAreaX, gameAreaY, gameAreaW, gameAreaH, SSD1306_WHITE);
  
        // Еда (пустая рамка 3x3)
        int foodPixelX = gameAreaX + borderWidth + foodX * cellSize;
        int foodPixelY = gameAreaY + borderWidth + foodY * cellSize;
        display.drawRect(foodPixelX, foodPixelY, 3, 3, SSD1306_WHITE);
  
        // Змейка
        for (int i = 0; i < snakeLength; i++) {
          int pixelX = gameAreaX + borderWidth + snakeX[i] * cellSize;
          int pixelY = gameAreaY + borderWidth + snakeY[i] * cellSize;
          
          if (i == 0) {
            // Голова - заполненная
            display.fillRect(pixelX, pixelY, 3, 3, SSD1306_WHITE);
          } else {
            // Тело - пустая рамка
            display.drawRect(pixelX, pixelY, 3, 3, SSD1306_WHITE);
          }
        }
  
        // Счёт (в верхней части, но не в игровом поле)
        display.setTextSize(1);
        display.setCursor((screenW / 2) - 3, 4);
        display.printf("%d", score);
  
        display.display();
      }
    }
  }
  