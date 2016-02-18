#include "Countly.h"
#include "HardwareSerial.h"
#include <HttpClient.h>
#include "WString.h"
#include <Process.h>
#include <SPI.h>
#include <Ethernet.h>

String mUrlString, mAppKey;
const char *deviceId;
const int BUFSIZE = 32;
char buf[BUFSIZE];
EepromUtil eepromUtil;
const String appVersion = "0.0.1";
EthernetClient client;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
long randomDigit;
int isEthernetConnected=0;
byte SNTP_server_IP[]    = { 192, 43, 244, 18}; // time.nist.gov

Countly::Countly(String urlStr, String appKey) {
	mUrlString = urlStr;
	mAppKey = appKey;
	deviceId = getUuidFromEeprom();
	if (deviceId == "") {
		eepromUtil.eeprom_erase_all();
		String uuid = generateUuid();
		writeUuidToEeprom(uuid, BUFSIZE);
		deviceId = uuid.c_str();
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

	String metricPostParam = "begin_session=1&app_key=" + mAppKey + "&device_id="
			+ String(deviceId)
			+ "&metrics={\"_locale\":\"en\",\"_app_version\":\"" + appVersion
			+ "\",\"_device\":\"Arduino\",\"_os\":\"Ethernet\"}";

	Serial.println(metricPostParam);

	String returnString;

	delay(1000);

	if (client.connect(mUrlString.c_str(), 80)) {
		Serial.println("connected");
		client.println("POST " + metricApiUrl + " HTTP/1.1");
		client.println("Host: " + mUrlString);
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.println("Connection: close");
		client.println("User-Agent: Arduino Ethernet/Countly");
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

	String eventPostParam = "end_session=1&app_key=" + mAppKey + "&device_id="
			+ String(deviceId) + "&events=[{\"count\":1,\"sum\":" + sum + ",\"key\":\"" + key + "\"}]";

	Serial.println(eventPostParam);

	String returnString;

	if (client.connect(mUrlString.c_str(), 80)) {
		Serial.println("connected");
		client.println("POST " + eventApiUrl + " HTTP/1.1");
		client.println("Host: " + mUrlString);
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.println("Connection: close");
		client.println("User-Agent: Arduino Ethernet/Countly");
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

String Countly::generateUuid() {
	randomSeed(analogRead(0));
	int i;
	String uuidString;
	for (i = 0; i < BUFSIZE; i++) {
		randomDigit = random(BUFSIZE);
		uuidString += "0123456789ABCDEFHIJKLMNOPRSTUVYZ"[randomDigit];
	}

	return uuidString;
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
