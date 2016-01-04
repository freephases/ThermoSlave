/**
* Thermocouple slave
* Simple program to send values from MAX31855 object to software serial ports 6 (rx) and 7 (tx)
*
*
*  Version 0.0.1.1 
*/

/**
* OnOff is simple class to manage digital ports see: https://github.com/freephases/arduino-onoff-lib
*/
#include <OnOff.h>
/**
* Software serial to send data to master - we do not need to read from master
*/
#include <SoftwareSerial.h>
#include <math.h>

//slave libs
#include <SPI.h>
#include <Adafruit_MAX31855.h>

//settings
#define DEBUG_TO_SERIAL 1
#define SLAVES_NAME "thermocouple slave"

const unsigned long readInterval = 100;//read every 2.5 sec, calcVI times out at 2 secs max
const int AVG_COUNT = 10;
int avgCount = 0;
//slave specifics
#define DO   3
#define CS   4
#define CLK  5

#define DO2   3
#define CS2  4
#define CLK2  5


//objects
Adafruit_MAX31855 thermocouple(CLK, CS, DO);
//Adafruit_MAX31855 thermocouple2(CLK2, CS2, DO2);
OnOff led(13);
SoftwareSerial master(6, 7);

unsigned long readMillis = 0; // last milli secs since last avg reading
typedef struct {
  double internal, temp;
} PayloadTX;      // create structure - a neat way of packaging data for RF comms
PayloadTX slaveData;
PayloadTX avgSlaveData;

void setupSlave()
{
  delay(500);
}


void resetAvg() {
  avgSlaveData.internal = 0.00;
  avgSlaveData.temp = 0.00;
  avgCount = 0;
}

char buf[100];

void sendSlaveData() {

  String tempStr = String(slaveData.internal);//map float to a string
  char internalTempBuf[16];
  tempStr.toCharArray(internalTempBuf, 16);//map float to char array

  String tempStr2 = String(slaveData.temp);//map float to a string
  char tempBuf[16];
  tempStr2.toCharArray(tempBuf, 16);//map float to char array

  sprintf(buf, "R|%s|%s|!", internalTempBuf, tempBuf); //build output up to send to master
  master.println(buf); //send result to master

  if (DEBUG_TO_SERIAL == 1) {
    Serial.println(buf);
  }
}

void readSlaveSensors() {
  if (millis() - readMillis > readInterval) {
    readMillis = millis();
    
    
    double c = thermocouple.readCelsius();
    if (isnan(c)) {
      if (DEBUG_TO_SERIAL == 1) {
        Serial.println("Something wrong with thermocouple!");
      }
      master.println("E|thermocouple error");
    } else {
      avgCount++;
      avgSlaveData.temp += c;
      avgSlaveData.internal += thermocouple.readInternal();
    }
    if (avgCount==AVG_COUNT) {
      led.on();
      slaveData.temp = avgSlaveData.temp/ (double) AVG_COUNT;    
      slaveData.internal = avgSlaveData.internal/ (double) AVG_COUNT;      
      sendSlaveData();
      resetAvg();
      led.off();
      
    }
  }
}
/*
send data
led.on();
         sendSlaveData();
         led.off();
*/

void setup() {
  led.on();
  Serial.begin(9600);
  Serial.println(SLAVES_NAME);
  delay(100);
  master.begin(9600);
  setupSlave();
  led.off();
}

void loop() {
  readSlaveSensors();
}



