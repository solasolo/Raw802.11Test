#include "Raw80211.h"

const int Raw80211::DST_MAC_OFFSET = 4;
const int Raw80211::SRC_MAC_OFFSET = 10;
const int Raw80211::BSS_MAC_OFFSET = 16;
const int Raw80211::SEQ_NUM_OFFSET = 22;
const int Raw80211::LENGTH_OFFSET = 24;
const int Raw80211::HEAD_LENGTH = 26;

Raw80211* Raw80211::Instance = NULL;

// Header template for sending our own packets
uint8_t DefaultHeader[] = {
    0x40, 0x0C,                         //  0- 1: Frame Control  //Version 0 && Data Frame && MESH
    0x00, 0x00,                         //  2- 3: Duration (will be overwritten)
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, //  4- 9: Destination address (broadcast)
    0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, // 10-15: Source address, set in "init"
    0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, // 16-21: BSSID, set in "init"
    0x00, 0x00                          // 22-23: Sequence / fragment number
};

/**
 * wifi_sniffer_packet_handler(uint8_t *buff, uint16_t pkt_type_len)
 * Packet handler called by firmware to handle received packet.
 * ESP8266 and ESP32 use slightly different interfaces. It seems ESP8266
 * does not actully provide the length as the last parameter but only an
 * indication for the packet type.
 */
#ifdef ESP8266
void wifi_sniffer_packet_handler(uint8_t *buff, uint16_t pkt_type_len)
{
#else
void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type)
{
#endif

  const wifi_promiscuous_pkt_t *pt = (wifi_promiscuous_pkt_t *)buff; // Dont know what these 3 lines do
  const wifi_ieee80211_packet_t *pk = (wifi_ieee80211_packet_t *)pt->payload;
  const wifi_ieee80211_mac_hdr_t *hdr = &pk->hdr;
  const uint8_t *data = pt->payload + Raw80211::HEAD_LENGTH;

  // Extract payload
  unsigned char *d = (unsigned char *)pt->payload + Raw80211::LENGTH_OFFSET;
  short len = ((unsigned short)d[0]) << 8 | d[1];

  //RawChannel::Debug("> ");
  //Raw80211::HeaderDebug((const uint8_t*)hdr);
  //RawChannel::PrintMAC(hdr->addr2);
  //RawChannel::Debug("\r\n");
  
  // Raw80211::HeaderDebug(data);
  // RawChannel::Debug(" (RSSI: %d, Bandwidth; %d, Mode: %d, Rate: %d, Length: %d)\n", pt->rx_ctrl.rssi, pt->rx_ctrl.cwb, pt->rx_ctrl.sig_mode, pt->rx_ctrl.rate, len);

  Raw80211* channel = Raw80211::Instance;  
  if(channel && channel->ReceiveCallback && memcmp(channel->getThisMac(), hdr->addr3, 6) == 0)
  {
    channel->ReceiveCallback(hdr->addr2, data, len);
  }
}

Raw80211::Raw80211(uint8_t channel)
    : RawChannel(channel)
{
  Instance = this;
  memcpy(this->_raw_header, DefaultHeader, Raw80211::HEAD_LENGTH);
}

void Raw80211::setPeer(const uint8_t mac[6])
{
  memcpy(this->PeerMac, mac, 6);
  memcpy(this->_raw_header + BSS_MAC_OFFSET, this->PeerMac, 6);
}

void Raw80211::Send(const uint8_t *data, uint16_t len)
{
  static uint16_t sequence = 0;
  uint8_t buf[len + Raw80211::HEAD_LENGTH];

  if (len > 1500)
    return;

  memcpy(buf, this->_raw_header, Raw80211::HEAD_LENGTH); // Copy raw header
  memcpy(buf + Raw80211::HEAD_LENGTH, data, len);    // Copy payload data
  buf[Raw80211::LENGTH_OFFSET] = (len >> 8) & 0xff;        // Copy length
  buf[Raw80211::LENGTH_OFFSET + 1] = len & 0xff;

  RawChannel::Debug("< ");
  Raw80211::HeaderDebug((const wifi_ieee80211_mac_hdr_t*)buf);

  memcpy(buf + SEQ_NUM_OFFSET, (char *)&sequence, 2);
#ifdef ESP32
  CHECK(esp_wifi_80211_tx(WIFI_IF_STA, buf, Raw80211::HEAD_LENGTH + len, true), "Wifi 80211 Send");
#else
  wifi_send_pkt_freedom(buf, Raw80211::HEAD_LENGTH + len, true);
#endif
}

/**
 * Raw80211::start()
 * Sets up hardware to begin receiving raw data frames to our own mac address
 * and broadcasts. Make sure to call init() first!
 */
void Raw80211::Start()
{
  // Prepare raw header for sending
  memcpy(this->_raw_header + SRC_MAC_OFFSET, this->ThisMac, 6);

#ifdef ESP8266
  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(0);
  WiFi.disconnect();

  wifi_set_promiscuous_rx_cb(wifi_sniffer_packet_handler);
  wifi_promiscuous_enable(1);
  wifi_promiscuous_set_mac(mac);
  wifi_set_channel(_channel);
#else
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  
  CHECK(esp_wifi_init(&cfg), "Wifi Init");
  CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM), "Wifi Set Storage");
  //CHECK(esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT40), "Wifi Set Bandwidth");
  CHECK(esp_wifi_config_80211_tx_rate(WIFI_IF_STA, WIFI_PHY_RATE_54M), "Wifi Config TX Rate");
  CHECK(esp_wifi_start(), "Wifi Start");
  CHECK(esp_wifi_set_mode(WIFI_MODE_STA), "Wifi Set Mode");
  CHECK(esp_wifi_disconnect(), "Wifi Disconnect");
  CHECK(esp_wifi_set_promiscuous(1), "Wifi Set Promiscuous");
  CHECK(esp_wifi_set_promiscuous_rx_cb(wifi_sniffer_packet_handler), "Wifi Set Promiscuous Callback");
  wifi_promiscuous_filter_t filter = {WIFI_PROMIS_FILTER_MASK_ALL};
  CHECK(esp_wifi_set_promiscuous_filter(&filter), "Wifi Set Promiscuous Filter"); 
  CHECK(esp_wifi_set_channel(this->Channel, WIFI_SECOND_CHAN_NONE), "Wifi Set Channel");
  CHECK(esp_wifi_set_max_tx_power(84), "Wifi Set Power");
#endif
}

void Raw80211::HeaderDebug(const wifi_ieee80211_mac_hdr_t *header)
{
#ifdef DEBUG_PRINT  
  RawChannel::PrintMAC(header->addr1);
  RawChannel::Debug(" / ");
  RawChannel::PrintMAC(header->addr2);
  RawChannel::Debug(" / ");
  RawChannel::PrintMAC(header->addr3);
  
  const uint8_t* pLen = (const uint8_t*)header + Raw80211::LENGTH_OFFSET;
  int len = pLen[0] * 256 + pLen[1];
  //RawChannel::Debug(" (Length: %d)\n", len);
  Serial.printf(" (Length: %d)\n", len);
#endif
}
