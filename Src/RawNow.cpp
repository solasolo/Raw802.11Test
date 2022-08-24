#include "RawNow.h"

#include <Arduino.h>

RawNow* RawNow::Instance = NULL;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  RawChannel::Debug("Packet Sent to: ");
  RawChannel::PrintMAC(mac_addr);
  RawChannel::Debug(" Send Status: ");
  RawChannel::Debug(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  RawChannel::Debug("\n");
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
  RawChannel::Debug("Packet Recv from: ");
  RawChannel::PrintMAC(mac_addr);
  RawChannel::Debug("\n");

  RawNow* channel = RawNow::Instance;  
  if(channel && channel->ReceiveCallback)
  {
     channel->ReceiveCallback(mac_addr, data, data_len);
  }
}

RawNow::RawNow(uint8_t channel)
    : RawChannel(channel)
{     
  Instance = this;
}

void RawNow::Start()
{
  WiFi.mode(WIFI_STA);
  CHECK(esp_wifi_set_channel(this->Channel, WIFI_SECOND_CHAN_NONE), "Wifi Set Channel");
  WiFi.disconnect();

  CHECK(esp_now_init(), "ESPNow Init");

  CHECK(esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_54M), "ESPNow Config Rate");

  CHECK(esp_now_register_send_cb(OnDataSent), "ESPNow Register Send Callback");
  CHECK(esp_now_register_recv_cb(OnDataRecv), "ESPNow Register Recv Callback");
}

void RawNow::Send(const uint8_t *data, uint16_t data_len)
{
    RawChannel::Debug("Sending to: ");
    RawChannel::PrintMAC(this->PeerMac);
    RawChannel::Debug("\n");

    CHECK(esp_now_send(this->PeerMac, data, data_len), "ESPNow Send");
}

void RawNow::setPeer(const uint8_t mac[6])
{
  memcpy(this->PeerMac, mac, 6);

  this->AddPeer(mac);
}

void RawNow::AddPeer(const uint8_t mac[6])
{
  bool exists = esp_now_is_peer_exist(mac);

  esp_now_peer_info_t Slave;
  memset(&Slave, 0, sizeof(Slave));

  memcpy(Slave.peer_addr, mac, 6);
  Slave.channel = 0;
  Slave.encrypt = 0;
  
  if (exists)
  {
    // Slave already paired.
    Serial.println("Already Paired");
  }
  else
  {
    // Slave not paired, attempt pair
    CHECK(esp_now_add_peer(&Slave), "ESPNow Add Peer");
  }
}

void RawNow::Scan()
{
  int8_t scanResults = WiFi.scanNetworks();

  Serial.println("");
  if (scanResults == 0)
  {
    Serial.println("No WiFi devices in AP Mode found");
  }
  else
  {
    Serial.print("Found ");
    Serial.print(scanResults);
    Serial.println(" devices ");
    for (int i = 0; i < scanResults; ++i)
    {
      // Print SSID and RSSI for each device found
      String SSID = WiFi.SSID(i);
      int32_t RSSI = WiFi.RSSI(i);
      String BSSIDstr = WiFi.BSSIDstr(i);

#ifdef DEBUG_PRINT
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(SSID);
      Serial.print(" (");
      Serial.print(RSSI);
      Serial.print(")");
      Serial.println("");
#endif

      delay(10);
      // Check if the current device starts with `Slave`
      if (SSID.indexOf("Slave") == 0)
      {
        // SSID of interest
        Serial.println("Found a Slave.");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(SSID);
        Serial.print(" [");
        Serial.print(BSSIDstr);
        Serial.print("]");
        Serial.print(" (");
        Serial.print(RSSI);
        Serial.print(")");
        Serial.println("");

        // we are planning to have only one slave in this example;
        // Hence, break after we find one, to be a bit efficient
        break;
      }
    }
  }
}
