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
  esp_wifi_set_channel(this->Channel, WIFI_SECOND_CHAN_NONE);
  WiFi.disconnect();

  if (esp_now_init() == ESP_OK)
  {
    RawChannel::Debug("ESPNow Init Success");
  }
  else
  {
    RawChannel::Debug("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }

  esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_54M);

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
}

void RawNow::Send(const uint8_t *data, uint16_t data_len)
{
    RawChannel::Debug("Sending to: ");
    RawChannel::PrintMAC(this->PeerMac);
    RawChannel::Debug("\n");

    esp_err_t result = esp_now_send(this->PeerMac, data, data_len);

    RawChannel::Debug("Send Status: ");
    if (result == ESP_OK)
    {
      RawChannel::Debug("Success\n");
    }
    else
    {
      this->CheckNowError(result);
    }
}

void RawNow::setPeer(const uint8_t mac[6])
{
  memcpy(this->PeerMac, mac, 6);

  this->AddPeer(mac);
}

void RawNow::AddPeer(const uint8_t mac[6])
{
  bool exists = false; // esp_now_is_peer_exist(addr);

  esp_now_peer_info_t Slave;
  memset(&Slave, 0, sizeof(Slave));

  memcpy(Slave.peer_addr, mac, 6);
  Slave.channel = 0;
  Slave.encrypt = 0;

  Serial.printf("Set Peer at Channel: %d\r\n", this->Channel);
  
  if (exists)
  {
    // Slave already paired.
    Serial.println("Already Paired");
  }
  else
  {
    // Slave not paired, attempt pair
    esp_err_t addStatus = esp_now_add_peer(&Slave);
    if (addStatus == ESP_OK)
    {
      // Pair success
      Serial.println("Pair success");
    }
    else
    {
      this->CheckNowError(addStatus);
    }
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

void RawNow::CheckNowError(esp_err_t res)
{
  switch (res)
  {
  case ESP_ERR_ESPNOW_NOT_INIT:
    RawChannel::Debug("ESPNOW Not Init\n");
    break;

  case ESP_ERR_ESPNOW_ARG:
    RawChannel::Debug("Invalid Argument\n");
    break;

  case ESP_ERR_ESPNOW_FULL:
    RawChannel::Debug("Peer list full\n");
    break;

  case ESP_ERR_ESPNOW_NO_MEM:
    RawChannel::Debug("Out of memory\n");
    break;

  case ESP_ERR_ESPNOW_EXIST:
    RawChannel::Debug("Peer Exists\n");
    break;

  default : RawChannel::Debug("Not sure what happened\n");
  }
}
