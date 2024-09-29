#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>
#include <Arduino.h>

//Setup the brewtrollers register bank
//All of the data accumulated will be stored here
modbusDevice regBank;
//Create the modbus slave protocol handler
modbusSlave slave;

//Global Set Up
int State; //State to Set Pin output to
unsigned long interval = 500; //Delay for on/off time of temp outputs
unsigned long previousMillis; //Last Miliseconds recorded
unsigned long currentMillis; //Current Miliseconds
unsigned long wakeupMillis; //Last Millis recored at wakeup
unsigned long wakeupInterval = 70000; //Interval for Wakeup
bool needWakeup; //if wakeup pulse is needed

int testInt = 0;
int testInt2 = 10;
int testInt3 = 10;

//Pin Set Up
const int TempUpPin = 2;
const int TempDownPin = 3;
const int OnOffPin = 4;
const int SpeedPin = 5;

//function declerations
void wakeupTest(int wakeupPin);
void rangeLimit(int low, int high, int reg);
void setPointMatch(int setPointReg, int valueToMatchReg, int upPin, int downPin, int isRunningReg);
void multiStateSelector(int selectorSetpointReg, int selectorValueToMatchReg, int numOfSelections, int selectorControlPin, int selectorRunningReg);

//functions definitions
void rangeLimit(int low, int high, int reg){
    slave.run();
    int testValue = regBank.get(reg);

    if (testValue < low){
    regBank.set(reg, low);
  }
    else if (testValue > high){
    regBank.set(reg, high);
  }
}

void setPointMatch(int setPointReg, int valueToMatchReg, int upPin, int downPin, int isRunningReg){
    int setPoint = regBank.get(setPointReg);
    int valueToMatch = regBank.get(valueToMatchReg);
    
    if(setPoint != valueToMatch){
      regBank.set(isRunningReg,1);
      currentMillis = millis();
      wakeupTest(TempUpPin);
      if(setPoint > valueToMatch){
          if (currentMillis - previousMillis >= interval) {
        // save the last time you blinked the LED
          previousMillis = currentMillis;
            if (State == HIGH) {
              State = LOW;
              slave.run();
            } 
            else {
              State = HIGH;
              valueToMatch = valueToMatch + 1;
              regBank.set(valueToMatchReg, valueToMatch);
              slave.run();
            }
            digitalWrite(upPin,State);            
          }
      }


      else if(setPoint < valueToMatch){
          if (currentMillis - previousMillis >= interval) {
        // save the last time you blinked the LED
          previousMillis = currentMillis;
            if (State == HIGH) {
              State = LOW;
              slave.run();
            } 
            else {
              State = HIGH;
              valueToMatch = valueToMatch - 1;
              regBank.set(valueToMatchReg, valueToMatch);
              slave.run();
            }
            digitalWrite(downPin,State);
          }
      }
    }
    else{
      regBank.set(isRunningReg,0);
    }
}

void wakeupTest(int wakeupPin){
  if (currentMillis - wakeupMillis >= wakeupInterval){
    needWakeup = true;
    while(needWakeup == true){
      currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        // save the last time you blinked the LED
          previousMillis = currentMillis;
          slave.run();
            if (State == HIGH) {
              State = LOW;
            } 
            else {
              State = HIGH;
              needWakeup = false;
              wakeupMillis = millis();
            }
        digitalWrite(wakeupPin,State);
      }
      
    }
  }
}

void multiStateSelector(int selectorSetpointReg, int selectorValueToMatchReg, int numOfSelections, int selectorControlPin, int selectorRunningReg){
    int selectorSetpoint = regBank.get(selectorSetpointReg);
    int selectorValueToMatch = regBank.get(selectorValueToMatchReg);

    if(selectorValueToMatch > numOfSelections - 1){
      selectorValueToMatch = 0;
    }

    if(selectorSetpoint != selectorValueToMatch){
      regBank.set(selectorRunningReg,1);
      currentMillis = millis();
      wakeupTest(TempUpPin);
      if (currentMillis - previousMillis >= interval) {
          previousMillis = currentMillis;
            if (State == HIGH) {
              State = LOW;
              slave.run();
            } 
            else {
              State = HIGH;
              selectorValueToMatch = selectorValueToMatch + 1;
              regBank.set(selectorValueToMatchReg, selectorValueToMatch);
              slave.run();
            }
            digitalWrite(selectorControlPin, State);            
          } 
      regBank.set(selectorRunningReg,0);
      }
}

