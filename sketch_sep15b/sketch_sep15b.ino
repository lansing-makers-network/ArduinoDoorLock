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
#include <Bounce.h>

#define RFID_SOUT 11    //PIN to RDID SOUT Pin
#define RFID_ENABLE 2   //PIN to RFID Enable Pin
#define RED_LED 5       //PIN to RGB LED Red Pin
#define GREEN_LED 4     //PIN to RGB LED Green Pin
#define BLUE_LED 3      //PIN to RGB LED Blue Pin
#define DIR 6           //PIN to OPEN direction on motor
#define MOTOR_CONT 7    //PIN TO Motor Control
#define CLOSE_COMP 8    //PIN to MAX CLOSE contact  **NOT USED
#define OPEN_COMP 9     //PIN to MAX OPEN contact   **NOT USED
#define KEEP_OPEN 10    //PIN to KEEP OPEN switch.

#define START_BYTE 0x0A  //RFID Start code.
#define STOP_BYTE 0x0D   //RFID Stop code.

// EEPROM Memory Space is defined as :
// -------------   ------  ---------------------------------------------------
// [0000 - 0001]   int     Number of authorized cards in memory
// [0001 - 0011]   String  Magic Card used to reprogram Arduino
// [0012 - 1023]   String@ IDs of authorized cards (each card taking 10 bytes)  

 String curTag;         //Last card number that was read.
 int numTagInMem;       //Number of cards in the DB.
 boolean keepOpenLock = false;   //Current state of the keep-on switch
 boolean doorUnlocked = true; // Current state of the door.
 SoftwareSerial RFID(RFID_SOUT, 12);

 Bounce keepOpenInput = Bounce(KEEP_OPEN, 10);
 
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
  
  digitalWrite(RFID_ENABLE, LOW);    //prepare RFID reader to accept data
  resetLEDS();
  
  //On Startup, drive the motor to open door (since this is the only absolute state currently..)
  openDoor(3000);
  
  
  //Read the keep open switch and obey it
  keepOpenInput.update();
  keepOpenLock = keepOpenInput.read();
  if (!keepOpenLock) {
  	closeDoor();
  }
  
  numTagInMem = EEPROM.read(0);
  
  
  
  Serial.println("Lansing Makers Network Door Access System.  Valid commands are (r,i,d,h,o,c)");
}

void loop()
{
  if(readCard())
  {
     digitalWrite(RFID_ENABLE, HIGH);
     if ((!doorUnlocked) && checkAccess(curTag,true))
     {
       openDoor();
       delay(15000);  //wait 15 seconds before locking door again
       closeDoor();
     }
     else 
     {
       delayAndBlink(2000, RED_LED);
     }
     clearSerialBuffer();     
     digitalWrite(RFID_ENABLE, LOW);
  }
  
  if (keepOpenInput.update()) {
  	keepOpenLock = keepOpenInput.read();
  	if (keepOpenLock) 
  	{
      openDoor();
    }
    else
    {
      closeDoor();
    }
  }
  if(Serial.available() > 0)
  {
    serialMenu();
  }
}

void resetLEDS()
{
  // Reset the LEDs to their extinguished state.
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);  
}

