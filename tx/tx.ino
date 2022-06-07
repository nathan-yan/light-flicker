
#include <RFM69OOK.h>
#include <SPI.h>
#include <RFM69OOKregisters.h>

RFM69OOK radio(10, 3, 1, 0);
unsigned long cnt;

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(2, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(2, HIGH);
  delay(100);
  digitalWrite(2, LOW);

  Serial.println("serial started");

  radio.initialize();
  Serial.println("initialized!");
  radio.transmitBegin();

  pinMode(3, OUTPUT);

  Serial.println("beginning transmission mode!");
  //radio.setFrequencyMHz(868.88);
  radio.setFrequencyMHz(433.9);
  radio.setPowerLevel(5);
}

void delayMicros(uint32_t d) {
  uint32_t t = micros() + d;
  while(micros() < t);
}

long scale = 100;

void flip(bool pol) {
  if (!pol) flip0();
  else flip1();
}

void flip0() {
  radio.send(1);
  delayMicros(1000);
  radio.send(0);
  delayMicros(1000);
}

void flip1() {
  radio.send(0);
  delayMicros(1000);
  radio.send(1);
  delayMicros(1000);
}

void startPacket() {
  flip1();
  delay(720);   // send eight 1s
}

bool sendByte(byte b, bool pol) {
  bool bit = b & 0x80;  // 1000 0000
  bool prev_bit = pol;
  for (int i = 0; i < 8; i++) {
    if (prev_bit == bit) {
      // delay for another 100ms
      delay(100);
    } else {
      pol = 1 - pol;
      flip(pol);
      delay(20);
    }

    prev_bit = bit;
    b <<= 1;
    bit = b & 0x80;
  }

  return pol;
}

void endPacket(bool pol) {
  flip0();
  if (pol == 1) {
    delay(820);
  } else {
    delay(800);
  }
}

void loop() {
  startPacket();
  bool pol = sendByte(0xe3, 1);
  pol = sendByte('h', pol);
  pol = sendByte('e', pol);
  pol = sendByte('l', pol);
  pol = sendByte('l', pol);
  pol = sendByte('o', pol);
  pol = sendByte('!', pol);
  endPacket(pol);
  delay(1000);
}
