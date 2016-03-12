#include "Countly.h"

const char* mUrlString;
const char* mAppKey;
const char* deviceId;
const int BUFSIZE = 32;
char buf[BUFSIZE];
EepromUtil eepromUtil;
const String appVersion = "0.0.1";
WiFiClient client;
char* mSsid;
char* mPass;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
long randomDigit;
int status = WL_IDLE_STATUS;
unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 10L * 1000L;
unsigned long lastConnectionEventsTime=0;
char randomCharset[33]="0123456789ABCDEFHIJKLMNOPRSTUVYZ";

Countly::Countly(const char* urlStr, const char* appKey, char* ssid,
		char* pass) {
	mUrlString = urlStr;
	mAppKey = appKey;
	mSsid = ssid;
	mPass = pass;
}

void Countly::init(){
	deviceId = getUuidFromEeprom();
	bool isDeviceIdValid=false;
	for(int i=0; i<sizeof(randomCharset) - 1; i++){
		if(deviceId[0]==randomCharset[i]){
			isDeviceIdValid=true;
		}
	}
	if (isDeviceIdValid==false) {
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
	client.stop();

	String metricPostParam = "begin_session=1&app_key=" + String(mAppKey)
			+ "&device_id="+String(deviceId)+"&metrics={\"_app_version\":\""
			+ appVersion + "\",\"_device\":\"NodeMCU\",\"_os\":\"Wifi\"}";

	Serial.println(metricPostParam);

	String returnString;

	delay(1000);
	if (client.connect(mUrlString, 80)) {
		Serial.println("connected");
		client.println(F("POST /i HTTP/1.1"));
		client.print(F("Host: "));
		client.println(mUrlString);
		client.println(F("Connection: close\r\nContent-Type: application/x-www-form-urlencoded"));
		client.println(F("User-Agent: Arduino WiFi"));
		client.print(F("Content-Length: "));
		client.println(metricPostParam.length()+1);
		client.println();
		  for(int i=0;i<metricPostParam.length();i++){
			  client.print(metricPostParam.charAt(i));
		  }
		client.println();
	}

	while (client.available()) {
		String line = client.readStringUntil('\r');
		Serial.print(line);
	}
}

void Countly::event(String key, int sum) {
	metrics();

	String eventPostParam = "end_session=1&app_key=" + String(mAppKey)
			+ "&device_id="+String(deviceId)+"&events=[{\"count\":1,\"sum\":" + sum + ",\"key\":\"" + key + "\"}]";

	Serial.println(eventPostParam);

	String returnString;
	client.stop();

	if (client.connect(mUrlString, 80)) {
		Serial.println("connected");
		client.println(F("POST /i HTTP/1.1"));
		client.print(F("Host: "));
		client.println(mUrlString);
		client.println(F("Connection: close\r\nContent-Type: application/x-www-form-urlencoded"));
		client.println(F("User-Agent: Arduino WiFi"));
		client.print(F("Content-Length: "));
		client.println(eventPostParam.length()+1);
		client.println();
		for(int i=0;i<eventPostParam.length();i++){
		   client.print(eventPostParam.charAt(i));
		}
		client.println();
		lastConnectionEventsTime = millis();
	}

	while (client.available()) {
		String line = client.readStringUntil('\r');
		Serial.print(line);
	}
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

void Countly::printWifiStatus() {
	// print the SSID of the network you're attached to:
	Serial.print("SSID: ");
	Serial.println(WiFi.SSID());

	// print your WiFi shield's IP address:
	IPAddress ip = WiFi.localIP();
	Serial.print("IP Address: ");
	Serial.println(ip);

	// print the received signal strength:
	long rssi = WiFi.RSSI();
	Serial.print("signal strength (RSSI):");
	Serial.print(rssi);
	Serial.println(" dBm");
}

void Countly::connectWifi() {
	// check for the presence of the shield:
	if (WiFi.status() == WL_NO_SHIELD) {
		Serial.println("WiFi shield not present");
		// don't continue:
		while (true)
			;
	}

	// attempt to connect to Wifi network:
	while (status != WL_CONNECTED) {
		Serial.print("Attempting to connect to SSID: ");
		Serial.println(mSsid);
		// Connect to WPA/WPA2 network. Change this line if using open or WEP network:
		status = WiFi.begin(mSsid, mPass);

		// wait 10 seconds for connection:
		delay(10000);
	}
	printWifiStatus();
	Serial.println("Connected to wifi");
}