void setup()
{


//Assign the modbus device ID.  
  regBank.setId(1);

/*
modbus registers follow the following format
00001-09999  Digital Outputs, A master device can read and write to these registers
10001-19999  Digital Inputs, A master device can only read the values from these registers
30001-39999  Analog Inputs, A master device can only read the values from these registers
40001-49999  Analog Outputs, A master device can read and write to these registers 

Analog values are 16 bit unsigned words stored with a range of 0-32767
Digital values are stored as bytes, a zero value is OFF and any nonzer value is ON

It is best to configure registers of like type into contiguous blocks.  this
allows for more efficient register lookup and and reduces the number of messages
required by the master to retrieve the data

*/

//Add Digital Output registers 00001-00016 to the register bank
  regBank.add(1); //Temp Setpoint Matching
  regBank.add(2);
  regBank.add(3);
  regBank.add(4);
  regBank.add(5);
  regBank.add(6);
  regBank.add(7);
  regBank.add(8);
  regBank.add(9);
  regBank.add(10);
  regBank.add(11);
  regBank.add(12);
  regBank.add(13);
  regBank.add(14);
  regBank.add(15);
  regBank.add(16);

//Add Digital Input registers 10001-10008 to the register bank
  regBank.add(10001);  
  regBank.add(10002);  
  regBank.add(10003);  
  regBank.add(10004);  
  regBank.add(10005);  
  regBank.add(10006);  
  regBank.add(10007);  
  regBank.add(10008);  

//Add Analog Input registers 30001-10010 to the register bank
  regBank.add(30001);  
  regBank.add(30002);  
  regBank.add(30003);  
  regBank.add(30004);  
  regBank.add(30005);  
  regBank.add(30006);  
  regBank.add(30007);  
  regBank.add(30008);  
  regBank.add(30009);  
  regBank.add(30010);  

//Add Analog Output registers 40001-40020 to the register bank
  regBank.add(40001); //Temp Setpoint
  regBank.add(40002); //Current Temp Value
  regBank.add(40003); //Speed Setpoint 
  regBank.add(40004); //Current Speed Value
  regBank.add(40005);  
  regBank.add(40006);  
  regBank.add(40007);  
  regBank.add(40008);  
  regBank.add(40009);  
  regBank.add(40010);  
  regBank.add(40011);  
  regBank.add(40012);  
  regBank.add(40013);  
  regBank.add(40014);  
  regBank.add(40015);  
  regBank.add(40016);  
  regBank.add(40017);  
  regBank.add(40018);  
  regBank.add(40019);  
  regBank.add(40020);  

/*
Assign the modbus device object to the protocol handler
This is where the protocol handler will look to read and write
register data.  Currently, a modbus slave protocol handler may
only have one device assigned to it.
*/
  slave._device = &regBank;  

// Initialize the serial port for coms at 9600 baud  
  slave.setBaud(19200);   
  
//Set Output Pins
  pinMode(TempUpPin,OUTPUT); // Temperature Up Pin
  pinMode(TempDownPin,OUTPUT); //Temperature Down Pin
  pinMode(OnOffPin,OUTPUT);
  pinMode(SpeedPin,OUTPUT);
  
  digitalWrite(OnOffPin,HIGH);
  digitalWrite(SpeedPin,HIGH);
  digitalWrite(TempUpPin,HIGH);
  digitalWrite(TempDownPin,HIGH);
  
  regBank.set(40001,64); // Setting Temp Setpoint to 64 to start
  regBank.set(40002,64); // Setting Temp Current to 64 to start
  regBank.set(40003,1); // Setting Speed Setpooint to 1 to start
  regBank.set(40004,1); // Setting Speed Current to 1 to start
  
  State = HIGH;
  currentMillis = millis();
  wakeupMillis = currentMillis - 70000;
}

void loop()
{
  currentMillis = millis();
  rangeLimit(64, 86, 40001); //Limit the Temp Setpoint
  multiStateSelector(40003,40004,4,SpeedPin,2); //Select Speed
  setPointMatch(40001,40002,TempUpPin,TempDownPin,1); //Match Temp Setpoint

}
