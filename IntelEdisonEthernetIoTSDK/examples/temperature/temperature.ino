#include <Bridge.h>
#include "Countly.h"

Countly countly = Countly("SERVER_URL", "APP_KEY");

void setup() {
	Serial.begin(9600);
}

void loop() {
	countly.event("Temperature", 25);
	delay(2000);
}
