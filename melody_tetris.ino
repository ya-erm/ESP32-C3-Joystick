// Tetris theme (Korobeiniki). https://github.com/robsoncouto/arduino-songs

static const int melodyTetris[] PROGMEM = {
  NOTE_E5, 4, NOTE_B4, 8, NOTE_C5, 8, NOTE_D5, 4, NOTE_C5, 8, NOTE_B4, 8,
  NOTE_A4, 4, NOTE_A4, 8, NOTE_C5, 8, NOTE_E5, 4, NOTE_D5, 8, NOTE_C5, 8,
  NOTE_B4, -4, NOTE_C5, 8, NOTE_D5, 4, NOTE_E5, 4,
  NOTE_C5, 4, NOTE_A4, 4, NOTE_A4, 4, REST, 4,

  REST, 8, NOTE_D5, 4, NOTE_F5, 8, NOTE_A5, 4, NOTE_G5, 8, NOTE_F5, 8,
  NOTE_E5, -4, NOTE_C5, 8, NOTE_E5, 4, NOTE_D5, 8, NOTE_C5, 8,
  NOTE_B4, 4, NOTE_B4, 8, NOTE_C5, 8, NOTE_D5, 4, NOTE_E5, 4,
  NOTE_C5, 4, NOTE_A4, 4, NOTE_A4, 4, REST, 4,

  NOTE_E5, 2, NOTE_C5, 2,
  NOTE_D5, 2, NOTE_B4, 2,
  NOTE_C5, 2, NOTE_A4, 2,
  NOTE_B4, 1,

  NOTE_E5, 2, NOTE_C5, 2,
  NOTE_D5, 2, NOTE_B4, 2,
  NOTE_C5, 4, NOTE_E5, 4, NOTE_A5, 2,
  NOTE_GS5, 1,

  NOTE_E5, 4, NOTE_B4, 8, NOTE_C5, 8, NOTE_D5, 4, NOTE_C5, 8, NOTE_B4, 8,
  NOTE_A4, 4, NOTE_A4, 8, NOTE_C5, 8, NOTE_E5, 4, NOTE_D5, 8, NOTE_C5, 8,
  NOTE_B4, -4, NOTE_C5, 8, NOTE_D5, 4, NOTE_E5, 4,
  NOTE_C5, 4, NOTE_A4, 4, NOTE_A4, 4, REST, 4,

  REST, 8, NOTE_D5, 4, NOTE_F5, 8, NOTE_A5, 4, NOTE_G5, 8, NOTE_F5, 8,
  REST, 8, NOTE_E5, 4, NOTE_C5, 8, NOTE_E5, 4, NOTE_D5, 8, NOTE_C5, 8,
  REST, 8, NOTE_B4, 4, NOTE_C5, 8, NOTE_D5, 4, NOTE_E5, 4,
  REST, 8, NOTE_C5, 4, NOTE_A4, 8, NOTE_A4, 4, REST, 4,
};

void playTetrisMelody() {
  playMelodyData(144, melodyTetris, sizeof(melodyTetris) / sizeof(melodyTetris[0]) / 2);
}
