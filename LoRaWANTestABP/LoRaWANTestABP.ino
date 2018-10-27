
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <CayenneLPP.h>
#include <Adafruit_GPS.h>

#define LED 14
#define BUTTON 42
#define PPS 21
#define Serial SerialUSB
#define UNCONFIRMED 0
#define CONFIRMED 1
#define FIX_BLINK_DELAY 1000
Adafruit_GPS GPS(&Serial1);
boolean DEBUG = false;
boolean connected = false;
boolean sendLocation = true;
unsigned long lastBlink;

void blink(uint8_t n);
void do_send(osjob_t* j);

// Network Session Key, Application Session Key, and Device Address are provided by The Things Network when you create a device.
// Take care to specify these in most-significant-bit (MSB) format.

// MSB
static u1_t PROGMEM NWKSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
// MSB
static u1_t PROGMEM APPSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static u4_t DEVADDR = 0x00000000;


// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }


//static uint8_t mydata[] = "Hello, world!";
static osjob_t sendjob;

CayenneLPP lpp(51);

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 30;

// LMIC pin definitions for the SAMD21E board used as an Arduino Zero. SS is pin 10, DIO are pins 17 and 18.
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LMIC_UNUSED_PIN,
    .dio = {17, 18, LMIC_UNUSED_PIN},
};

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);
            connected = true;
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
      Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
      // Prepare upstream data transmission at the next possible time.
      lpp.reset();
      if (GPS.fix && sendLocation) {
        Serial.print("lat = ");
        Serial.print(GPS.latitudeDegrees);
        Serial.print("  lon = ");
        Serial.print(GPS.longitudeDegrees);
        Serial.print("  alt = ");
        Serial.print(GPS.altitude);
        Serial.println();
        lpp.addGPS(1, GPS.latitudeDegrees, GPS.longitudeDegrees, GPS.altitude);
      } else {
        lpp.addPresence(2, 1);
      }
      blink(3);
      LMIC_setTxData2(2, lpp.getBuffer(), lpp.getSize(), CONFIRMED);
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void initGPS() {
  GPS.begin(115200);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  //GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ); // every 5 seconds
  GPS.sendCommand("$PQTXT,W,0,0*22"); // disable GPTXT messages
}

void updateLED() {
  unsigned long now = millis();
  if ((GPS.fix) && (sendLocation) && ((now - lastBlink) > FIX_BLINK_DELAY)) {
    lastBlink = now;
    digitalWrite(LED, HIGH);
    delay(50);
    digitalWrite(LED, LOW);
  }
}

void blink(uint8_t n) {
  for(uint8_t i=0;i<n;i++) {
    digitalWrite(LED, HIGH);
    delay(50);
    digitalWrite(LED, LOW);
    delay(50);
  }
}

void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(PPS, INPUT);
  pinMode(LED, OUTPUT);
  while ((!Serial) && (digitalRead(BUTTON) == HIGH)) {
    digitalWrite(LED, HIGH);
    delay(10);
    digitalWrite(LED, LOW);
    delay(30);
  }

  digitalWrite(LED, HIGH);
  delay(200);
  digitalWrite(LED, LOW);
  delay(1000);

  lastBlink = millis();

  initGPS();

  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
  LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);


  Serial.println("using ABP...");
  LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
  LMIC_selectSubBand(1);
  // Disable link check validation
  LMIC_setLinkCheckMode(0);

  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF7,23);
  connected = true;

  // Start job (sending automatically starts OTAA too)
  do_send(&sendjob);
}

void loop() {
  os_runloop_once();
  GPS.read();
  if (GPS.newNMEAreceived()) {
    if (DEBUG) Serial.print(GPS.lastNMEA());
    GPS.parse(GPS.lastNMEA());
  }
  updateLED();
  if (digitalRead(BUTTON) == LOW) {
    unsigned long pressed = millis();
    while (digitalRead(BUTTON) == LOW) {
      if ((millis() - pressed) > 1000) {
        sendLocation = !sendLocation;
        break;
      }
    }
    os_clearCallback(&sendjob);
    do_send(&sendjob);
  }

}
