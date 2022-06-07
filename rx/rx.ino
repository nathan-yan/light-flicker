//
// Simple OOK playground -- receives and dumps received signal and timing
//

#include <RFM69OOK.h>
#include <SPI.h>
#include <RFM69OOKregisters.h>

#define TSIZE 400
#define MAX_0_DUR 100000 // 100 ms
#define MIN_1_DUR 50 // 100 us
#define TOL 50 // +- tolerance

#define AVG_N 30

RFM69OOK radio(8, 3, 1, 0);

uint16_t tt[TSIZE];

byte s0 = 0;
byte pos = 0;
bool restart = true;

uint32_t t0 = millis();

int lastx[AVG_N];
uint32_t c = 0;
int counts = 0;

void setup() {
  Serial.begin(115200);

  delay(2000);

  pinMode(4, OUTPUT); 
  digitalWrite(4, HIGH);
  delay(100);
  digitalWrite(4, LOW);

  pinMode(13, OUTPUT);

  radio.initialize();
  // radio.setBandwidth(OOK_BW_10_4);
  // radio.setRSSIThreshold(-70);
  radio.setFixedThreshold(10);
  // radio.setSensitivityBoost(SENSITIVITY_BOOST_HIGH);
  radio.setFrequencyMHz(433.9);
  radio.receiveBegin();
  radio.setPowerLevel(5);

  Serial.println(F("start"));
}

bool is_high = false;
int holding = false;
int current_bit = 0;
int bits = 0;

bool in_packet = false; 
int packet_bit_count = 0;

uint64_t bit_stream = 0;

void acc(bool bit) {
  if (in_packet) {
    packet_bit_count++;
  }

  bit_stream <<= 1;
  bit_stream |= bit;
}

void loop() {
  bool s = radio.poll();

  int start = c % AVG_N;

  // take average of last n counts via
  // circular buffer
  
  counts += s;
  
  if (c >= AVG_N) {
    counts -= lastx[start];
  }

  lastx[start] = s;

  c++;

  delayMicroseconds(20);

  // every 100 milliseconds check if transmitter held
  uint32_t t = millis();
  if (t - t0 > 100) {
    Serial.print(current_bit);

    acc(current_bit);

    bits++;
    t0 = t;
  }

  if (counts > 20) {
    if (!is_high) {
      // transition from low to high
      current_bit = 1 - current_bit;
      is_high = true;
      t0 = t;

      Serial.print(current_bit);Serial.print("*");
      acc(current_bit);

      bits++;
    }
  } else if (counts < 10) {
    if (is_high) {
      // transition from high to low
      current_bit = 1 - current_bit;
      is_high = false;
      t0 = t;

      Serial.print(current_bit);Serial.print("^");
      acc(current_bit);
      
      bits++;
    }
  }

  if (bits % 50 == 0) {
    Serial.println();
    bits++;
  }

  if ((bit_stream & 0xff) == 0xff) {
    Serial.println("detected packet start!");
    bit_stream = 0;
    in_packet = true;
    packet_bit_count = 0;
  }

  // we've accumulated a byte
  if (in_packet) {
    if (packet_bit_count == 8) {
      packet_bit_count = 0;

      if (bit_stream == 0x00) {
        Serial.println("end packet");
        in_packet = false;
        bit_stream = 0;
      } else {
        Serial.println();
        char p[2];
        p[0] = bit_stream;
        p[1] = '\0';
        Serial.print(p);
        Serial.println();
        bit_stream = 0;
      }
    }
  }

}
