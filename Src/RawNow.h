#ifndef _RAW_NOW_H
#define _RAW_NOW_H

#include "RawChannel.h"
#include <esp_now.h>

class RawNow : public RawChannel
{
public:    
   static RawNow* Instance;
    
public:
    RawNow(uint8_t channel = 1);

    void Start();
    void Send(const uint8_t *data, uint16_t data_len);
    
    void setPeer(const uint8_t mac[6]);

    void Scan();

private:
    void AddPeer(const uint8_t mac[6]);    
};

#endif
