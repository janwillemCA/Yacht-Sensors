This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <SoftwareSerial.h> //library for serial connections

SoftwareSerial Bluetooth(2, 3); //Pin 2, 3 == RX, TX
String command = "";

int pressConversionTable[6] = {710,679,614,537,468,387}; //Array of known resistance values of the oil pressure sensor
int tempConversionTable[17] = {780,736,689,638,586,534,484,437,394,352,315,280,248,218,196,184,171}; //Array of known resistance values of the temperature sensor
double pressure;
double temperature;
int batteryStatus;

#define TEMP_STEPSIZE (5)
#define TEMP_OFFSET (20)
#define SIGNAL_TEMP (0)
#define SIGNAL_PRESS (1)
#define SIGNAL_BATT (2)
#define TEMP_LIMIT (80)
#define PRESS_LIMIT (2)
#define BATT_ERROR (1)
#define BATT_OK (0)
#define OPEN_SENSOR (1000)

void setup() 
{
  setPins(); //declare all used pins for leds
  setConnection(); //set BC417 ready for communication
  batteryStatus = BATT_OK;
}

void loop() 
{  
  readModule(); //Read all information in console --> Will be send to the IOS application
  readpressConversionTable(); //Pressure measurements will be send to the IOS application
  readtempConversionTable();
  readBattStatus();
 
  updateSignalLights();
 
  sendBluetoothMessages();
 
  delay(100);
}

void setPins()
{
    pinMode(7, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(13, OUTPUT);
    digitalWrite(7, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(10, HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(12, HIGH);
    digitalWrite(13, HIGH);
}

void setConnection()
{
  Bluetooth.begin(9600); //connection for bluetooth
  Serial.begin(9600); //serial debug
  Bluetooth.write("AT");
  delay(1000);
  Bluetooth.write("AT+STATE?");
  delay(1000);
}

void readModule()
{
  if(Bluetooth.available())
  {
    while(Bluetooth.available())
    {
      command += (char)(Bluetooth.read());
    }
    Serial.println(command);
    command = "";
  }
  if(Serial.available())
  {
    delay(10);
    Bluetooth.write(Serial.println());
  }
}

void sendBluetoothMessages()
{  
  String bluetoothMessage = "T";
  bluetoothMessage += temperature;
  bluetoothMessage += "P";
  bluetoothMessage += pressure;
  bluetoothMessage += "B";
  bluetoothMessage += batteryStatus;
  
  Bluetooth.println(bluetoothMessage);  
  Serial.println(bluetoothMessage);
}

void readpressConversionTable()
{
  int sensorValue = analogRead(A5);  
  pressure = 99;
  if(sensorValue < pressConversionTable[0])
  {
    for(int i = 0; i < 6; i++)   //loop trough the pressure array
    {
      if(pressConversionTable[i] <= sensorValue)
      {
        double valTemp = sensorValue - pressConversionTable[i];
        double deltaVal = pressConversionTable[i-1] - pressConversionTable[i];
        double factor = valTemp / deltaVal;
        pressure = i - factor;
        break;    
      }
    }
  }
  else
  {
    pressure = 0;
  }
}

void readtempConversionTable()
{
  int sensorValue = analogRead(A3);  
  temperature = 999;
  if(sensorValue < tempConversionTable[0])
  {
    for(int i = 0; i < 17; i++)   //loop trough the pressure array
    {
      if(tempConversionTable[i] <= sensorValue)
      {
        double valTemp = sensorValue - tempConversionTable[i];
        double deltaVal = tempConversionTable[i-1] - tempConversionTable[i];
        double factor = valTemp / deltaVal;
        temperature = i * TEMP_STEPSIZE - factor * TEMP_STEPSIZE + TEMP_OFFSET;
        break;    
      }
    }
  }
  else
  {
    if(sensorValue > OPEN_SENSOR)
    {
      temperature = 999;
    }
    else
    {
      temperature = 0;
    }
  }
}

void readBattStatus ()
{
  int Val = analogRead(A1);
  
  if ( Val < 0xbf)
  {
    batteryStatus = BATT_ERROR;
  }
  else
  {
    batteryStatus = BATT_OK;
  }
  
  batteryStatus = analogRead(A1);
}

void greenLight(int lamp)
{
  switch(lamp)
  {
    case SIGNAL_TEMP: 
        digitalWrite(9, HIGH);
        digitalWrite(7, LOW);
        break;
        
    case SIGNAL_PRESS: 
        digitalWrite(11, HIGH);
        digitalWrite(10, LOW);
        break;
        
    case SIGNAL_BATT: 
        digitalWrite(13, HIGH);
        digitalWrite(12, LOW);
        break;
    
    default:
        digitalWrite(9, HIGH);
        digitalWrite(7, HIGH);
        digitalWrite(11, HIGH);
        digitalWrite(10, HIGH);
        digitalWrite(13, HIGH);
        digitalWrite(12, HIGH);  
        break;      
  }
}

void redLight(int lamp)
{
  switch(lamp)
  {
    case SIGNAL_TEMP: 
        digitalWrite(7, HIGH);
        digitalWrite(9, LOW);
        break;
        
    case SIGNAL_PRESS: 
        digitalWrite(10, HIGH);
        digitalWrite(11, LOW);
        break;
        
    case SIGNAL_BATT: 
        digitalWrite(12, HIGH);
        digitalWrite(13, LOW);
        break;
    
    default:
        digitalWrite(9, HIGH);
        digitalWrite(7, HIGH);
        digitalWrite(11, HIGH);
        digitalWrite(10, HIGH);
        digitalWrite(13, HIGH);
        digitalWrite(12, HIGH);  
        break;  
  }
}

void updateSignalLights()
{
  if(temperature > TEMP_LIMIT)
  {
     redLight(SIGNAL_TEMP);
  }
  else
  {
    greenLight(SIGNAL_TEMP);
  }
  
  if(pressure < PRESS_LIMIT)
  {
     redLight(SIGNAL_PRESS);
  }
  else
  {
    greenLight(SIGNAL_PRESS);
  }
  
  if(batteryStatus == BATT_ERROR)
  {
     redLight(SIGNAL_BATT);
  }
  else
  {
    greenLight(SIGNAL_BATT);
  }
}