boolean readCard()
{
  // returns true if a card was successfully read.  returns false in all other senerios.
  // we expect this function to be in a loop
  //
  // stores read card as a String in the "curTag" global variable.
  
  boolean tagRead = false;
  int rfidByte;
  
  if(RFID.available() > 0)
  {
   rfidByte = RFID.read();
   if (rfidByte == STOP_BYTE)
   {
     Serial.println("]");
     tagRead = false;
     return true;
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
  return false;
}

boolean checkAccess(String tag, boolean checkMagic)
{
  //  checks the database if the card is "valid".  The tag passed in is the one checked.
  //  the second boolean is if we should check for the magic card. we may want to ignore it.
  //  returns if the card is valid.  

  String memTag;
  boolean foundKey = false;
  // tag number 0 is the magic key

  for (int loc=0; loc < 10; loc++)
  {
     memTag = String(memTag + char(EEPROM.read(1+loc)));
  }

  if ((tag == memTag) && (checkMagic))
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
    if (tag == memTag)
    {
      // we have a tag in memory
      foundKey = true;
    }
  }
  return foundKey;
  
}

void programKey()
{
  // we saw the magic card, and now we need to either add or remove the following card.
  // it will blink 3 times blue to indicate programming mode.
  
  String memTag;
  
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

  while (!readCard())
  {
    //wait for the card to be read 
  }

  digitalWrite(RFID_ENABLE, HIGH);
  if (checkAccess(curTag,false))
  {
    //the card already exists in the db.  remove it. 
     digitalWrite(RED_LED, HIGH);
     delay(500);
     digitalWrite(RED_LED, LOW);
     delay(500);
     digitalWrite(RED_LED, HIGH);
     delay(500);
     digitalWrite(RED_LED, LOW);
     
     for (int i=1; i <= numTagInMem; i++)
     {
        memTag = "";
        for (int loc=0; loc < 10; loc++)
        {
            memTag = String(memTag + char(EEPROM.read((i*10)+1+loc)));
        }
        if (curTag == memTag)
        {
           // we now have the index of the card in memory.
           for (int loc=0; loc < 10; loc++)
           {
            EEPROM.write((i*10)+loc+1,'*');  //replace selected tag with *s
           }  
        }
      }
  }
  else
  {
    //the card is new. add it.
     numTagInMem++;
     
     Serial.print("Adding key to memory location ");
     Serial.println(numTagInMem);
     
     EEPROM.write(0,numTagInMem);
     for (int loc=0; loc < 10; loc++)
     {
       EEPROM.write((numTagInMem * 10)+loc+1,curTag.charAt(loc));
     }    
     digitalWrite(GREEN_LED, HIGH);
     delay(500);
     digitalWrite(GREEN_LED, LOW);
     delay(500);
     digitalWrite(GREEN_LED, HIGH);
     delay(500);
     digitalWrite(GREEN_LED, LOW);
  } 
}

void serialMenu()
{
   char resp;
   int respValue;
 
   digitalWrite(BLUE_LED, HIGH);
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
     case 'i':
       while (Serial.read() >= 0)
        ; // clear read buffer 
       Serial.println("Are you sure you wish to re-initilze the database?  (y/n)");
       while (Serial.available() == 0)
       {
          //wait. 
       }
       resp = Serial.read();
       if (resp == 'y')
       {
          Serial.print("Re-Initilizing database....  ");
          numTagInMem = 0;
          EEPROM.write(0,numTagInMem);
          delay(1000);
          Serial.println("DONE!");
       }
       else
       {
         delay(1000); 
         Serial.println("Aborted."); 
       }
       break;
     case 'o':
       openDoor();
       break;
     case 'c':
       closeDoor();
       break;         
     case 'd':
       while (Serial.read() >= 0)
        ; // clear read buffer 
       respValue = 0;
       Serial.print("Enter the index of the card you wish to have removed from the database. [1-");
       Serial.print(numTagInMem);
       Serial.println("].  Terminate with a dot (.)  Use 99 to cancel.");
       while(true)
       {
          resp = Serial.read();
          if(resp>47 && resp<58)
          {
            // we have a valid number
            respValue = ((respValue * 10) + (resp - 48));
          }
          if (resp == 46)
          {
            // number was finished
            break;
          }
       }
       if(respValue > numTagInMem)
       {
         Serial.println("The index number is not value.  No tags have been deleted.");
       }
       else
       {
         Serial.print("Removing door code :");
         for (int loc=0; loc < 10; loc++)
         {
          Serial.print(char(EEPROM.read((respValue*10)+1+loc)));
         }
         Serial.println("");
         for (int loc=0; loc < 10; loc++)
         {
            EEPROM.write((respValue*10)+loc+1,'*');  //replace selected tag with *s
         }          
       }
       break;
     case 'h':
       Serial.println("-----------------------------");
       Serial.println("Door Access Admin Application");
       Serial.println("-----------------------------");
       Serial.println("[r] Read database");
       Serial.println("[i] Init database");
       Serial.println("[d] Delete item from database");
       Serial.println("[o] Open Door");
       Serial.println("[c] Close Door");
       break;
   }
   while (Serial.read() >= 0)
    ; // clear read buffer 
    
   resetLEDS();  
}

void clearSerialBuffer()
{
  // this replicates the .flush() function of Arduino < 1.0.3.  
   while (RFID.read() >= 0)
     ; // clear read buffer
}

void openDoor(int driveTime = 2000)
{
   // unlatch the door.  
   Serial.println("Unlatching Door");
   digitalWrite(DIR,HIGH);
   digitalWrite(MOTOR_CONT, HIGH);
   blinkAndDelay(driveTime, GREEN_LED);
   digitalWrite(MOTOR_CONT, LOW);
   digitalWrite(GREEN_LED, HIGH);
   doorUnlocked = true;  
}

void closeDoor(int driveTime = 1000)
{
  // re-latch the door
   Serial.println("Latching Door");
   digitalWrite(DIR,LOW);
   digitalWrite(MOTOR_CONT, HIGH);
   delay(driveTime);
   digitalWrite(MOTOR_CONT, LOW);
   resetLEDS();
   doorUnlocked = false;
}

void blinkAndDelay(int delayTime, int pin) {
	digitalWrite(pin, HIGH);
	for (int i=0; i<delayTime; i++) {
		if ((i % 500) == 0) {
			digitalWrite(pin, HIGH);
		} else if ((i % 250) == 0) {
			digitalWrite(pin, LOW);
		}
		delay(250);
		i = i + 250;
	}
}
