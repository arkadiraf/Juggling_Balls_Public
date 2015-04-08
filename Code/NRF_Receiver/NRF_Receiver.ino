/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example of using interrupts
 *
 * This is an example of how to user interrupts to interact with the radio.
 * It builds on the pingpair_pl example, and uses ack payloads.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

//RF24 radio(8,7);
RF24 radio(9,10);
// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };


void setup(void)
{
//    pinMode(7,OUTPUT);
//  digitalWrite(7,HIGH);
//  delay(100);
  Serial.begin(57600);
  radio.begin();
  radio.setRetries(15,15);
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  
  radio.startListening();
  attachInterrupt(1, check_radio, FALLING);

}

void loop(void)
{
//   if (radio.available()){
//         // Dump the payloads until we've gotten everything
//      double angles[2];
//      bool done = false;
//      while (!done)
//      {
//        // Fetch the payload, and see if this was the last one.
//        done = radio.read( &angles[0], sizeof(double)*2 );
//        Serial.print("Angles: ");
//        Serial.print(angles[0]);
//        Serial.print(",");
//        Serial.println(angles[1]);
//      }
// }
   
}

void check_radio(void)
{
  // What happened?
  bool tx,fail,rx;
  radio.whatHappened(tx,fail,rx);
  if ( rx )
  {
   
         // Dump the payloads until we've gotten everything
      //unsigned long time = 0;
      double Vbat=0;
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &Vbat, sizeof(double) );
        Serial.print("Vbat: ");
        Serial.println(Vbat);
      }
  }
}

// vim:ai:cin:sts=2 sw=2 ft=cpp
