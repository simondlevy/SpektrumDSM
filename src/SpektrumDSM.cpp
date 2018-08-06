/*
   SpektrumDSM.cpp : implementation of interrupt-based Spektrum DSM receiver for Arduino

   Copyright (C) Simon D. Levy 2017

   This file is part of SpektrumDSM.

   SpektrumDSM is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   SpektrumDSM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with SpektrumDSM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "SpektrumDSM.h"

SpektrumDSM::SpektrumDSM(uint8_t rcChans, uint8_t chanShift, uint8_t chanMask, uint8_t valShift)
{
    _rcChans = rcChans;
    _chanShift = chanShift;
    _chanMask = chanMask;
    _valShift = valShift;

    _gotNewFrame = false;
    _lastInterruptMicros = 0;
}

void SpektrumDSM::handleSerialEvent(uint32_t usec)
{
    // Reset time 
    _lastInterruptMicros = usec;

    // check for new frame, i.e. more than 2.5ms passed
    static uint32_t spekTimeLast;
    uint32_t spekTimeNow = usec;
    uint32_t spekInterval = spekTimeNow - spekTimeLast;
    spekTimeLast = spekTimeNow;
    if (spekInterval > 2500) {
        _rxBufPos = 0;
    }

    // put the data in buffer
    while ((dsmSerialAvailable()) && (_rxBufPos < BUFFER_SIZE)) {
        _rxBuf[_rxBufPos++] = dsmSerialRead();
    }

    // parse frame if done
    if (_rxBufPos == BUFFER_SIZE) {

        // grab fade count
        _fadeCount = _rxBuf[0];

        // convert to channel data in [0,1024]
        for (int b = 2; b < BUFFER_SIZE; b += 2) {
            uint8_t bh = _rxBuf[b];
            uint8_t bl = _rxBuf[b+1];
            uint8_t spekChannel = 0x0F & (bh >> _chanShift);
            if (spekChannel < _rcChans) {
                _rcValue[spekChannel] = ((((uint16_t)(bh & _chanMask) << 8) + bl) >> _valShift);
            }
        }

        // we have a new frame
        _gotNewFrame = true;
    }
}


bool SpektrumDSM::gotNewFrame(void)
{
    bool retval = _gotNewFrame;
    if (_gotNewFrame) {
        _gotNewFrame = false;
    }
    return retval;
}

void SpektrumDSM::getChannelValues(uint16_t values[], uint8_t count)
{
    for (uint8_t k=0; k<count; ++k) {
        values[k] = _rcValue[k] + 988;
    }
}

void SpektrumDSM::getChannelValuesNormalized(float values[], uint8_t count)
{
    for (uint8_t k=0; k<count; ++k) {
        values[k] = (_rcValue[k] - 512) / 512.f;
    }
}

uint8_t SpektrumDSM::getFadeCount(void)
{
    return _fadeCount;
}

bool SpektrumDSM::timedOut(uint32_t usec, uint32_t maxMicros)
{
    
    uint32_t lag = usec - _lastInterruptMicros;
    return  lag > maxMicros;
}

SpektrumDSM1024::SpektrumDSM1024(void) : SpektrumDSM(7, 2, 0x03, 0)
{
}

SpektrumDSM2048::SpektrumDSM2048(void) : SpektrumDSM(8, 3, 0x07, 1)
{
}
