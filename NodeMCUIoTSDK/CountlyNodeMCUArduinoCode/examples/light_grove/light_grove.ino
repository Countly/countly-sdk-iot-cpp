#include "Countly.h"

float Rsensor;

Countly	countly = Countly("SERVER_URL", "APP_KEY", "SSID", "SSID_PASSWORD");

void setup() {
	Serial.begin(9600);
    EEPROM.begin(512);
	countly.init();
}

void loop() {
	int sensorValue = analogRead(0);
	Rsensor = (float) (1023 - sensorValue) * 10 / sensorValue;
	countly.event(F("Light"), Rsensor);
	delay(3000);
}
