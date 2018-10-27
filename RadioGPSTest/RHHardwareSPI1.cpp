// RHHardwareSPI1.h
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2011 Mike McCauley
// Contributed by Joanna Rutkowska
// $Id: RHHardwareSPI1.cpp,v 1.19 2018/01/06 23:50:45 mikem Exp mikem $

#include <RHHardwareSPI1.h>
#include "wiring_private.h"

// Declare a single default instance of the hardware SPI interface class
RHHardwareSPI1 hardware_spi1;

#ifdef RH_HAVE_HARDWARE_SPI

RHHardwareSPI1::RHHardwareSPI1(Frequency frequency, BitOrder bitOrder, DataMode dataMode)
    :
    RHGenericSPI(frequency, bitOrder, dataMode)
{
}

uint8_t RHHardwareSPI1::transfer(uint8_t data)
{
    return SPI1.transfer(data);
}

void RHHardwareSPI1::attachInterrupt()
{
#if (RH_PLATFORM == RH_PLATFORM_ARDUINO || RH_PLATFORM == RH_PLATFORM_NRF52)
    SPI1.attachInterrupt();
#endif
}

void RHHardwareSPI1::detachInterrupt()
{
#if (RH_PLATFORM == RH_PLATFORM_ARDUINO || RH_PLATFORM == RH_PLATFORM_NRF52)
    SPI1.detachInterrupt();
#endif
}

void RHHardwareSPI1::begin()
{
    // Perhaps this is a uniform interface for SPI?
    // Currently Teensy and ESP32 only
   uint32_t frequency;
   if (_frequency == Frequency16MHz)
       frequency = 16000000;
   else if (_frequency == Frequency8MHz)
       frequency = 8000000;
   else if (_frequency == Frequency4MHz)
       frequency = 4000000;
   else if (_frequency == Frequency2MHz)
       frequency = 2000000;
   else
       frequency = 1000000;

#if ((RH_PLATFORM == RH_PLATFORM_ARDUINO) && defined (__arm__) && (defined(ARDUINO_SAM_DUE) || defined(ARDUINO_ARCH_SAMD))) || defined(ARDUINO_ARCH_NRF52)
    // Arduino Due in 1.5.5 has its own BitOrder :-(
    // So too does Arduino Zero
    ::BitOrder bitOrder;
#else
    uint8_t bitOrder;
#endif

   if (_bitOrder == BitOrderLSBFirst)
       bitOrder = LSBFIRST;
   else
       bitOrder = MSBFIRST;

    uint8_t dataMode;
    if (_dataMode == DataMode0)
	dataMode = SPI_MODE0;
    else if (_dataMode == DataMode1)
	dataMode = SPI_MODE1;
    else if (_dataMode == DataMode2)
	dataMode = SPI_MODE2;
    else if (_dataMode == DataMode3)
	dataMode = SPI_MODE3;
    else
	dataMode = SPI_MODE0;

    // Save the settings for use in transactions
   _settings = SPISettings(frequency, bitOrder, dataMode);
   SPI1.begin();
   pinPeripheral(11, PIO_SERCOM);
   pinPeripheral(12, PIO_SERCOM);
   pinPeripheral(13, PIO_SERCOM);
}

void RHHardwareSPI1::end()
{
    return SPI1.end();
}

void RHHardwareSPI1::beginTransaction()
{
#if defined(SPI_HAS_TRANSACTION)
    SPI1.beginTransaction(_settings);
#endif
}

void RHHardwareSPI1::endTransaction()
{
#if defined(SPI_HAS_TRANSACTION)
    SPI1.endTransaction();
#endif
}

void RHHardwareSPI1::usingInterrupt(uint8_t interrupt)
{
#if defined(SPI_HAS_TRANSACTION)

#endif
}

#endif
