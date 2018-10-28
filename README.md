# SAMD21 LoRa GPS Module

This repo has example code for the [SAMD21-based LoRa GPS development board described here](https://nootropicdesign.com/projectlab/2018/10/27/samd21-lora-gps).
This board can be programmed with Arduino as an Arduino Zero. The board uses a SAMD21E18A microcontroller which has fewer pins than the 'G' variant of this chip used on the Arduino Zero, and the Arduino Zero pins used for SPI are not present in this package. The way to get around this is to either define a new Arduino core for this chip variant, or to define a new SPI interface on pins that we do have. I want to program this chip as an ordinary Arduino Zero, so I defined a new SPI interface called `SPI1` according to [this Adafruit tutorial](https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/creating-a-new-spi).

This required some modifications to certain libraries, described below.


### RadioGPSTest

Simple example demonstrating the use of the Quectel L80 GPS module and HopeRF LoRa module. The files `RHHardwareSPI1.cpp` and `RHHardwareSPI1.h` extend the RadioHead library so that it can use the SPI1 interface defined in the sketch by this code:
```
#ifdef USE_SPI1
SPIClass SPI1(&sercom1, 12, 13, 11, SPI_PAD_0_SCK_1, SERCOM_RX_PAD_3);
RH_RF95 rf95(SS, DIO0, hardware_spi1);
#else
```

Dependencies:

* [RadioHead library](http://www.airspayce.com/mikem/arduino/RadioHead/) - Powerful Arduino library for a variety of radio modules
* [Adafruit GPS library](https://github.com/adafruit/Adafruit_GPS) - Arduino GPS library for NMEA sentence parsing


### LoRaWANTestABP

LoRaWAN node that uses the ABP method of connecting to a gateway. While considered less secure, this method connects much faster and is more reliable.
[See the project description here](https://nootropicdesign.com/projectlab/2018/10/28/lorawan-end-devices/).

Dependencies:

* [arduino-lmic library](https://github.com/nootropicdesign/arduino-lmic) - LoRaWAN stack for embedded devices. This fork of the library has been modified to support the SPI1 interface used by the SAMD21E.
* [CayenneLPP library](https://github.com/sabas1080/CayenneLPP) - Cayenne low-power payload for efficient packing of GPS data into LoRaWAN packets
* [Adafruit GPS library](https://github.com/adafruit/Adafruit_GPS) - Arduino GPS library for NMEA sentence parsing



### LoRaWANTestOTAA

LoRaWAN node that uses the over-the-air activation (OTAA) method of connecting to a gateway.
[See the project description here](https://nootropicdesign.com/projectlab/2018/10/28/lorawan-end-devices/).

* [arduino-lmic library](https://github.com/nootropicdesign/arduino-lmic) - LoRaWAN stack for embedded devices. This fork of the library has been modified to support the SPI1 interface used by the SAMD21E.
* [CayenneLPP library](https://github.com/sabas1080/CayenneLPP) - Cayenne low-power payload for efficient packing of GPS data into LoRaWAN packets
* [Adafruit GPS library](https://github.com/adafruit/Adafruit_GPS) - Arduino GPS library for NMEA sentence parsing
