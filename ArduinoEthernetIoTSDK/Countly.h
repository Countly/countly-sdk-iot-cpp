#include "WString.h"
#include <EepromUtil.h>
#include <stdbool.h>
#include <stdint.h>
#include <WString.h>
#include <SPI.h>
#include <Ethernet.h>

#ifndef Countly_h
#define Countly_h

class Countly
{
  public:
	Countly (String urlStr, String appKey);
	void metrics();
	void event(String key, int sum);
    String generateUuid();
    const char* getUuidFromEeprom();
    bool writeUuidToEeprom(String uuid, int len);
};

#endif

