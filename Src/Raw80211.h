#ifndef _RAW80211_H
#define _RAW80211_H

#include "RawChannel.h"

void setup_raw();
void get_mac(uint8_t mac[6]);
void mac2str(const uint8_t *ptr, char *string);
void dumphex(const uint8_t *data, uint16_t len, const char *prefix = "");

class Raw80211 : public RawChannel
{
public:
    static const int DST_MAC_OFFSET;
    static const int SRC_MAC_OFFSET;
    static const int BSS_MAC_OFFSET;
    static const int SEQ_NUM_OFFSET;
    static const int LENGTH_OFFSET;
    static const int HEAD_LENGTH;

public:
    static Raw80211* Instance;
    static void HeaderDebug(const wifi_ieee80211_mac_hdr_t *header);

public:
    Raw80211(uint8_t channel = 1);

    void Start();
    void Send(const uint8_t *data, uint16_t data_len);
    
    void setPeer(const uint8_t mac[6]);

private:
    uint8_t _raw_header[24];
};

#endif
