// Edu_Ball 31/05/2015 11:37

/* Pinout:
    Arduino Nano 5V: 
    D9-->CE NRF
    D10-->CSN NRF
    D13 (SCK)-->SCK NRF
    D11 (MOSI)-->MOSI NRF
    D12 (MISO)-->MISO NRF
    D3-->IRQ
*/
///////////////
// libraries //
///////////////
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

///////////////
// Variables //
///////////////

// declrae Message in unione format 
#define MSGLENGHT 18
union Message_Type
{
  uint8_t  MBytes[MSGLENGHT]; //Blue Green Red
    struct MSG_Type
  {
    uint32_t Millis;
    uint16_t Counter;
    int16_t Acc[3];
    int16_t Gyr[3];  
  }MSG;
}Message;

#define SAMPLE2GACC 2048.0   // +/-16g full scale, 16 bit resolution--> 2^16/32=2048
#define SAMPLE2AGYRO 16.384  // +/-2000 Deg/Sec, 16 but tesolution--> 2^16/4000=16.384
	
////////////////////////////
// communication variable //
////////////////////////////

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(9,10);
// Radio pipe addresses for the 2 nodes to communicate.
//const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
const uint64_t Rxpipes =0xB3F3F3b3b7LL;
const uint64_t Txpipes =0xA3F3F3bA3c3LL;







///////////////
// SETUP	 //
///////////////
void setup(){
    // initialize serial communication
    Serial.begin(115200);
  
  // Nrf initialize
  radio.begin();
  radio.setChannel(52);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_1MBPS);
  radio.setPayloadSize(MSGLENGHT);
  radio.setRetries(0,0);
  radio.setAutoAck(false);
  radio.openWritingPipe(Txpipes);
  radio.openReadingPipe(1,Rxpipes);
  radio.startListening();

  // interrupt pins
  pinMode(3, INPUT);
  
  //initialize communication struct:
  Message.MSG.Millis=0;
  Message.MSG.Counter=0;
  
}

///////////////
// MAIN LOOP //
///////////////
void loop(){
	 // receive messega
	 if (radio.available()){
            radio.read( &Message.MBytes, MSGLENGHT );
//            Serial.print("mill: ");
//            Serial.print(Message.MSG.Millis);
//            Serial.print(" Cnt: ");
//            Serial.print(Message.MSG.Counter);
//            Serial.print(" acc: ");
//            Serial.print(Message.MSG.Acc[0]);
//            Serial.print(", ");
//            Serial.print(Message.MSG.Acc[1]);
//            Serial.print(", ");
//            Serial.print(Message.MSG.Acc[2]);
//            Serial.print(", ");
//            Serial.print(Message.MSG.Gyr[0]);
//            Serial.print(", ");
//            Serial.print(Message.MSG.Gyr[1]);
//            Serial.print(", ");
//            Serial.println(Message.MSG.Gyr[2]);
            
            // send to matlab/simulink:
            Serial.print("MSG: ");
            Serial.print(Message.MSG.Millis);
            Serial.print(",");
            Serial.print(Message.MSG.Counter);
            Serial.print(",");
            Serial.print(Message.MSG.Acc[0]);
            Serial.print(",");
            Serial.print(Message.MSG.Acc[1]);
            Serial.print(",");
            Serial.print(Message.MSG.Acc[2]);
            Serial.print(",");
            Serial.print(Message.MSG.Gyr[0]);
            Serial.print(",");
            Serial.print(Message.MSG.Gyr[1]);
            Serial.print(",");
            Serial.println(Message.MSG.Gyr[2]);
                
          }         
}// end main
