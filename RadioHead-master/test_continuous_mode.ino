#define RFM69_CS D0
#define RFM69_IRQ D8
#define RFM69_RST D1
#define SPI_CK D5 // For info only - not used in below Sketch
#define SPI_MISO D6 // For info only - not used in below Sketch
#define SPI_MOSI D7 // For info only - not used in below Sketch
#define SWITCH_PIN D4
#define rxPin_433 D2 // the data from the RFM69 G2(Data) is routed to this pin!
#define txPin_433 D3 // Just define as unused-pin (as we are not using Tx)
#define pttPin_433 D3 // Just define as unused-pin (as we are not using Tx)

#include <RH_ASK.h> // https://github.com/PaulStoffregen/RadioHead -> I'm using release 1.89 2020/08/05
RH_ASK simple_ASK(2000, rxPin_433, txPin_433, pttPin_433);

// #include "SPI.h" // not nec as included by the below
#include <RH_RF69.h> // https://github.com/PaulStoffregen/RadioHead -> I'm using release 1.89 2020/08/05

// NB. on earlier versions of RadioHead (such as release 1.75 which works on the ATTiny) the...
// ...ESP866 crashed with "ISR not in IRAM" message. This is fixed by editing RH_RF69.ccp and...
// ...changing "void RH_RF69::isr?()" to "void ICACHE_RAM_ATTR RH_RF69::isr?()" in 3 places

RH_RF69 rf69(RFM69_CS, RFM69_IRQ);

#define RF69_FREQ 433.92300 // for my devices with OOK@2000bps, BW=3.9Hz DCC=4%

// still works for frequency varied over approx +/-4kHz (BW set to 3.9kHz) or +/-8.5kHz (BW set to 7.8kHz)

#define CONFIG_PACKET_VARIABLE(RH_RF69_PACKETCONFIG1_PACKETFORMAT_VARIABLE | RH_RF69_PACKETCONFIG1_CRCAUTOCLEAROFF)

#define CONFIG_OOK_CONT_SYNC(RH_RF69_DATAMODUL_DATAMODE_CONT_WITH_SYNC | RH_RF69_DATAMODUL_MODULATIONTYPE_OOK | RH_RF69_DATAMODUL_MODULATIONSHAPING_OOK_NONE)

#define CONFIG_OOK_CONT_NO_SYNC(RH_RF69_DATAMODUL_DATAMODE_CONT_WITHOUT_SYNC | RH_RF69_DATAMODUL_MODULATIONTYPE_OOK | RH_RF69_DATAMODUL_MODULATIONSHAPING_OOK_NONE)

#define CONFIG_RegRxBw3_9 0x46 // BW=3.9kHz DCC=4%

#define CONFIG_RegRxBw7_8 0x45 // BW=7.8kHz DCC=4%

#define CONFIG_RegRxBw83 0xF1 // BW=83khz DCC=1/8%

const RH_RF69::ModemConfig OOK_Rb2Bw3_9 = {
  CONFIG_OOK_CONT_SYNC,
  0x3E,
  0x80,
  0x00,
  0x10,
  CONFIG_RegRxBw3_9,
  CONFIG_RegRxBw3_9,
  CONFIG_PACKET_VARIABLE
}; // OOK@2000bps, BW=3.9Hz DCC=4%

const RH_RF69::ModemConfig OOK_Rb2Bw7_8 = {
  CONFIG_OOK_CONT_SYNC,
  0x3E,
  0x80,
  0x00,
  0x10,
  CONFIG_RegRxBw7_8,
  CONFIG_RegRxBw7_8,
  CONFIG_PACKET_VARIABLE
}; // OOK@2000bps, BW=7.8Hz DCC=4%

const RH_RF69::ModemConfig OOK_Rb2Bw83 = {
  CONFIG_OOK_CONT_SYNC,
  0x3E,
  0x80,
  0x00,
  0x10,
  CONFIG_RegRxBw83,
  CONFIG_RegRxBw83,
  CONFIG_PACKET_VARIABLE
}; // OOK@2000bps. BW=83khz DCC=1/8%

void setup() {

  Serial.begin(74880);
  delay(2000);
  Serial.println();

  if (simple_ASK.init()) Serial.println("simple_ASK OOK-Receiver init OK");

  else {
    Serial.println("simple_ASK OOK-Receiver init failed - hanging...");
    while (true) delay(1000);
  }

  Serial.print("simple_ASK Transmition BitRate=");
  Serial.print(simple_ASK.speed());
  Serial.println("bps");

  pinMode(RFM69_RST, OUTPUT);

  digitalWrite(RFM69_RST, HIGH);
  delay(50); // NB. RFM69 is HIGH=RESET

  digitalWrite(RFM69_RST, LOW);
  delay(100);

  Serial.println("\nrf69 reset done OK");

  if (rf69.init()) Serial.println("rf69 radio init OK");

  else {
    Serial.println("rf69 radio init failed - hanging...");
    while (true) delay(1000);
  }

  Serial.print("rf69.setFrequency=");
  Serial.print(RF69_FREQ, 4);
  Serial.println(" MHz");

  rf69.setFrequency(RF69_FREQ);

  Serial.println("rf69.setTxPower=17");

  rf69.setTxPower(17, true); // range from 14-20 for power, 2nd arg must be true for RFM69HCW

  rf69.setModemRegisters( & OOK_Rb2Bw3_9);

  Serial.println("rf69.setModemConfig=done");

  byte bMSB = rf69.spiRead(RH_RF69_REG_03_BITRATEMSB), bLSB = rf69.spiRead(RH_RF69_REG_04_BITRATELSB);

  Serial.print("BitRate=0x");
  Serial.print(bMSB, HEX);
  Serial.write(' ');
  Serial.print(bLSB, HEX);

  Serial.print(" = ");
  Serial.print(32000000 / ((bMSB * 256) + bLSB));
  Serial.println("bps");

  Serial.print("OP-Mode=0x");
  Serial.println(rf69.spiRead(RH_RF69_REG_01_OPMODE), HEX);

  Serial.print("DataMode=0x");
  Serial.println(rf69.spiRead(RH_RF69_REG_02_DATAMODUL), HEX);

  #if(false) // Only enable if you want to reduce the number of pulses when viewing on G2(data) with an oscilloscope.

  Serial.print("Changing to fixed OOKPEAK: ");

  rf69.spiWrite(RH_RF69_REG_1B_OOKPEAK, 0);

  byte OOKPEAK = rf69.spiRead(RH_RF69_REG_1B_OOKPEAK);

  Serial.print("OOKPEAK=0x");
  Serial.print(OOKPEAK, HEX);
  Serial.print("=B");
  Serial.println(OOKPEAK, BIN);

  Serial.print("Change Fixed threshold OOKFIX: ");

  #define THESHOLD_DB 0x50

  rf69.spiWrite(RH_RF69_REG_1D_OOKFIX, THESHOLD_DB); // Fixed threshold value in the OOK demodulator=0x50dB.

  Serial.print("OOKFIX=0x");
  Serial.println(rf69.spiRead(RH_RF69_REG_1D_OOKFIX), HEX);

  #endif

  rf69.setModeRx(); // This line is needed [at the end of setup()] to allow "simple_ASK.recv()" to receive stuff

}

void loop() {

  uint8_t buf[20], len = sizeof(buf); // must be large enough for whole string (inc. any zeros)

  if (simple_ASK.recv(buf, & len)) {
    buf[len] = 0;
    Serial.print("OOK Got: ");
    Serial.println((char * ) buf);
  }

}