// Nokia Tune. https://github.com/robsoncouto/arduino-songs

static const int melodyNokia[] = {
  NOTE_E5, 8, NOTE_D5, 8, NOTE_FS4, 4, NOTE_GS4, 4,
  NOTE_CS5, 8, NOTE_B4, 8, NOTE_D4, 4, NOTE_E4, 4,
  NOTE_B4, 8, NOTE_A4, 8, NOTE_CS4, 4, NOTE_E4, 4,
  NOTE_A4, 2,
};

void playNokiaMelody() {
  playMelodyData(200, melodyNokia, sizeof(melodyNokia) / sizeof(melodyNokia[0]) / 2);
}
