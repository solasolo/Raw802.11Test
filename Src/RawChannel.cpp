#include "RawChannel.h"

RawChannel::RawChannel(uint8_t channel)
{
    this->ReceiveCallback = NULL;

    this->Channel = channel;
    this->getMac(this->ThisMac);
}

void RawChannel::getMac(uint8_t *mac)
{
#ifdef ESP32
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
#else
    wifi_get_macaddr(STATION_IF, mac);
#endif
}

void RawChannel::setCallback(RawChannel::CBReceive cb)
{
  RawChannel::ReceiveCallback = cb;
}

const uint8_t *RawChannel::getThisMac()
{
    return this->ThisMac;
}

void RawChannel::Debug(const char* format, ...)
{
#ifdef DEBUG_PRINT

    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);

    int len = Serial.printf(format, copy);

    va_end(copy);
#endif
}

void RawChannel::PrintMAC(const uint8_t *mac)
{
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/**
 * dumphex(data, len, [prefix])
 * Writes data out to Serial using familiar layout with 16 bytes on every
 * line followed by the ASCII representation.
 * Provide an optional prefix to indent all the lines.
 */
void dumphex(const uint8_t *data, uint16_t len, const char *prefix)
{
  for (uint16_t i = 0; i < len; i++)
  {
    // Add prefix if first in line
    if (i % 16 == 0) // First in line
      Serial.print(prefix);

    Serial.printf("%02x ", data[i]);

    // Show in ascii at end of line, complicated as it has to handle
    // incomplete lines
    if (i % 16 == 15 || i == len - 1)
    {
      // Fill line if it is not a full one
      for (uint16_t j = 16 - i % 16; j > 0; j--)
        Serial.print("   ");

      // Output ascii
      char filler = '.';
      for (uint16_t j = i - i % 16; j <= i && j < len; j++)
        Serial.print(data[j] >= 32 && data[j] < 127 ? (char)data[j] : filler);
      Serial.println();
    }
  }
}

/**
 * mac2str(*mac, *buf)
 * Converts a mac-address into a printable string.
 */
void mac2str(const uint8_t *mac, char *string)
{
  sprintf(string, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
