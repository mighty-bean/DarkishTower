/**************************************************************************
  DARKISH TOWER
  An homage to the Dark Tower board game.

  Greg Snook
  Mighty Studios, 2021
 **************************************************************************/

#include "src/game_state.h"

void setup(void) {
	Serial.begin(115200);
	Serial.print(F("Starting Darkish Tower"));

	// enable the DAC
	pinMode(PIN_DAC1, ANALOG); 

	// turn on secondary power (the speaker)
	pinMode(21, OUTPUT);
	digitalWrite(21, HIGH);

	gGameState.setup();
}

void loop() {
	gGameState.update();
	usleep(10 * 1000);  
}
