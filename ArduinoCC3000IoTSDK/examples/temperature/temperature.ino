#include "Countly.h"

Countly countly = Countly("SERVER_URL", "APP_KEY", "SSID", "SSID_PASSWORD");

void setup() {
	Serial.begin(9600);
	countly.init();
}

void loop() {
	countly.metrics();
	delay(1000);
	countly.event(F("Temperature"), 25);
	delay(2000);
}
