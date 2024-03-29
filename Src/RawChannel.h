#ifndef _RAW_CHANNWL_H_
#define _RAW_CHANNWL_H_

#define DEBUG_PRINT

#ifdef ESP32
#include <WiFi.h>
#include <esp_wifi.h>
#else
#include <ESP8266WiFi.h>
#endif

// IEEE802.11 data structures ---------------------
#ifndef ESP32
typedef enum
{
    WIFI_PKT_MGMT,
    WIFI_PKT_CTRL,
    WIFI_PKT_DATA,
    WIFI_PKT_MISC,
} wifi_promiscuous_pkt_type_t;
#endif

typedef enum
{
    ASSOCIATION_REQ,
    ASSOCIATION_RES,
    REASSOCIATION_REQ,
    REASSOCIATION_RES,
    PROBE_REQ,
    PROBE_RES,
    NU1, /* ......................*/
    NU2, /* 0110, 0111 not used */
    BEACON,
    ATIM,
    DISASSOCIATION,
    AUTHENTICATION,
    DEAUTHENTICATION,
    ACTION,
    ACTION_NACK,
} wifi_mgmt_subtypes_t;

typedef struct
{
    unsigned interval : 16;
    unsigned capability : 16;
    unsigned tag_number : 8;
    unsigned tag_length : 8;
    char ssid[0];
    uint8_t rates[1];
} wifi_mgmt_beacon_t;

typedef struct
{
    unsigned protocol : 2;
    unsigned type : 2;
    unsigned subtype : 4;
    unsigned to_ds : 1;
    unsigned from_ds : 1;
    unsigned more_frag : 1;
    unsigned retry : 1;
    unsigned pwr_mgmt : 1;
    unsigned more_data : 1;
    unsigned wep : 1;
    unsigned strict : 1;
} wifi_header_frame_control_t;

/**
 * Ref: https://github.com/lpodkalicki/blog/blob/master/esp32/016_wifi_sniffer/main/main.c
 */
typedef struct
{
    wifi_header_frame_control_t frame_ctrl;
    uint8_t addr1[6]; /* receiver address */
    uint8_t addr2[6]; /* sender address */
    uint8_t addr3[6]; /* filtering address */
    unsigned sequence_ctrl : 16;
    uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct
{
    wifi_ieee80211_mac_hdr_t hdr;
    uint8_t payload[2]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

struct WifiMacData
{
};

#define CHECK(fn, where) ({                                                                        \
    esp_err_t err_rc_ = (fn);                                                                      \
    if (err_rc_ != ESP_OK) {                                                                       \
        RawChannel::Debug("Error @ %s\n", where);                                                  \
        _esp_error_check_failed_without_abort(err_rc_, __FILE__, __LINE__, __ASSERT_FUNC, #fn);    \      
    }                                                                                              \                                                                            
})

class RawChannel
{
protected:
    typedef void (*CBReceive)(const uint8_t src_mac[6], const uint8_t *buff, uint16_t buff_len);

public:
    static void Debug(const char* format, ...);
    static void PrintMAC(const uint8_t *mac);

public:
    RawChannel(uint8_t channel = 1);
    
    void setCallback(CBReceive cb);
    const uint8_t* getThisMac();

    CBReceive ReceiveCallback;

protected:
    uint8_t Channel;
    uint8_t PeerMac[6];
    uint8_t ThisMac[6];

    // bool Check(esp_err_t res, const char* where);
    void getMac(uint8_t *mac);
};

#endif
