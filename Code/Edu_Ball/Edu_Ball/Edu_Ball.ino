// Edu_Ball 31/05/2015 11:37

/* Pinout:
    Arduino 3.3V:
    A5 (SCL)-->SCL
    A4 (SDA)-->SDA
    D2-->INT DRDYG
    
    D8-->Reed Switch
    
    D6-->NeoPIxel DataIn
    
    A0-->Vraw/2
    
    D9-->CE NRF
    D10-->CSN NRF
    D13 (SCK)-->SCK NRF
    D11 (MOSI)-->MOSI NRF
    D12 (MISO)-->MISO NRF
    D3-->IRQ
    
    connector:
    Vcc--> baterry Vcc
    Dtr --> directly
    Tx --> through 1k // in order to protect from shorts
    Rx --> through 1k
    Grnd --> battery GRN
*/
#define BallNumber 1
///////////////
// libraries //
///////////////
#include <math.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <Adafruit_NeoPixel.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"

///////////////
// Variables //
///////////////

#define rad2deg 57.295
#define REEDSWITCHPIN 8

///////////////
// MPU 6050 //
///////////////

MPU6050 accelgyro;

#define MPUREADDELAY 10
// Single radio pipe address for the 2 nodes to communicate.

const byte INT_MPU_PIN = 2; // INT DRDYG

int16_t ax, ay, az;
int16_t gx, gy, gz;

double AcclValue[3]={0,0,0}; //in G values
double GyroValue[3]={0,0,0}; //in Degrees per second

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
// Single radio pipe address for the 2 nodes to communicate.
unsigned long CommunicationMillisStamp=0;

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(9,10);
// Radio pipe addresses for the 2 nodes to communicate.
//const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
const uint64_t Txpipes =0xB3F3F3b3b7LL;
const uint64_t Rxpipes =0xA3F3F3bA3c3LL;

/////////////
// Battery //
/////////////
double Vbaterry=3.7;
#define VBATTERY_PIN A0
#define ADC2VBAT 163.0 // Convert ADC reading to voltage, VBatt*10/57=Vref/ADC_Res Vref=1.1v ADC_Res=1023 
#define BATTERYMINIMUM 3.3 // minimum allowed battery voltage
byte BatteryStatus=1; // variable to indicate battery status,0- empty, 1-full
#define BatterySampleDelay 1000 // sample period of the battery voltage
unsigned long VbatSampleMillis=0; // variable to hold millis counter

////////////////
// Neo Pixels //
////////////////
#define NEOPIXELS 8
#define NEODATAPIN 6
// neo Pixel initialize
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEOPIXELS, NEODATAPIN, NEO_GRB + NEO_KHZ800);






///////////////
// SETUP	 //
///////////////
void setup(){
      // join I2C bus (I2Cdev library doesn't do this automatically)
    Wire.begin();
    // initialize serial communication
    Serial.begin(57600);
  // neo Pixel initialize
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
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
  //MPU6050 Settings
  // initialize device
    accelgyro.initialize();

    // verify connection
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

    // change accel/gyro scale values
    // set accelerometer to 16G and gyro to 2000 deg/sec
    accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
    accelgyro.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
  
  // set battery pin as input  with internal referense of 1.1v and read voltage
  analogReference(INTERNAL);
  delay(10);// settle down
  pinMode(VBATTERY_PIN, INPUT);
  Vbaterry=((double)analogRead(VBATTERY_PIN))/ADC2VBAT;
  
  // Set pin modes
  pinMode(REEDSWITCHPIN, INPUT);
  // interrupt pins
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  
  //initialize communication struct:
  Message.MSG.Millis=0;
  Message.MSG.Counter=0;
  
  radio.stopListening();
}

///////////////
// MAIN LOOP //
///////////////
void loop(){
  //Serial.println(millis());
	
	// update battery voltage
	if ((millis()-VbatSampleMillis)>BatterySampleDelay){
		VbatSampleMillis=millis();
		Vbaterry=((double)analogRead(VBATTERY_PIN))/ADC2VBAT;
		if (Vbaterry>(BATTERYMINIMUM+0.1)){ //add 0.1 in order to remove turning on and off when battery close to value.
			BatteryStatus=1;
		}else if (Vbaterry<BATTERYMINIMUM){
			BatteryStatus=0;
		}
		//Serial.println(Vbaterry);
	}
	 // accelerometer interrupt ready
	 if ((millis()-Message.MSG.Millis)>MPUREADDELAY){
            Message.MSG.Millis=millis();
            Message.MSG.Counter++;
            // read raw accel/gyro measurements from device
            accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
            Message.MSG.Acc[0]=ax;
            Message.MSG.Acc[1]=ay;
            Message.MSG.Acc[2]=az;
            Message.MSG.Gyr[0]=gx;
            Message.MSG.Gyr[1]=gy;
            Message.MSG.Gyr[2]=gz;
            radio.write( &Message.MBytes, MSGLENGHT );
                
//                AcclValue[0]=((double)ax)/SAMPLE2GACC;
//                AcclValue[1]=((double)ay)/SAMPLE2GACC;
//                AcclValue[2]=((double)az)/SAMPLE2GACC;
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
//                
          }         
}// end main
