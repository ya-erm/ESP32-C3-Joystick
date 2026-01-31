// Воспроизведение мелодии по массиву: пары (частота, длительность).
// Длительность: 4 = четверть, 8 = восьмая, отрицательная = с точкой.
void playMelodyData(int tempo, const int* melody, int notes) {
  int wholenote = (60000 * 4) / tempo;
  int divider, noteDuration;

  for (int i = 0; i < notes; i++) {
    int pitch = melody[i * 2];
    divider = melody[i * 2 + 1];
    if (divider > 0) {
      noteDuration = wholenote / divider;
    } else if (divider < 0) {
      noteDuration = wholenote / abs(divider);
      noteDuration = (int)(noteDuration * 1.5);
    } else {
      continue;
    }

    unsigned long noteEndTime = millis() + (unsigned long)noteDuration;

    if (pitch == REST) {
      while (millis() < noteEndTime) {
        if (shouldStopMusic()) return;
      }
    } else {
      tone(BUZZER_PIN, pitch, (int)(noteDuration * 0.9));
      while (millis() < noteEndTime) {
        if (shouldStopMusic()) {
          noTone(BUZZER_PIN);
          return;
        }
      }
      noTone(BUZZER_PIN);
    }
  }
  noTone(BUZZER_PIN);
}

void playMelody(int id) {
  if (id == 0) return;

  int melody[20];
  int melodySize;
  int duration;
  int durations[20];

  switch (id) {
    case 1: {
      const int m[] = { 659, 659, 0, 659, 0, 523, 659, 0, 784, 0, 0, 392 };
      const int d[] = { 150, 150, 150, 150, 150, 150, 150, 300, 300, 150, 150, 150 };
      melodySize = sizeof(m) / sizeof(m[0]);
      for (int i = 0; i < melodySize; i++) {
        melody[i] = m[i];
        durations[i] = d[i];
      }
      duration = 0;
      break;
    }
    case 2:
      playNokiaMelody();
      return;
    default:
      return;
  }

  unsigned long noteEndTime;
  for (int i = 0; i < melodySize; i++) {
    int noteDuration = (duration == 0) ? durations[i] : duration;

    if (melody[i] == 0) {
      noteEndTime = millis() + (unsigned long)noteDuration;
      while (millis() < noteEndTime) {
        if (shouldStopMusic()) return;
      }
    } else {
      tone(BUZZER_PIN, melody[i], noteDuration);
      noteEndTime = millis() + (unsigned long)(noteDuration * 1.3);
      while (millis() < noteEndTime) {
        if (shouldStopMusic()) {
          noTone(BUZZER_PIN);
          return;
        }
      }
    }
  }
  noTone(BUZZER_PIN);
}
