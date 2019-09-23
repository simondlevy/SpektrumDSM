/*
   Demo sketch for DSMRX library on ESP32

   Displays channel values in interval [1000, 2000]

   Copyright (C) Simon D. Levy 2019

   This file is part of DSMRX.

   DSMRX is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   DSMRX is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with DSMRX.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <DSMRX.h>

static const uint8_t CHANNELS = 8;

DSM2048 rx;

void coreTask(void * params)
{
    while (true) {
      
        if (Serial2.available()) {
           rx.handleSerialEvent(micros()); 
        }

        delay(1);
    } 
}

uint8_t dsmSerialAvailable(void)
{
    return Serial2.available();
}

uint8_t dsmSerialRead(void)
{
    return Serial2.read();
}

void setup(void)
{
    Serial.begin(115000);

    Serial2.begin(115000);

    TaskHandle_t task;
    xTaskCreatePinnedToCore(coreTask, "Task", 10000, NULL, 1, &task, 0); 
}

void loop(void)
{
    if (rx.timedOut(micros())) {
        Serial.println("*** TIMED OUT ***");
    }

    else if (rx.gotNewFrame()) {

        uint16_t values[CHANNELS];

        rx.getChannelValues(values, CHANNELS);

        for (int k=0; k<CHANNELS; ++k) {
            Serial.print("Ch. ");
            Serial.print(k+1);
            Serial.print(": ");
            Serial.print(values[k]);
            Serial.print("    ");
        }

        Serial.print("Fade count = ");
        Serial.println(rx.getFadeCount());
    }

    // Need a little loop delay
    delay(5);
}
