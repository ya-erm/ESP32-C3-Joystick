bool shouldStopMusic() {
    JoystickData data = readJoysticks();
    return data.lb || data.ry > 50 || data.ry < -50 || data.ly > 50 || data.ly < -50;
}

void playMusic(int id) {
    switch(id) {
      case 0:
        playHarryPotterMelody();
        break;
      case 1:
        playStarWarsMelody();
        break;
      case 2:
        playPacmanMelody();
        break;
      case 3:
        playMarioMelody();
        break;
      case 4:
        playTetrisMelody();
        break;
    }
}