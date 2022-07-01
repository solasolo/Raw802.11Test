#include "raw80211.h"

// #define SENDER 

unsigned int Count = 0;
char SendBuff[1500];

void setup() {
  Serial.begin(115200);

  Raw80211::init("123456", 3);
  Raw80211::register_cb(cb);
  
  Raw80211::start();  

  Serial.println("Started");
}

void loop() {
#ifdef SENDER  
  unsigned long t = micros();
  
  memcpy(SendBuff, (char*)&t, sizeof(t));
  sprintf(SendBuff + 8, "Test:%06u", Count++); 

  Raw80211::send((const uint8_t*)SendBuff, 1000);  
#endif

  delay(1000);
}

void cb(const wifi_ieee80211_mac_hdr_t *hdr, signed int rssi, const uint8_t *buff, uint16_t buff_len)
{
#ifdef SENDER 
  unsigned long t = micros();
  unsigned long rt = *((unsigned long*)buff);
 
  Serial.printf("RECV:[%12.6f - %12.6f](%7.3f) %s\n", t / 1e6, rt / 1e6, (t - rt) / 1e3, buff + 8); 
#else
  Raw80211::send((const uint8_t*)buff, buff_len);  
#endif

}
