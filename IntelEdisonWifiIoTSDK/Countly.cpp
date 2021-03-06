#include "Countly.h"

const char* mUrlString;
const char* mAppKey;
const char* deviceId;
char* mSsid;
char* mPass;
const int BUFSIZE = 32;
char buf[BUFSIZE];
EepromUtil eepromUtil;
const String appVersion = "0.0.1";
WiFiClient client;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
long randomDigit;
int status = WL_IDLE_STATUS;
char randomCharset[33]="0123456789ABCDEFHIJKLMNOPRSTUVYZ";

Countly::Countly(const char* urlStr, const char* appKey, char* ssid, char* pass) {
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

	String metricApiUrl = "/i";

	String metricPostParam = "begin_session=1&app_key=" + String(mAppKey)
			+ "&device_id=" + String(deviceId)
			+ "&metrics={\"_app_version\":\"" + appVersion
			+ "\",\"_device\":\"IntelEdison\",\"_os\":\"WiFi\"}";

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
		client.println("User-Agent: Intel Edison WiFi/Countly");
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
		client.println("User-Agent: Intel Edison WiFi/Countly");
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
