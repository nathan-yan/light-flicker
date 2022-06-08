//
// Simple OOK playground -- receives and dumps received signal and timing
//

#include <RFM69OOK.h>
#include <SPI.h>
#include <RFM69OOKregisters.h>

#include <Servo.h>
#include <packetizer.h>

#define TSIZE 400
#define MAX_0_DUR 100000 // 100 ms
#define MIN_1_DUR 50 // 100 us
#define TOL 50 // +- tolerance

RFM69OOK radio(8, 3, 1, 0);
Packetizer pk(radio);
Servo servo;

void setup() {
  Serial.begin(115200);

  delay(2000);

  pinMode(4, OUTPUT); 
  digitalWrite(4, HIGH);
  delay(100);
  digitalWrite(4, LOW);

  pinMode(13, OUTPUT);

  radio.initialize();
  radio.setBandwidth(OOK_BW_83_3);
  // radio.setRSSIThreshold(-70);
  radio.setFixedThreshold(40);
  radio.setFrequencyMHz(433.9);
  radio.receiveBegin();
  radio.setPowerLevel(20);

  Serial.println(F("start"));

  Serial.println(HOLD_TIME);
  Serial.println(OFFSET_TIME);

  pinMode(11, OUTPUT);
}

char packet_buffer[20];

void loop() {
  // size - 1 for buflen for string terminator
  int res = pk.listen(radio.poll(), packet_buffer, 19);

  if (res > 0) {
    packet_buffer[res + 1] = '\0';
    Serial.print("received packet: "); Serial.println(packet_buffer);
  }

  // delayMicroseconds(20);
}
