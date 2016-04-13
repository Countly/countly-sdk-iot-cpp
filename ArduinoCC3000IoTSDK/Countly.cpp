#include "Countly.h"

const char* mUrlString;
const char* mAppKey;
const char* deviceId;
const int BUFSIZE = 32;
char buf[BUFSIZE];
EepromUtil eepromUtil;
const String appVersion = "0.0.1";
char* mSsid;
char* mPass;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
long randomDigit;
int status = WL_IDLE_STATUS;
unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 10L * 1000L;
unsigned long lastConnectionEventsTime = 0;
char randomCharset[33] = "0123456789ABCDEFHIJKLMNOPRSTUVYZ";
uint32_t ip;

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS,
ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SECURITY   WLAN_SEC_WPA2
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

Adafruit_CC3000_Client client;

Countly::Countly(const char* urlStr, const char* appKey, char* ssid,
		char* pass) {
	mUrlString = urlStr;
	mAppKey = appKey;
	mSsid = ssid;
	mPass = pass;
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
	connectWifi();
}

void Countly::metrics() {

	if (client.connected()){
		client.close();
	}

	String metricPostParam = "begin_session=1&app_key=" + String(mAppKey)
			+ "&device_id=" + String(deviceId) + "&metrics={\"_app_version\":\""
			+ String(appVersion) + "\",\"_device\":\"Arduino\",\"_os\":\"CC3000\"}";

	Serial.println(metricPostParam);

	String returnString;

	delay(1000);
	client = cc3000.connectTCP(ip, 80);
	if (client.connected()) {
		Serial.println("connected");
		client.println(F("POST /i HTTP/1.1"));
		client.print(F("Host: "));
		client.println(mUrlString);
		client.println(F("Connection: close\r\nContent-Type: application/x-www-form-urlencoded"));
		client.println(F("User-Agent: Arduino CC3000"));
		client.print(F("Content-Length: "));
		client.println(metricPostParam.length() + 1);
		client.println();
		for (int i = 0; i < metricPostParam.length(); i++) {
			client.print(metricPostParam.charAt(i));
		}
		client.println();
	}

	while (client.connected() && client.available()) {
		char c = client.read();
		Serial.write(c);
	}
	client.flush();
}

void Countly::event(String key, int sum) {
	if (client.connected()){
		client.close();
	}

	String eventPostParam = "end_session=1&app_key=" + String(mAppKey)
			+ "&device_id=" + String(deviceId)
			+ "&events=[{\"count\":1,\"sum\":" + sum + ",\"key\":\"" + key
			+ "\"}]";

	Serial.println(eventPostParam);

	String returnString;
	client = cc3000.connectTCP(ip, 80);
	if (client.connected()) {
		Serial.println("connected");
		client.println(F("POST /i HTTP/1.1"));
		client.print(F("Host: "));
		client.println(mUrlString);
		client.println(F("Connection: close\r\nContent-Type: application/x-www-form-urlencoded"));
		client.println(F("User-Agent: Arduino CC3000"));
		client.print(F("Content-Length: "));
		client.println(eventPostParam.length() + 1);
		client.println();
		for (int i = 0; i < eventPostParam.length(); i++) {
			client.print(eventPostParam.charAt(i));
		}
		client.println();
		lastConnectionEventsTime = millis();
	}

	while (client.connected() && client.available()) {
		char c = client.read();
		Serial.write(c);
	}
	client.flush();
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

void Countly::connectWifi() {
	/* Initialise the module */
	Serial.println(F("\nInitializing..."));
	if (!cc3000.begin()) {
		Serial.println(F("Couldn't begin()! Check your wiring?"));
		while (1)
			;
	}

	// Optional SSID scan
	// listSSIDResults();

	Serial.print(F("\nAttempting to connect to "));
	Serial.println(mSsid);
	if (!cc3000.connectToAP(mSsid, mPass, WLAN_SECURITY)) {
		Serial.println(F("Failed!"));
		while (1)
			;
	}

	Serial.println(F("Connected!"));

	/* Wait for DHCP to complete */
	Serial.println(F("Request DHCP"));
	while (!cc3000.checkDHCP()) {
		delay(100); // ToDo: Insert a DHCP timeout!
	}

	ip = 0;
	// Try looking up the website's IP address
	while (ip == 0) {
		if (!cc3000.getHostByName(mUrlString, &ip)) {
			Serial.println(F("Couldn't resolve!"));
		}
		delay(500);
	}

	cc3000.printIPdotsRev(ip);
}
