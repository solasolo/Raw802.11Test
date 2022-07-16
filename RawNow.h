#ifndef _RAW_NOW_H
#define _RAW_NOW_H

#include "RawChannel.h"
#include "esp_now.h"

class RawNow : public RawChannel
{
public:
    RawNow(uint8_t channel = 1);

    void Start();
    void Send(const uint8_t *data, uint16_t data_len);
    void setCallback(RAW_CB cb);

    void Scan();

private:
    esp_now_peer_info_t Slave;
};

#endif
