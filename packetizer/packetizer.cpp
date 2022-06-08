#include <packetizer.h>

Packetizer::Packetizer(RFM69OOK &radio) {
  this->radio = radio;
}

void Packetizer::flip(bool pol) {
  if (!pol) flip0();
  else flip1();
}

void Packetizer::flip0() {
  radio.send(0);
}

void Packetizer::flip1() {
  radio.send(1);
}

void Packetizer::startPacket() {
  flip(1);
  delayMicroseconds(8 * HOLD_TIME + OFFSET_TIME); // send byte of 1s
}

bool Packetizer::sendByte(byte b, bool pol) {
  bool bit = b & 0x80;  // 1000 0000
  bool prev_bit = pol;
  for (int i = 0; i < 8; i++) {
    if (prev_bit == bit) {
      // delay for another 100ms
      delayMicroseconds(HOLD_TIME);
    } else {
      pol = 1 - pol;
      flip(pol);
      delayMicroseconds(HOLD_TIME + OFFSET_TIME);
    }

    prev_bit = bit;
    b <<= 1;
    bit = b & 0x80;
  }

  return pol;
}

void Packetizer::endPacket(bool pol) {
  flip(0);
  delayMicroseconds(8 * HOLD_TIME + OFFSET_TIME);
}

void Packetizer::sendPacket(char* buf, int buflen) {
  startPacket();

  bool pol = 1;
  for (int i = 0; i < buflen; i++) {
    pol = sendByte(buf[i], pol);
  }

  endPacket(pol);
}

void Packetizer::acc(bool bit) {
  if (in_packet) {
    packet_bit_count++;
  }

  bit_stream <<= 1;
  bit_stream |= bit;
}

int Packetizer::listen(bool s, char* buf, int buflen) {
  int loc = c % AVG_N;

  counts += s;

  if (c >= AVG_N) {
    counts -= lastn[loc];
  }

  lastn[loc] = s;
  c++;

  uint32_t t = micros();
  if (t - t0 >= HOLD_TIME) {
    acc(current_bit);

    Serial.print(current_bit);

    t0 = t0 + HOLD_TIME;
  } else {
    if (counts > HIGH_COUNTS) {
      if (!is_high) {
        current_bit = !current_bit;
        is_high = true;
        t0 = t;
      }
    } else if (counts < LOW_COUNTS) {
      if (is_high) {
        current_bit = !current_bit;
        is_high = false;
        t0 = t;
      }
    }
  }

  if ((bit_stream & 0xff) == 0xff && !in_packet) {
    Serial.println("detected packet start");

    bit_stream = 0;
    in_packet = true;
    packet_bit_count = 0;
    byte_count = 0;
  }

  if (in_packet) {
    if (packet_bit_count == 8) {
      Serial.println();
      packet_bit_count = 0;

      if (bit_stream == 0x00) {
        in_packet = false;
        bit_stream = 0;

        Serial.println("end packet");

        return byte_count;
      } else {
        if (byte_count >= buflen) {
          in_packet = false;
          return -1;
        }

        buf[byte_count] = bit_stream;
      }

      byte_count++;
      bit_stream = 0;
    }
  }

  return 0;
}