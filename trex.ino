extern bool sound;

static const int TREX_SCREEN_W = SCREEN_WIDTH;
static const int TREX_SCREEN_H = SCREEN_HEIGHT;
static const int TREX_HUD_H = 10;
static const int TREX_FOOTER_H = 11;
static const int TREX_GROUND_Y = SCREEN_HEIGHT - 12;
static const int TREX_PLAYER_X = 14;
static const int TREX_SAFE_ZONE = 44;
static const int TREX_LIVES_START = 3;
static const int TREX_LIVES_MAX = 5;
static const int TREX_JUMP_MOMENTUM = 8;
static const int TREX_DUCK_JUMP_MOMENTUM = 6;
static const int TREX_JUMP_APEX_HOLD_FRAMES = 2;
static const int TREX_FALL_GRAVITY_HOLD_FRAMES = 1;
static const int TREX_MAX_FALL_SPEED = -5;
static const int TREX_SCROLL_SPEED_START = 3;
static const int TREX_SCROLL_SPEED_MAX = 6;
static const int TREX_SCROLL_SPEED_UP_EVERY = 520;
static const int TREX_RIGHT_BOOST_SPEED = 2;
static const int TREX_CACTUS_Y_OFFSET = 5;
static const bool TREX_ENABLE_MULTI_CACTI = true;
static const int TREX_MULTI_CACTI_SCORE = 650;
static const int TREX_MULTI_CACTI_CHANCE = 35;
static const bool TREX_ENABLE_NIGHT_MODE = false;
static const int TREX_RENDER_TILE = 8;

static GFXcanvas16 trexCanvas(TREX_SCREEN_W, TREX_SCREEN_H);
static uint16_t trexPreviousFrame[TREX_SCREEN_W * TREX_SCREEN_H];
static bool trexPreviousFrameValid = false;

static void trexInvalidateFrame() {
  trexPreviousFrameValid = false;
}

static void trexFlushFrame() {
  uint16_t* frame = trexCanvas.getBuffer();
  if (!trexPreviousFrameValid) {
    display.drawRGBBitmap(0, 0, frame, TREX_SCREEN_W, TREX_SCREEN_H);
    memcpy(trexPreviousFrame, frame, sizeof(trexPreviousFrame));
    trexPreviousFrameValid = true;
    return;
  }

  for (int y = 0; y < TREX_SCREEN_H; y += TREX_RENDER_TILE) {
    int h = min(TREX_RENDER_TILE, TREX_SCREEN_H - y);
    for (int x = 0; x < TREX_SCREEN_W; x += TREX_RENDER_TILE) {
      int w = min(TREX_RENDER_TILE, TREX_SCREEN_W - x);
      bool dirty = false;
      for (int row = 0; row < h && !dirty; row++) {
        int offset = (y + row) * TREX_SCREEN_W + x;
        for (int col = 0; col < w; col++) {
          if (frame[offset + col] != trexPreviousFrame[offset + col]) {
            dirty = true;
            break;
          }
        }
      }
      if (!dirty) continue;
      for (int row = 0; row < h; row++) {
        int offset = (y + row) * TREX_SCREEN_W + x;
        display.drawRGBBitmap(x, y + row, frame + offset, w, 1);
        memcpy(trexPreviousFrame + offset, frame + offset, w * sizeof(uint16_t));
      }
    }
  }
}


