#include "RawChannel.h"

RawChannel::RawChannel(uint8_t channel)
{
    this->Channel = channel;
}

void RawChannel::geMac(uint8_t *mac)
{
#ifdef ESP32
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
#else
    wifi_get_macaddr(STATION_IF, mac);
#endif
}

void RawChannel::setPeer(uint8_t addr[6])
{
    memcpy(this->PeerAddress, addr, 6);
}

void RawChannel::PrintMAC(const uint8_t *mac)
{
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}