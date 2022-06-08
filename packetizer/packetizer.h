#ifndef PACKETIZER_H
#define PACKETIZER_H
#include <RFM69OOK.h>
#include <Arduino.h>

#define HOLD_TIME 2000    // us
#define OFFSET_TIME 1000  // us

#define AVG_N 10
#define HIGH_COUNTS 7
#define LOW_COUNTS 3

class Packetizer {
  public:
    RFM69OOK &radio;

    bool is_high = false;
    bool current_bit = 0;
    bool in_packet = false;
    int packet_bit_count = 0;
    int byte_count = 0;
    uint64_t bit_stream = 0;

    int lastn[AVG_N];
    uint64_t c = 0;
    int counts = 0;
    uint64_t t0 = 0;

    Packetizer(RFM69OOK &radio);
    
    void flip(bool pol);
    void sendPacket(char* buf, int buflen);

    int listen(bool s, char* buf, int buflen);

  private:
    void flip0();
    void flip1();
    void startPacket();
    bool sendByte(byte b, bool pol);
    void endPacket(bool pol);

    void acc(bool bit);
};

#endif