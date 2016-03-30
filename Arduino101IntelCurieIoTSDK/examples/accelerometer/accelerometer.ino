#include "Countly.h"
#include "CurieIMU.h"

IPAddress ipAddress(192, 168, 1, 177);
Countly countly = Countly("SERVER_URL", "APP_KEY", ipAddress);

void setup() {
	Serial.begin(9600);
	countly.init();
	CurieIMU.begin();
	CurieIMU.setAccelerometerRange(2);
}

void loop() {
	int x = CurieIMU.readAccelerometer(X_AXIS);
	int y = CurieIMU.readAccelerometer(Y_AXIS);
	int z = CurieIMU.readAccelerometer(Z_AXIS);
	countly.event("X_AXIS", x);
	countly.event("Y_AXIS", y);
	countly.event("Z_AXIS", z);
	delay(2000);
}
