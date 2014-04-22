/*
Modification of the Pulse Sensor Amped 1.1 code to send data using the Firmata protocol. Code to blink
LED on pin 13 and fade LED on pin 5 in original version was removed for this Firmata version.

Jeff Hoefs 1/12/13

Edited notes from original example below:

>> Pulse Sensor Amped 1.1 <<
This code is for Pulse Sensor Amped by Joel Murphy and Yury Gitman
    www.pulsesensor.com 
    >>> Pulse Sensor purple wire goes to Analog Pin 0 <<<
Pulse Sensor sample aquisition and processing happens in the background via Timer 2 interrupt. 2mS sample rate.
PWM on pins 3 and 11 will not work when using this code, because we are using Timer 2!
The following variables are automatically updated:
Signal :    int that holds the analog signal data straight from the sensor. updated every 2mS.
IBI  :      int that holds the time interval between beats. 2mS resolution.
BPM  :      int that holds the heart rate value, derived every beat, from averaging previous 10 IBI values.
QS  :       boolean that is made true whenever Pulse is found and BPM is updated. User must reset.
Pulse :     boolean that is true when a heartbeat is sensed then returns to false until the next heartbeat.

Code Version 02 by Joel Murphy & Yury Gitman  Fall 2012
This update changes the HRV variable name to IBI, which stands for Inter-Beat Interval, for clarity.
Switched the interrupt to Timer2 (use of Timer2 disables PWM on pins 3 & 11). 
500Hz sample rate, 2mS resolution IBI value.
Tidied up inefficiencies since the last version. 
*/

#include <Firmata.h>

#define PULSE_PIN      0

// define the binary protocol for the sensor interface
#define PULSE_SENSOR   0x0E       // sensor identifier (any value between 0x00 and 0x0F)
#define SIGNAL_ID      0x00       // id for signal data
#define BPM_ID         0x01       // id for BPM data
#define IBI_ID         0x02       // id for IBI data

#define START_READING  0x01
#define STOP_READING   0x02


//  VARIABLES
byte dataBuffer[3];                // array to store data type (1 byte) and value (2 bytes)
unsigned long currentMillis;       // sotre the current value from millis()
unsigned long previousMillis;      // for comparison with currentMillis
boolean reading = false;           // do not read on startup


// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, the Inter-Beat Interval
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.


void setup() {
  //Firmata.setFirmwareVersion(2, 3);
  Firmata.attach(START_SYSEX, sysexCallback);  // listen for incoming data from client application
  Firmata.begin(57600);             // Firmata baud rate is currently fixed at 57600
  
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 
  
   // UN-COMMENT THE NEXT LINE IF YOU ARE POWERING The Pulse Sensor AT LOW VOLTAGE, 
   // AND APPLY THAT VOLTAGE TO THE A-REF PIN
   //analogReference(EXTERNAL);   
}


void loop() {
  // check for data from the client application
  while (Firmata.available()) {
    Firmata.processInput();
  }
  
  currentMillis = millis();
  if (currentMillis - previousMillis > 20) {
    previousMillis += 20;                // run this every 20ms  
    
    if (reading) {
      reportPulseData();
    }
  }
}


void reportPulseData() {
  sendData(SIGNAL_ID, Signal);         // send Processing the raw Pulse Sensor data
  
  if (QS == true) {                    // Quantified Self flag is true when arduino finds a heartbeat
    sendData(BPM_ID, BPM);             // send heart rate with a 'B' prefix
    sendData(IBI_ID, IBI);             // send time between beats with a 'Q' prefix
    QS = false;                        // reset the Quantified Self flag for next time    
  }  
}


// send data to client application
void sendData(byte id, int value) {
  dataBuffer[0] = id;                      // identifies type of data (Signal, BPM or IBI)
  dataBuffer[1] = value & 0x00FF;          // least-significant byte
  dataBuffer[2] = (value >> 8) & 0x00FF;   // most-significant byte
  Firmata.sendSysex(PULSE_SENSOR, 3, dataBuffer);
}

// start reading the pulse sensor
void startReading() {
  //sei(); // need to disable timer 2 only, ont all interrupts
  reading = true;  
}

// stop reading the pulse sensor
void stopReading() {
  reading = false; 
  //cli(); // need to disable timer 2 only, not all interrupts
}


// handle incoming data from client application
void sysexCallback(byte command, byte argc, byte *argv) {
  byte action;

  if (command == PULSE_SENSOR) {
    action = argv[0];
    
    if (action == START_READING) {
      startReading();
    } else if (action == STOP_READING) {
      stopReading();
    }
  } 
}

