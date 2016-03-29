#include "Countly.h"


String mUrlString, mAppKey;
const char *deviceId;
const int BUFSIZE = 32;
char buf[BUFSIZE];
EepromUtil eepromUtil;
const String appVersion = "0.0.1";
EthernetClient client;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress dnsServer(8, 8, 8, 8);
IPAddress ip(192, 168, 1, 177);
long randomDigit;
int isEthernetConnected = 0;
char randomCharset[33]="0123456789ABCDEFHIJKLMNOPRSTUVYZ";

Countly::Countly(String urlStr, String appKey,IPAddress ipAddress) {
	mUrlString = urlStr;
	mAppKey = appKey;
	ip=ipAddress;
}

void Countly::init() {
	deviceId = getUuidFromEeprom();
	bool isDeviceIdValid = false;
	for (int i = 0; i < sizeof(randomCharset) - 1; i++) {
		if (deviceId[0] == randomCharset[i]) {
			isDeviceIdValid = true;
		}
	}
	if (isDeviceIdValid == false) {
		eepromUtil.eeprom_erase_all();
		const char* uuid = generateUuid();
		writeUuidToEeprom(uuid, BUFSIZE);
		deviceId = getUuidFromEeprom();
	} else {
		deviceId = getUuidFromEeprom();
	}
	Ethernet.begin(mac, ip, dnsServer);
}

void Countly::metrics() {
	String metricApiUrl = "/i";

	String metricPostParam = "begin_session=1&app_key=" + mAppKey
			+ "&device_id=" + String(deviceId) + "&metrics={\"_app_version\":\""
			+ appVersion + "\",\"_device\":\"Arduino101\",\"_os\":\"Intel Curie\"}";

	Serial.println(metricPostParam);

	String returnString;

	delay(1000);

	if (client.connect(mUrlString.c_str(), 80)) {
		Serial.println("connected");
		client.println("POST " + metricApiUrl + " HTTP/1.1");
		client.println("Host: " + mUrlString);
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.println("Connection: close");
		client.println("User-Agent: Arduino101 Intel Curie Ethernet/Countly");
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

	String eventApiUrl = "/i";

	String eventPostParam = "end_session=1&app_key=" + mAppKey + "&device_id="
			+ String(deviceId) + "&events=[{\"count\":1,\"sum\":" + sum
			+ ",\"key\":\"" + key + "\"}]";

	Serial.println(eventPostParam);

	String returnString;

	if (client.connect(mUrlString.c_str(), 80)) {
		Serial.println("connected");
		client.println("POST " + eventApiUrl + " HTTP/1.1");
		client.println("Host: " + mUrlString);
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.println("Connection: close");
		client.println("User-Agent: Arduino101 Intel Curie Ethernet/Countly");
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
		uuidString += randomCharset[randomDigit];
	}
	uuidString.toCharArray(buf, BUFSIZE);
	char* retChar = (char*) buf;
	return retChar;
}

const char* Countly::getUuidFromEeprom() {
	eepromUtil.eeprom_read_string(100, buf, BUFSIZE);
	return buf;
}

bool Countly::writeUuidToEeprom(String uuid, int len) {
	eepromUtil.eeprom_erase_all();
	uuid.toCharArray(buf, BUFSIZE);
	char* bufferStr = (char*) buf;
	eepromUtil.eeprom_write_string(100, bufferStr);
	return true;
}
