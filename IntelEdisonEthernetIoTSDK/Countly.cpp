#include "Countly.h"
#include "HardwareSerial.h"
#include <Ethernet.h>

const char* mUrlString;
const char* mAppKey;
const char* deviceId;
const int BUFSIZE = 32;
char buf[BUFSIZE];
EepromUtil eepromUtil;
const String appVersion = "0.0.1";
EthernetClient client;
int isEthernetConnected = 0;
long randomDigit;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

Countly::Countly(const char* urlStr, const char* appKey) {
	mUrlString = urlStr;
	mAppKey = appKey;
	deviceId = getUuidFromEeprom();
	Serial.print("DEVICE ID");
	Serial.println(deviceId);

	if (deviceId == "") {
		eepromUtil.eeprom_erase_all();
		const char* uuid = generateUuid();
		writeUuidToEeprom(uuid, BUFSIZE);
		deviceId = uuid;
	} else {
		deviceId = getUuidFromEeprom();
	}
}

void Countly::metrics() {
	if (isEthernetConnected == 0) {
		isEthernetConnected = Ethernet.begin(mac);
		if (isEthernetConnected == 0) {
			Serial.println("Failed to configure Ethernet using DHCP");
		}
	}

	String metricApiUrl = "/i";

	String metricPostParam = "begin_session=1&app_key=" + String(mAppKey)
			+ "&device_id=" + String(deviceId)
			+ "&metrics={\"_app_version\":\"" + appVersion
			+ "\",\"_device\":\"IntelEdison\",\"_os\":\"ArduinoEthernet\"}";

	Serial.println(metricPostParam);

	String returnString;

	delay(1000);

	if (client.connect(mUrlString, 80)) {
		Serial.println("connected");
		client.println("POST " + metricApiUrl + " HTTP/1.1");
		client.print("Host: ");
		client.println(mUrlString);
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.println("Connection: close");
		client.println("User-Agent: Intel Edison Ethernet/Countly");
		client.print("Content-Length: ");
		client.println(metricPostParam.length());
		client.println();
		client.print(metricPostParam);
		client.println();
	}

	if (client.available()) {
		char c = client.read();
		returnString += c;
	}
	client.flush();
	client.stop();

	Serial.println(returnString);
}

void Countly::event(String key, int sum) {
	metrics();

	if (isEthernetConnected == 0) {
		isEthernetConnected = Ethernet.begin(mac);
		if (isEthernetConnected == 0) {
			Serial.println("Failed to configure Ethernet using DHCP");
		}
	}

	String timeStamp = "1455598239";
	String eventApiUrl = "/i";

	String eventPostParam = "end_session=1&app_key=" + String(mAppKey)
			+ "&device_id=" + String(deviceId)
			+ "&events=[{\"count\":1,\"sum\":" + sum + ",\"key\":\"" + key
			+ "\"}]";

	Serial.println(eventPostParam);

	String returnString;

	if (client.connect(mUrlString, 80)) {
		Serial.println("connected");
		client.println("POST " + eventApiUrl + " HTTP/1.1");
		client.print("Host: ");
		client.println(mUrlString);
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.println("Connection: close");
		client.println("User-Agent: Intel Edison Ethernet/Countly");
		client.print("Content-Length: ");
		client.println(eventPostParam.length());
		client.println();
		client.print(eventPostParam);
		client.println();
	}

	if (client.available()) {
		char c = client.read();
		returnString += c;
	}

	client.flush();
	client.stop();

	Serial.println(returnString);
}

const char* Countly::generateUuid() {
	randomSeed(analogRead(0));
	int i;
	String uuidString;
	for (i = 0; i < BUFSIZE; i++) {
		randomDigit = random(BUFSIZE);
		uuidString += "0123456789ABCDEFHIJKLMNOPRSTUVYZ"[randomDigit];
	}
	uuidString.toCharArray(buf, BUFSIZE);
	char* retChar = (char*) buf;
	return retChar;
}

const char* Countly::getUuidFromEeprom() {
	eepromUtil.eeprom_read_string(BUFSIZE, buf, BUFSIZE);
	return buf;
}

bool Countly::writeUuidToEeprom(String uuid, int len) {
	eepromUtil.eeprom_erase_all();
	uuid.toCharArray(buf, BUFSIZE);
	char* bufferStr = (char*) buf;
	eepromUtil.eeprom_write_string(BUFSIZE, bufferStr);
	return true;
}
