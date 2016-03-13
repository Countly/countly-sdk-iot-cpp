#include "Countly.h"
#include <math.h>


const int thresholdvalue = 10;
float Rsensor;

Countly countly = Countly("SERVER_URL", "APP_KEY");

void setup() {
	Serial.begin(9600);
	countly.init();
}

void loop() {
	int sensorValue = analogRead(0);
	Rsensor = (float) (1023 - sensorValue) * 10 / sensorValue;
	countly.metrics();
	delay(3000);
	countly.event("Light", Rsensor);
	delay(3000);
}
