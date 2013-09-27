/*  -------------------------------------------------------------------------
    Copyright 2013 - Lansing Makers Network

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
    --------------------------------------------------------------------------
    Initial Project by Nick Kwiatkowski (@quetwo)  Released as Apache License on 9/20/13
    
*/

#include <EEPROM.h>
#include <SoftwareSerial.h>

#define RFID_SOUT 11    //PIN to RDID SOUT Pin
#define RFID_ENABLE 2   //PIN to RFID Enable Pin
#define RED_LED 5       //PIN to RGB LED Red Pin
#define GREEN_LED 4     //PIN to RGB LED Green Pin
#define BLUE_LED 3      //PIN to RGB LED Blue Pin
#define DIR 6           //PIN to OPEN direction on motor
#define MOTOR_CONT 7    //PIN TO Motor Control
#define CLOSE_COMP 8    //PIN to MAX CLOSE contact
#define OPEN_COMP 9     //PIN to MAX OPEN contact
#define KEEP_OPEN 10    //PIN to KEEP OPEN switch.

#define START_BYTE 0x0A 
#define STOP_BYTE 0x0D

// EEPROM Memory Space is defined as :
// -------------   ------  ---------------------------------------------------
// [0000 - 0001]   int     Number of authorized cards in memory
// [0001 - 0011]   String  Magic Card used to reprogram Arduino
// [0012 - 1023]   String@ IDs of authorized cards (each card taking 10 bytes)  

 int rfidByte;
 boolean tagRead = false;
 String curTag;
 int numTagInMem;
 
 SoftwareSerial RFID(RFID_SOUT, 12);

void setup() 
{ 
  RFID.begin(2400);    //RFID Reader is locked to 2400 baud
  Serial.begin(9600);
  
  pinMode(OPEN_COMP,INPUT);
  pinMode(CLOSE_COMP,INPUT);
  pinMode(KEEP_OPEN, INPUT);
  pinMode(RFID_ENABLE, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(MOTOR_CONT, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  
  digitalWrite(RFID_ENABLE, LOW);    
  resetLEDS();
  
  numTagInMem = EEPROM.read(0);
  //numTagInMem = 0;
  
  Serial.print("I know of ");
  Serial.print(numTagInMem);
  Serial.println(" authorized keys.");
}

void loop()
{
  while(RFID.available() > 0)
  {
   rfidByte = RFID.read();
   if (rfidByte == STOP_BYTE)
   {
     Serial.println("]");
     tagRead = false;
     digitalWrite(RFID_ENABLE, HIGH);
     checkAccess();
     delay(2000);
     clearSerialBuffer();     
     resetLEDS();
     digitalWrite(RFID_ENABLE, LOW);
   }
   if (tagRead)
   {
      curTag = String(curTag + char(rfidByte));
      Serial.print(char(rfidByte));
   }
   if (rfidByte == START_BYTE)
   {
     tagRead = true;
     curTag = "";
     Serial.print("Read Card [");
   }
  }
  if(Serial.available() > 0)
  {
   switch(Serial.read())
   {
     case 'r':
       Serial.println("-----------------------------");
       Serial.print("Number of tags in Memory : ");
       Serial.println(numTagInMem);
       Serial.println("-----------------------------");
       Serial.print("Magic Key: ");
       for (int loc=0; loc < 10; loc++)
       {
          Serial.print(char(EEPROM.read(1+loc)));
       }
       Serial.println("");
       Serial.println("-----------------------------");
       for (int i=1; i <= numTagInMem; i++)
       {
        Serial.print(i);
        Serial.print(" | ");
        for (int loc=0; loc < 10; loc++)
        {
         Serial.print(char(EEPROM.read((i*10)+1+loc)));
         }
         Serial.println("");
       }
       
     break;
   }
   while (RFID.read() >= 0)
    ; // clear read buffer 
  }
}

void resetLEDS()
{
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);  
}

void checkAccess()
{
  String memTag;
  boolean foundKey = false;
  // tag number 0 is the magic key

  for (int loc=0; loc < 10; loc++)
  {
     memTag = String(memTag + char(EEPROM.read(1+loc)));
  }

  if (curTag == memTag)
  {
     // we have our magic "programming key"
     programKey();
  }  
  
  for (int i=1; i <= numTagInMem; i++)
  {
    memTag = "";
    for (int loc=0; loc < 10; loc++)
    {
       memTag = String(memTag + char(EEPROM.read((i*10)+1+loc)));
    }
    if (curTag == memTag)
    {
      // we have a tag in memory
      digitalWrite(GREEN_LED, HIGH); 
      foundKey = true;
    }
  }
  
  if (foundKey)
 {
  openDoor();
  delay(15000);
  closeDoor();
 }
  else
 {
    digitalWrite(RED_LED, HIGH); 
 } 
  
}

void programKey()
{
  boolean cardProgramming = true;
  digitalWrite(RFID_ENABLE, HIGH);
  RFID.flush();
  digitalWrite(BLUE_LED, HIGH);
  delay(500);
  digitalWrite(BLUE_LED, LOW);
  delay(200);
  digitalWrite(BLUE_LED, HIGH);
  delay(500);
  digitalWrite(BLUE_LED, LOW);
  clearSerialBuffer();
  digitalWrite(RFID_ENABLE, LOW);     
  while(cardProgramming)
  {
   rfidByte = RFID.read();
   if (rfidByte == -1)
   {
     continue;
   }
   if (rfidByte == STOP_BYTE)
   {
     tagRead = false;
     digitalWrite(RFID_ENABLE, HIGH);

     // write card to memory
     numTagInMem++;
     
     Serial.print("Adding the following key to memory location ");
     Serial.println(numTagInMem);
     Serial.println(curTag);
     
     EEPROM.write(0,numTagInMem);
     for (int loc=0; loc < 10; loc++)
     {
       EEPROM.write((numTagInMem * 10)+loc+1,curTag.charAt(loc));
     }    
     cardProgramming = false;

     digitalWrite(GREEN_LED, HIGH);
     delay(500);
     digitalWrite(GREEN_LED, LOW);
     delay(500);
     digitalWrite(GREEN_LED, HIGH);
     delay(500);
     digitalWrite(GREEN_LED, LOW);
   }
   if (tagRead)
   {
      curTag = String(curTag + char(rfidByte));
   }
   if (rfidByte == START_BYTE)
   {
     tagRead = true;
     curTag = "";
   }
  } 
}

void clearSerialBuffer()
{
   while (RFID.read() >= 0)
     ; // clear read buffer
}

void openDoor()
{
   Serial.println("Opening Door");
   digitalWrite(DIR,LOW);
   digitalWrite(MOTOR_CONT, HIGH);
   delay(1000);
   digitalWrite(MOTOR_CONT, LOW);  
}

void closeDoor()
{
   Serial.println("Closing Door");
   digitalWrite(DIR,HIGH);
   digitalWrite(MOTOR_CONT, HIGH);
   delay(1000);
   digitalWrite(MOTOR_CONT, LOW);
}
