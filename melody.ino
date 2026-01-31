
void playMelody(int id) {
    if (id == 0) return; // OFF
    
    int melody[20];
    int melodySize;
    int duration;
    int durations[20]; // для разных длительностей нот
    
    switch(id) {
      case 1:
        {
          const int m[] = { 659, 659,   0, 659,   0, 523, 659,   0, 784,   0,   0, 392};
          const int d[] = { 150, 150, 150, 150, 150, 150, 150, 300, 300, 150, 150, 150};
          melodySize = sizeof(m)/sizeof(m[0]);
          for (int i = 0; i < melodySize; i++) {
            melody[i] = m[i];
            durations[i] = d[i];
          }
          duration = 0; // используем индивидуальные длительности
        }
        break;
      case 2:
        playNokiaMelody();
        return;
      default:
        return;
    }
  
    for (int i = 0; i < melodySize; i++) {
      if (melody[i] == 0) {
        delay(duration == 0 ? durations[i] : duration);
      } else {
        int noteDuration = (duration == 0) ? durations[i] : duration;
        tone(BUZZER_PIN, melody[i], noteDuration);
        delay(noteDuration * 1.3);
      }
    }
  }