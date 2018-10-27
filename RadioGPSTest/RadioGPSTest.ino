
// LoRa radio connected to SPI1 pins on SAMD21
#define USE_SPI1

#include <SPI.h>
#include <RH_RF95.h>
#ifdef USE_SPI1
#include <RHHardwareSPI1.h>
#endif
#include <Adafruit_GPS.h>

#define LED 14
#define SS 10
#define DIO0 17
#define Serial SerialUSB
#define PPS 21
#define SEND_INTERVAL 15000

boolean firstTime = false;
Adafruit_GPS GPS(&Serial1);
#ifdef USE_SPI1
SPIClass SPI1(&sercom1, 12, 13, 11, SPI_PAD_0_SCK_1, SERCOM_RX_PAD_3);
RH_RF95 rf95(SS, DIO0, hardware_spi1);
#else
RH_RF95 rf95(SS, DIO0);
#endif
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
unsigned long nextSendTime = millis();

void waitForNMEA() {
  while (!GPS.newNMEAreceived()) {
    GPS.read();
  }
  Serial.println(GPS.lastNMEA());
}

void initGPS() {
  if (firstTime) {
    // factory default baud for L80 GPS module is 9600. Set it to 115200
    GPS.begin(9600);
    GPS.sendCommand("$PQBAUD,W,115200*43"); // set baud to 115200
    delay(1000);
  }
  GPS.begin(115200);

  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand("$PQTXT,W,0,0*22"); // disable GPTXT messages
}

void setup() {
  delay(2000);
  pinMode(PPS, INPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  delay(50);
  digitalWrite(LED, LOW);

  initGPS();

  Serial.print("rf95.init()...");
  while (!rf95.init()) {
    Serial.println("RF95 init failed!");
    delay(1000);
  }
  Serial.println("ready");

  rf95.setTxPower(23, false);
  rf95.setFrequency(915.0);
}

void loop() {
  if (GPS.fix) {
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }
  GPS.read();
  if (GPS.newNMEAreceived()) {
    //Serial.print(GPS.lastNMEA());
    GPS.parse(GPS.lastNMEA());
  }
  if ((GPS.fix) && (millis() > nextSendTime)) {
    nextSendTime += SEND_INTERVAL;
    char lat[16];
    char lon[16];
    strcpy(lat, String(GPS.latitudeDegrees, 4).c_str());
    strcpy(lon, String(GPS.longitudeDegrees, 4).c_str());
    sprintf((char *)buf, "lat=%s, lon=%s", lat, lon);
    Serial.println((char *)buf);

    rf95.send(buf, strlen((char *)buf));
    rf95.waitPacketSent();
  }

  if (rf95.available()) {
    // Should be a message for us now
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len)) {
      Serial.print("got message: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
    } else {
      Serial.println("recv failed");
    }
  }
}
