#include "src/Raw80211.h"
#include "src/RawNow.h"

//#define SENDER
#define DEBUG_PRINT

unsigned int Count = 0;
char SendBuff[1500];

uint8_t PeerAddress[] = {0x60, 0x55, 0xf9, 0x71, 0x8c, 0xc8};

Raw80211 Channel(3);
//RawNow Channel(3);

void setup()
{
    Serial.begin(115200);

    RawChannel::PrintMAC(Channel.getThisMac());
    Serial.println();

    Channel.setCallback(cb);
    Channel.Start();

#ifdef SENDER
    Channel.setPeer(PeerAddress);
#endif

    Serial.println("Started");
}

void loop()
{
#ifdef SENDER
    unsigned long t = micros();

    memcpy(SendBuff, (char *)&t, sizeof(t));
    sprintf(SendBuff + 8, "Test:%06u", Count++);

    Channel.Send((const uint8_t *)SendBuff, 100);
#endif

    // Channel.Scan();

    delay(1000);
}

void cb(const uint8_t mac[6], const uint8_t *buff, uint16_t buff_len)
{
#ifdef SENDER
    unsigned long t = micros();
    unsigned long rt = *((unsigned long *)buff);

    Serial.printf("RECV:[%12.6f - %12.6f](%7.3f) %s\n", t / 1e6, rt / 1e6, (t - rt) / 1e3, buff + 8);
#else
    Channel.setPeer(mac);
    Channel.Send((const uint8_t *)buff, buff_len);
#endif
}
