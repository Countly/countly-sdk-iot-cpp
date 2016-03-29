#include "Countly.h"

float Rsensor;
IPAddress ipAddress(192, 168, 1, 177);
Countly countly = Countly("SERVER_URL", "APP_KEY", ipAddress);

void setup() {
	Serial.begin(9600);
	countly.init();
}

void loop() {
	int sensorValue = analogRead(0);
	Rsensor = (float) (1023 - sensorValue) * 10 / sensorValue;
	countly.event("Light", Rsensor);
	delay(3000);
}