static const uint8_t trex_up_1s_bitmap[] PROGMEM = {
0x16, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xfe, 0xfa,
0xfe, 0xfe, 0xbe, 0xbe, 0xbe, 0x3e, 0x3c, 0x00, 0x00, 0x3f, 0x7c, 0xf8, 0xf0, 0xf0, 0xf8, 0xfc,
0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x04, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0x03, 0x3f, 0x2f, 0x07, 0x03, 0x07, 0x3f, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00
};
static const uint8_t trex_up_2s_bitmap[] PROGMEM = {
0x16, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xfe, 0xfa,
0xfe, 0xfe, 0xbe, 0xbe, 0xbe, 0x3e, 0x3c, 0x00, 0x00, 0x3f, 0x7c, 0xf8, 0xf0, 0xf0, 0xf8, 0xfc,
0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x04, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0x03, 0x3f, 0x2f, 0x07, 0x03, 0x03, 0x07, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00
};
static const uint8_t trex_up_3s_bitmap[] PROGMEM = {
0x16, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xfe, 0xfa,
0xfe, 0xfe, 0xbe, 0xbe, 0xbe, 0x3e, 0x3c, 0x00, 0x00, 0x3f, 0x7c, 0xf8, 0xf0, 0xf0, 0xf8, 0xfc,
0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x04, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0x03, 0x07, 0x0f, 0x0b, 0x03, 0x07, 0x3f, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00
};
static const uint8_t trex_duck_1s_bitmap[] PROGMEM = {
0x1d, 0x0f, 0x00, 0x0e, 0x1c, 0x3c, 0x78, 0xf8, 0xf8, 0xf8, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
0xfc, 0xfc, 0xf8, 0xf8, 0xfc, 0xfe, 0xf6, 0xfe, 0xfe, 0x7e, 0x7e, 0x7e, 0x7e, 0x7c, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x0b, 0x03, 0x3f, 0x2f, 0x07, 0x03, 0x01, 0x01, 0x07, 0x05,
0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00
};
static const uint8_t trex_duck_2s_bitmap[] PROGMEM = {
0x1d, 0x0f, 0x00, 0x0e, 0x1c, 0x3c, 0x78, 0xf8, 0xf8, 0xf8, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
0xfc, 0xfc, 0xf8, 0xf8, 0xfc, 0xfe, 0xf6, 0xfe, 0xfe, 0x7e, 0x7e, 0x7e, 0x7e, 0x7c, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x3d, 0x2f, 0x07, 0x03, 0x03, 0x07, 0x07, 0x05, 0x01, 0x07, 0x05,
0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00
};
static const uint8_t cacti_small_big_bitmap[] PROGMEM = {
0x17, 0x1a, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
0xfc, 0xfe, 0xfe, 0xfc, 0x00, 0x80, 0xc0, 0x80, 0x00, 0x00, 0x7e, 0xff, 0x80, 0xff, 0xff, 0x00,
0xfc, 0xfc, 0x00, 0x7f, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xc0, 0xff, 0x7f, 0x3f, 0x00,
0x00, 0x80, 0x00, 0x00, 0xff, 0xff, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x81, 0x81, 0xff, 0xff,
0xff, 0xff, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t cacti_big_small_bitmap[] PROGMEM = {
0x17, 0x1a, 0x00, 0x00, 0x80, 0x00, 0x00, 0xfc, 0xfe, 0xfe, 0xfc, 0x00, 0x80, 0xc0, 0x80, 0x00,
0x00, 0x00, 0x00, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xc0, 0xff, 0xff,
0xff, 0xff, 0xc0, 0xff, 0x7f, 0x3f, 0x00, 0x7e, 0xff, 0x80, 0xff, 0xff, 0x00, 0xfc, 0xfc, 0x00,
0x00, 0x00, 0x00, 0x81, 0x81, 0xff, 0xff, 0xff, 0xff, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00,
0x00, 0xff, 0xff, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00
};
static const uint8_t cacti_big_big_bitmap[] PROGMEM = {
0x1b, 0x1a, 0x00, 0x00, 0x80, 0x00, 0x00, 0xfc, 0xfe, 0xfe, 0xfc, 0x00, 0x80, 0xc0, 0x80, 0x00,
0xe0, 0xf0, 0xe0, 0x00, 0xfc, 0xfe, 0xfe, 0xfc, 0x00, 0x80, 0xc0, 0x80, 0x00, 0x00, 0x7f, 0xff,
0xff, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xc0, 0xff, 0x7f, 0x3f, 0x00, 0x0f, 0x1f, 0x3f, 0x38, 0xff,
0xff, 0xff, 0xff, 0xe0, 0xff, 0x7f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x81, 0x81, 0xff, 0xff, 0xff,
0xff, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
};
static const uint8_t cacti_small_small_small_bitmap[] PROGMEM = {
0x1b, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xf8,
0x00, 0xff, 0xff, 0x80, 0xfe, 0x7c, 0x00, 0xfe, 0x00, 0xff, 0xff, 0xff, 0x00, 0xf8, 0x00, 0x7f,
0xfe, 0x80, 0xff, 0xff, 0x00, 0xfe, 0xfc, 0x00, 0x00, 0x01, 0x03, 0x02, 0xff, 0xff, 0x00, 0x00,
0x00, 0x00, 0x03, 0x06, 0xff, 0xff, 0xff, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x01,
0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00
};
static const uint8_t pterodactyl_1_bitmap[] PROGMEM = {
0x17, 0x14, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
0x03, 0x07, 0xff, 0xff, 0xff, 0x7f, 0x3f, 0x3f, 0x3e, 0x3c, 0x3c, 0x1c, 0x14, 0x14, 0x04, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t pterodactyl_2_bitmap[] PROGMEM = {
0x17, 0x14, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf0, 0xc0, 0x0e, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3e, 0x3c, 0x3c, 0x1c, 0x14, 0x14, 0x04, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t hearts_5x_bitmap[] PROGMEM = {
0x1f, 0x05, 0x00, 0x06, 0x0f, 0x1e, 0x0f, 0x06, 0x00, 0x06, 0x0f, 0x1e, 0x0f, 0x06, 0x00, 0x06,
0x0f, 0x1e, 0x0f, 0x06, 0x00, 0x06, 0x0f, 0x1e, 0x0f, 0x06, 0x00, 0x06, 0x0f, 0x1e, 0x0f, 0x06,
0x00
};

struct TrexObstacle {
  int x;
  int y;
  int w;
  int h;
  int type;
  bool active;
};

struct TrexHeart {
  int x;
  int y;
  bool active;
  unsigned long nextSpawn;
};

static uint16_t trexLoadHiScore();
static void trexSaveHiScore(uint16_t score);
static uint8_t trexSpriteWidth(const uint8_t* bitmap);
static uint8_t trexSpriteHeight(const uint8_t* bitmap);
static void trexDrawSprite(int x, int y, const uint8_t* bitmap);
static void trexDrawSprite(int x, int y, const uint8_t* bitmap, uint8_t visibleWidth);
static void trexDrawSpriteBottom(int x, int bottomY, const uint8_t* bitmap);
static const uint8_t* trexPlayerBitmap(bool ducking, bool jumping, uint16_t score);
static const uint8_t* trexObstacleBitmap(const TrexObstacle& obstacle, bool wingUp);
static void trexDrawPlayer(int bottomY, bool ducking, bool dead, bool blinkOn, bool jumping, uint16_t score);
static void trexDrawCactusGroundCutout(const TrexObstacle& obstacle);
static void trexDrawCactus(const TrexObstacle& obstacle);
static void trexDrawPterodactyl(const TrexObstacle& obstacle, bool wingUp);
static void trexDrawHeart(int x, int y);
static void trexDrawLives(int lives);
static void trexDrawFooter(const char* leftText, const char* rightText);
static void trexDrawGround(int offset);
static bool trexRectsOverlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh);
static bool trexPlayerHitsObstacle(int bottomY, bool ducking, const TrexObstacle& obstacle);
static int trexRightmostObstacleX(TrexObstacle obstacles[], int count);
static uint8_t trexSingleCactusWidth(int type);
static int trexCactusLeftGapOffset(const TrexObstacle& obstacle);
static int trexCactusRightGapOffset(const TrexObstacle& obstacle);
static void trexSpawnObstacle(TrexObstacle obstacles[], int count, uint16_t score);
static void trexResetObstacles(TrexObstacle obstacles[], int count);
static void trexDrawGame(uint16_t score, uint16_t hiScore, int lives, int playerBottom,
                         bool ducking, bool dead, bool blinkOn, bool jumping, TrexObstacle obstacles[],
                         int obstacleCount, const TrexHeart& heart, int groundOffset,
                         bool night, bool wingUp, bool gameOver, bool paused);

static uint16_t trexLoadHiScore() {
  preferences.begin("games", true);
  uint16_t hiScore = preferences.getUShort("trexHi", 0);
  preferences.end();
  return hiScore;
}

static void trexSaveHiScore(uint16_t score) {
  preferences.begin("games", false);
  preferences.putUShort("trexHi", score);
  preferences.end();
}

static uint8_t trexSpriteWidth(const uint8_t* bitmap) {
  return pgm_read_byte(bitmap);
}

static uint8_t trexSpriteHeight(const uint8_t* bitmap) {
  return pgm_read_byte(bitmap + 1);
}

static void trexDrawSprite(int x, int y, const uint8_t* bitmap) {
  trexDrawSprite(x, y, bitmap, trexSpriteWidth(bitmap));
}

static void trexDrawSprite(int x, int y, const uint8_t* bitmap, uint8_t visibleWidth) {
  uint8_t w = trexSpriteWidth(bitmap);
  uint8_t h = trexSpriteHeight(bitmap);
  visibleWidth = min(visibleWidth, w);
  uint8_t byteRows = (h + 7) / 8;

  for (uint8_t rowByte = 0; rowByte < byteRows; rowByte++) {
    for (uint8_t sx = 0; sx < visibleWidth; sx++) {
      uint8_t col = pgm_read_byte(bitmap + 2 + rowByte * w + sx);
      for (uint8_t bit = 0; bit < 8; bit++) {
        uint8_t sy = rowByte * 8 + bit;
        if (sy >= h) break;
        if (col & (1 << bit)) trexCanvas.drawPixel(x + sx, y + sy, SSD1306_WHITE);
      }
    }
  }
}

static void trexDrawSpriteBottom(int x, int bottomY, const uint8_t* bitmap) {
  trexDrawSprite(x, bottomY - trexSpriteHeight(bitmap), bitmap);
}

static const uint8_t* trexPlayerBitmap(bool ducking, bool jumping, uint16_t score) {
  if (ducking) return ((score / 8) & 1) ? trex_duck_2s_bitmap : trex_duck_1s_bitmap;
  if (jumping) return trex_up_1s_bitmap;
  switch ((score / 8) % 3) {
    case 1: return trex_up_2s_bitmap;
    case 2: return trex_up_3s_bitmap;
    default: return trex_up_1s_bitmap;
  }
}

static const uint8_t* trexObstacleBitmap(const TrexObstacle& obstacle, bool wingUp) {
  if (obstacle.type == 10) return wingUp ? pterodactyl_1_bitmap : pterodactyl_2_bitmap;
  switch (obstacle.type) {
    case 1: return cacti_big_small_bitmap;
    case 2: return cacti_big_big_bitmap;
    case 3: return cacti_small_small_small_bitmap;
    default: return cacti_small_big_bitmap;
  }
}

static void trexDrawPlayer(int bottomY, bool ducking, bool dead, bool blinkOn, bool jumping, uint16_t score) {
  if (blinkOn) return;
  const uint8_t* bitmap = trexPlayerBitmap(ducking, jumping, score);
  trexDrawSpriteBottom(TREX_PLAYER_X, bottomY, bitmap);
  if (dead) trexCanvas.drawPixel(TREX_PLAYER_X + 14, bottomY - trexSpriteHeight(bitmap) + 4, SSD1306_BLACK);
}

static void trexDrawCactus(const TrexObstacle& obstacle) {
  trexDrawSprite(obstacle.x, obstacle.y, trexObstacleBitmap(obstacle, false), obstacle.w);
  trexDrawCactusGroundCutout(obstacle);
}

static void trexDrawPterodactyl(const TrexObstacle& obstacle, bool wingUp) {
  trexDrawSprite(obstacle.x, obstacle.y, trexObstacleBitmap(obstacle, wingUp));
}

static void trexDrawHeart(int x, int y) {
  trexCanvas.drawPixel(x + 1, y, ST77XX_RED);
  trexCanvas.drawPixel(x + 3, y, ST77XX_RED);
  trexCanvas.drawFastHLine(x, y + 1, 5, ST77XX_RED);
  trexCanvas.drawFastHLine(x, y + 2, 5, ST77XX_RED);
  trexCanvas.drawFastHLine(x + 1, y + 3, 3, ST77XX_RED);
  trexCanvas.drawPixel(x + 2, y + 4, ST77XX_RED);
}

static void trexDrawLives(int lives) {
  uint8_t visibleWidth = min(31, 6 * lives + 1);
  uint8_t h = trexSpriteHeight(hearts_5x_bitmap);
  uint8_t byteRows = (h + 7) / 8;
  int x = TREX_SCREEN_W - visibleWidth - 2;
  int y = 2;
  for (uint8_t rowByte = 0; rowByte < byteRows; rowByte++) {
    for (uint8_t sx = 0; sx < visibleWidth; sx++) {
      uint8_t col = pgm_read_byte(hearts_5x_bitmap + 2 + rowByte * 31 + sx);
      for (uint8_t bit = 0; bit < 8; bit++) {
        uint8_t sy = rowByte * 8 + bit;
        if (sy >= h) break;
        if (col & (1 << bit)) trexCanvas.drawPixel(x + sx, y + sy, ST77XX_RED);
      }
    }
  }
}

static void trexDrawFooter(const char* leftText, const char* rightText) {
  const int footerY = TREX_GROUND_Y + 1;
  trexCanvas.fillRect(0, footerY, TREX_SCREEN_W, TREX_SCREEN_H - footerY, SSD1306_BLACK);
  trexCanvas.setTextSize(1);
  trexCanvas.setTextColor(UI_FOOTER_COLOR);
  trexCanvas.setCursor(0, footerY + 2);
  trexCanvas.print(leftText);
  int rightX = TREX_SCREEN_W - strlen(rightText) * 6;
  trexCanvas.setCursor(max(0, rightX), footerY + 2);
  trexCanvas.print(rightText);
  trexCanvas.setTextColor(SSD1306_WHITE);
}


static void trexDrawGround(int offset) {
  trexCanvas.drawFastHLine(0, TREX_GROUND_Y, TREX_SCREEN_W, UI_SEPARATOR_COLOR);
  for (int x = -offset; x < TREX_SCREEN_W; x += 16) {
    trexCanvas.drawPixel(x + 3, TREX_GROUND_Y + 3, UI_SEPARATOR_COLOR);
    trexCanvas.drawFastHLine(x + 9, TREX_GROUND_Y + 5, 4, UI_SEPARATOR_COLOR);
  }
}

static void trexDrawCactusGroundCutout(const TrexObstacle& obstacle) {
  trexCanvas.drawPixel(constrain(obstacle.x + trexCactusLeftGapOffset(obstacle), 0, TREX_SCREEN_W - 1), TREX_GROUND_Y, SSD1306_BLACK);
  trexCanvas.drawPixel(constrain(obstacle.x + trexCactusRightGapOffset(obstacle), 0, TREX_SCREEN_W - 1), TREX_GROUND_Y, SSD1306_BLACK);
}

static bool trexRectsOverlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
  return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

static bool trexPlayerHitsObstacle(int bottomY, bool ducking, const TrexObstacle& obstacle) {
  int px;
  int py;
  int pw;
  int ph;
  if (ducking) {
    px = TREX_PLAYER_X + 2;
    py = bottomY - 11;
    pw = 25;
    ph = 10;
  } else {
    px = TREX_PLAYER_X + 3;
    py = bottomY - 21;
    pw = 16;
    ph = 20;
  }

  int ox;
  int oy;
  int ow;
  int oh;
  if (obstacle.type == 10) {
    ox = obstacle.x + 3;
    oy = obstacle.y + 4;
    ow = max(1, obstacle.w - 6);
    oh = 8;
  } else {
    ox = obstacle.x + 2;
    oy = obstacle.y + 3;
    ow = max(1, obstacle.w - 4);
    oh = max(1, obstacle.h - 5);
  }
  return trexRectsOverlap(px, py, pw, ph, ox, oy, ow, oh);
}

static int trexRightmostObstacleX(TrexObstacle obstacles[], int count) {
  int rightmost = -TREX_SAFE_ZONE;
  for (int i = 0; i < count; i++) {
    if (obstacles[i].active) rightmost = max(rightmost, obstacles[i].x + obstacles[i].w);
  }
  return rightmost;
}

static uint8_t trexSingleCactusWidth(int type) {
  switch (type) {
    case 1: return 14;
    case 2: return 14;
    case 3: return 18;
    default: return 10;
  }
}

static int trexCactusLeftGapOffset(const TrexObstacle& obstacle) {
  if (obstacle.type == 10) return obstacle.w / 2 - 2;
  switch (obstacle.type) {
    case 1: return 4;
    case 2: return 4;
    case 3: return 6;
    default: return 3;
  }
}

static int trexCactusRightGapOffset(const TrexObstacle& obstacle) {
  if (obstacle.type == 10) return obstacle.w / 2 + 2;
  switch (obstacle.type) {
    case 1: return 9;
    case 2: return 9;
    case 3: return 11;
    default: return 6;
  }
}

static void trexSpawnObstacle(TrexObstacle obstacles[], int count, uint16_t score) {
  if (trexRightmostObstacleX(obstacles, count) > TREX_SCREEN_W - TREX_SAFE_ZONE) return;
  if (random(100) > 8) return;

  for (int i = 0; i < count; i++) {
    if (obstacles[i].active) continue;

    bool bird = score > 250 && random(100) < 22;
    bool multiCactus = TREX_ENABLE_MULTI_CACTI &&
                       score > TREX_MULTI_CACTI_SCORE &&
                       random(100) < TREX_MULTI_CACTI_CHANCE;
    obstacles[i].type = bird ? 10 : (multiCactus ? random(4) : random(2));
    obstacles[i].active = true;
    obstacles[i].x = TREX_SCREEN_W + random(8, 34);
    const uint8_t* bitmap = trexObstacleBitmap(obstacles[i], false);
    obstacles[i].h = trexSpriteHeight(bitmap);
    if (bird) {
      obstacles[i].w = trexSpriteWidth(bitmap);
      int positions[] = {TREX_HUD_H + 8, TREX_HUD_H + 18, TREX_HUD_H + 18, TREX_HUD_H + 28};
      obstacles[i].y = positions[random(4)];
    } else {
      obstacles[i].w = multiCactus ? trexSpriteWidth(bitmap) : trexSingleCactusWidth(obstacles[i].type);
      obstacles[i].y = TREX_GROUND_Y - obstacles[i].h + TREX_CACTUS_Y_OFFSET;
    }
    return;
  }
}

static void trexResetObstacles(TrexObstacle obstacles[], int count) {
  for (int i = 0; i < count; i++) {
    obstacles[i].active = false;
    obstacles[i].x = -32;
    obstacles[i].y = 0;
    obstacles[i].w = 0;
    obstacles[i].h = 0;
    obstacles[i].type = 0;
  }
}

static void trexDrawGame(uint16_t score, uint16_t hiScore, int lives, int playerBottom,
                         bool ducking, bool dead, bool blinkOn, bool jumping, TrexObstacle obstacles[],
                         int obstacleCount, const TrexHeart& heart, int groundOffset,
                         bool night, bool wingUp, bool gameOver, bool paused) {
  display.invertDisplay(TREX_ENABLE_NIGHT_MODE && night);
  trexCanvas.fillRect(0, 0, TREX_SCREEN_W, TREX_HUD_H, SSD1306_BLACK);
  trexCanvas.fillRect(0, TREX_HUD_H, TREX_SCREEN_W, TREX_SCREEN_H - TREX_HUD_H, SSD1306_BLACK);
  trexCanvas.setTextSize(1);

  trexCanvas.setTextColor(UI_HEADER_COLOR);
  trexCanvas.setCursor(0, 1);
  char scoreText[6];
  snprintf(scoreText, sizeof(scoreText), "%05u", score);
  trexCanvas.print(scoreText);
  trexCanvas.setTextColor(UI_SEPARATOR_COLOR);
  trexCanvas.setCursor(42, 1);
  trexCanvas.print("HI");
  snprintf(scoreText, sizeof(scoreText), "%05u", hiScore);
  trexCanvas.setCursor(58, 1);
  trexCanvas.print(scoreText);
  trexCanvas.setTextColor(SSD1306_WHITE);
  trexDrawLives(lives);

  trexDrawGround(groundOffset);
  for (int i = 0; i < obstacleCount; i++) {
    if (!obstacles[i].active) continue;
    if (obstacles[i].type == 10) trexDrawPterodactyl(obstacles[i], wingUp);
    else trexDrawCactus(obstacles[i]);
  }
  if (heart.active) trexDrawHeart(heart.x, heart.y);

  trexDrawPlayer(playerBottom, ducking, dead, blinkOn, jumping, score);

  if (paused || gameOver) {
    const int boxW = gameOver ? 78 : 52;
    const int boxH = 22;
    const int boxX = (TREX_SCREEN_W - boxW) / 2;
    const int boxY = (TREX_SCREEN_H - TREX_FOOTER_H - boxH) / 2;
    trexCanvas.fillRect(boxX, boxY, boxW, boxH, SSD1306_BLACK);
    trexCanvas.drawRect(boxX, boxY, boxW, boxH, UI_SEPARATOR_COLOR);
    trexCanvas.setTextColor(SSD1306_WHITE);
    if (gameOver) {
      trexCanvas.setCursor(boxX + 12, boxY + 7);
      trexCanvas.print("GAME OVER");
      trexDrawFooter("LB - exit", "RB - play again");
    } else {
      trexCanvas.setCursor(boxX + 11, boxY + 7);
      trexCanvas.print("Pause");
      trexDrawFooter("LB - exit", "continue - RB");
    }
  }

  trexCanvas.setTextColor(SSD1306_WHITE);
  trexFlushFrame();
}

void playTrexGame() {
  trexInvalidateFrame();
  uint16_t hiScore = trexLoadHiScore();
  uint16_t score = 0;
  int lives = TREX_LIVES_START;
  int playerBottom = TREX_GROUND_Y;
  int velocityY = 0;
  bool ducking = false;
  bool gameOver = false;
  bool paused = false;
  bool night = false;
  bool jumpWasPressed = true;
  bool rbWasPressed = true;
  int apexHoldFrames = 0;
  int fallGravityHoldFrames = 0;
  unsigned long lastFrame = millis();
  unsigned long blinkUntil = 0;
  unsigned long nextNightSwitch = 0;
  int groundOffset = 0;
  bool hiScoreDirty = false;

  const int obstacleCount = 4;
  TrexObstacle obstacles[obstacleCount];
  TrexHeart heart = {-16, TREX_HUD_H + 14, false, millis() + 16000};
  trexResetObstacles(obstacles, obstacleCount);

  while (true) {
    JoystickData input = readJoysticks();
    bool jumpPressed = input.ly > 50 || input.ry > 50;
    bool duckPressed = input.lb || input.ly < -50 || input.ry < -50;
    bool speedBoost = input.lx > 50 || input.rx > 50;

    if (input.lb && input.rb) {
      noTone(BUZZER_PIN);
      if (hiScoreDirty) trexSaveHiScore(hiScore);
      display.invertDisplay(false);
      display.setTextColor(SSD1306_WHITE);
      display.clearDisplay();
      trexInvalidateFrame();
      return;
    }

    if (paused) {
      trexDrawGame(score, hiScore, lives, playerBottom, ducking, false, false,
                   playerBottom < TREX_GROUND_Y, obstacles, obstacleCount, heart, groundOffset, night,
                   (millis() / 180) % 2, false, true);
      if (input.lb) {
        noTone(BUZZER_PIN);
        if (hiScoreDirty) trexSaveHiScore(hiScore);
        display.invertDisplay(false);
        display.setTextColor(SSD1306_WHITE);
        display.clearDisplay();
        trexInvalidateFrame();
        return;
      }
      if (input.rb && !rbWasPressed) {
        paused = false;
        rbWasPressed = true;
        lastFrame = millis();
        if (sound) tone(BUZZER_PIN, OK_TONE, 40);
        delay(120);
      } else {
        rbWasPressed = input.rb;
        delay(50);
      }
      continue;
    }

    if (gameOver) {
      trexDrawGame(score, hiScore, lives, playerBottom, ducking, true, false,
                   false, obstacles, obstacleCount, heart, groundOffset, night,
                   (millis() / 180) % 2, true, false);

      if (input.lb) {
        noTone(BUZZER_PIN);
        if (hiScoreDirty) trexSaveHiScore(hiScore);
        display.invertDisplay(false);
        display.setTextColor(SSD1306_WHITE);
        display.clearDisplay();
        trexInvalidateFrame();
        return;
      }
      if (input.rb && !rbWasPressed) {
        score = 0;
        lives = TREX_LIVES_START;
        playerBottom = TREX_GROUND_Y;
        velocityY = 0;
        apexHoldFrames = 0;
        fallGravityHoldFrames = 0;
        ducking = false;
        gameOver = false;
        paused = false;
        night = false;
        blinkUntil = 0;
        nextNightSwitch = millis() + 18000;
        heart = {-16, TREX_HUD_H + 14, false, millis() + 16000};
        trexResetObstacles(obstacles, obstacleCount);
        trexInvalidateFrame();
        if (sound) tone(BUZZER_PIN, OK_TONE, 50);
        hiScoreDirty = false;
        rbWasPressed = true;
        delay(220);
      } else {
        rbWasPressed = input.rb;
      }
      delay(50);
      continue;
    }

    if (input.rb && !rbWasPressed) {
      paused = true;
      rbWasPressed = true;
      if (sound) tone(BUZZER_PIN, BACK_TONE, 40);
      continue;
    }
    rbWasPressed = input.rb;

    unsigned long now = millis();
    int frameDelay = max(21, 43 - (int)(score / 256));
    if (now - lastFrame < (unsigned long)frameDelay) {
      delay(1);
      continue;
    }
    lastFrame = now;

    bool onGround = playerBottom >= TREX_GROUND_Y && velocityY == 0;
    ducking = duckPressed && onGround;
    if (jumpPressed && !jumpWasPressed && onGround) {
      velocityY = ducking ? TREX_DUCK_JUMP_MOMENTUM : TREX_JUMP_MOMENTUM;
      apexHoldFrames = 0;
      fallGravityHoldFrames = 0;
      ducking = false;
      if (sound) tone(BUZZER_PIN, 1200, 40);
    }
    jumpWasPressed = jumpPressed;

    if (!onGround || velocityY > 0) {
      playerBottom -= velocityY;
      if (abs(velocityY) <= 1 && apexHoldFrames < TREX_JUMP_APEX_HOLD_FRAMES) {
        apexHoldFrames++;
      } else if (velocityY < 0 && fallGravityHoldFrames < TREX_FALL_GRAVITY_HOLD_FRAMES) {
        fallGravityHoldFrames++;
      } else {
        apexHoldFrames = 0;
        fallGravityHoldFrames = 0;
        velocityY--;
      }
      if (velocityY < TREX_MAX_FALL_SPEED) velocityY = TREX_MAX_FALL_SPEED;
      if (duckPressed && velocityY < 2) velocityY -= 2;
      int minBottom = trexSpriteHeight(trex_up_1s_bitmap);
      if (playerBottom < minBottom) {
        playerBottom = minBottom;
        velocityY = min(velocityY, 0);
      }
      if (playerBottom >= TREX_GROUND_Y) {
        playerBottom = TREX_GROUND_Y;
        velocityY = 0;
      }
    }

    int baseScrollSpeed = min(TREX_SCROLL_SPEED_MAX, TREX_SCROLL_SPEED_START + (int)(score / TREX_SCROLL_SPEED_UP_EVERY));
    int scrollSpeed = baseScrollSpeed + (speedBoost ? TREX_RIGHT_BOOST_SPEED : 0);
    groundOffset = (groundOffset + scrollSpeed) % 96;
    for (int i = 0; i < obstacleCount; i++) {
      if (!obstacles[i].active) continue;
      obstacles[i].x -= scrollSpeed + (obstacles[i].type == 10 ? 1 : 0);
      if (obstacles[i].x + obstacles[i].w < 0) obstacles[i].active = false;
    }
    trexSpawnObstacle(obstacles, obstacleCount, score);

    if (!heart.active && lives < TREX_LIVES_MAX && now > heart.nextSpawn) {
      heart.active = true;
      heart.x = TREX_SCREEN_W + random(16, 48);
      heart.y = random(TREX_HUD_H + 8, TREX_GROUND_Y - 12);
    }
    if (heart.active) {
      heart.x -= scrollSpeed;
      if (heart.x < -8) {
        heart.active = false;
        heart.nextSpawn = now + random(14000, 24000);
      }
    }

    bool blinking = now < blinkUntil;
    for (int i = 0; i < obstacleCount; i++) {
      if (!obstacles[i].active || blinking) continue;
      if (trexPlayerHitsObstacle(playerBottom, ducking, obstacles[i])) {
        if (lives > 0) {
          lives--;
          blinkUntil = now + 1200;
          if (sound) tone(BUZZER_PIN, 220, 180);
        } else {
          gameOver = true;
          if (hiScoreDirty) {
            trexSaveHiScore(hiScore);
            hiScoreDirty = false;
          }
          if (sound) tone(BUZZER_PIN, 160, 500);
        }
        break;
      }
    }

    if (heart.active && lives < TREX_LIVES_MAX) {
      const uint8_t* playerBitmap = trexPlayerBitmap(ducking, playerBottom < TREX_GROUND_Y, score);
      int playerY = playerBottom - trexSpriteHeight(playerBitmap);
      if (trexRectsOverlap(TREX_PLAYER_X, playerY, trexSpriteWidth(playerBitmap), trexSpriteHeight(playerBitmap),
                           heart.x, heart.y, 5, 5)) {
        lives++;
        heart.active = false;
        heart.nextSpawn = now + random(16000, 26000);
        if (sound) tone(BUZZER_PIN, 1500, 70);
      }
    }

    score++;
    if (score > hiScore) {
      hiScore = score;
      hiScoreDirty = true;
    }
    if (TREX_ENABLE_NIGHT_MODE && nextNightSwitch == 0) nextNightSwitch = now + 18000;
    if (TREX_ENABLE_NIGHT_MODE && now > nextNightSwitch) {
      night = !night;
      nextNightSwitch = now + 18000;
    }

    trexDrawGame(score, hiScore, lives, playerBottom, ducking, false,
                 blinking && ((now / 90) % 2 == 0), playerBottom < TREX_GROUND_Y, obstacles, obstacleCount,
                 heart, groundOffset, night, (score / 8) % 2, false, false);
  }
}
