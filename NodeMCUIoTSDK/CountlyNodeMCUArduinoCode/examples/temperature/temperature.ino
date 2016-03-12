#include <Bridge.h>
#include "Countly.h"

Countly countly = Countly("SERVER_URL", "APP_KEY", "SSID", "SSID_PASSWORD");

void setup() {
	Serial.begin(9600);
	countly.connectWifi();
}

void loop() {
	countly.event(F("Temperature"), 25);
	delay(2000);
}
