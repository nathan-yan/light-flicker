
#include <RFM69OOK.h>
#include <SPI.h>
#include <RFM69OOKregisters.h>

#include <packetizer.h>

RFM69OOK radio(10, 3, 1, 0);
Packetizer pk(radio);

unsigned long cnt;

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(2, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(2, HIGH);
  delay(100);
  digitalWrite(2, LOW);

  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);

  Serial.println("serial started");

  radio.initialize();
  Serial.println("initialized!");
  radio.transmitBegin();

  pinMode(3, OUTPUT);

  Serial.println("beginning transmission mode!");
  //radio.setFrequencyMHz(868.88);
  radio.setFrequencyMHz(433.9);
  radio.setPowerLevel(20);

  Serial.println(HOLD_TIME);
  Serial.println(OFFSET_TIME);
}

void loop() {
  if (digitalRead(7) == LOW) {
    Serial.println("sending hello");

    char data[6] = {'h', 'e', 'l', 'l', 'o', ' '};
    pk.sendPacket(data, 6);

    delay(1000);
  } else if (digitalRead(8) == LOW) {
    Serial.println("sending world");
    
    char data[6] = {'w', 'o', 'r', 'l', 'd', '!'};
    pk.sendPacket(data, 6);

    delay(1000);
  }
}
