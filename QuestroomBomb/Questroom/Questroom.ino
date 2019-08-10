#include <Wtv020sd16p.h>

//PUT YOUR CONSTANTS THERE
byte password[4] = {1,9,8,4};//Put your e-lock password there (in DEC, four digits) example {1,2,3,4}
byte correctWire = 2; //Put your correct Wire number there (form 1 to 8) example (2); 
int timeInSeconds = 3600; //Put your time for timer in seconds. i.e. 60 minutes = 3600 seconds

//PIN Definition
byte aResetPin = 3;  // The pin number of the reset pin.
byte aClockPin = 2;  // The pin number of the clock pin.
byte aDataPin = 4;  // The pin number of the data pin.

byte muxA = 14;
byte muxB = 16;
byte muxC = 10; 
byte muxOut = 5;

byte load1 = 7;
byte load2 = 8;

byte sDATA = A3;
byte sCLK = A1;
byte sLAT = A0;
byte sOE = 15;

byte dotLed = A2;

byte startBtn = 9;

//Time variables
unsigned long millisNow = 0;
unsigned long millisPrew = 0;
int interval = 1000;
boolean dotLedState = 1;

//global variables
byte diffCounter = 0;
int secondsLeft = timeInSeconds;

//display arrays
byte displayArray[4] = {255,255,255,255};//display memory

byte errorMsg[4] = {140,190,190,255};
byte nmbrMsg[4] = {186,189,255,255};
byte interstateMsg[4] = {191,191,191,191};
byte numArray[10] = {192,243,164,161,147,137,136,227,128,129};


//wire state arrays
boolean wireState[8] = {0,0,0,0,0,0,0,0};
boolean prevWireState[8] = {0,0,0,0,0,0,0,0};

Wtv020sd16p audio(aResetPin,aClockPin,aDataPin,NULL);

void diffCheck()
{
  muxRead();
  byte counter = 0;
  for (int i = 0;i<=7;i++)
  {
    
    if (wireState[i]!=prevWireState[i])
    {
      counter++;
    }
	
  }
  diffCounter = counter;
}



void updateDisplay()
{
  digitalWrite(sLAT,LOW);
  for (int i = 0;i<4;i++)
  {
    shiftOut(sDATA, sCLK, LSBFIRST, displayArray[i]); 
  }
  digitalWrite(sLAT,HIGH);
}



void setup() {
  audio.reset();
  //  Serial.begin(9600); //debug output
   // delay(1000);

  //pinmode setup
  pinMode(load1,OUTPUT);
  pinMode(load2,OUTPUT);
 
  pinMode(dotLed,OUTPUT);
 
  pinMode(sDATA,OUTPUT);
  pinMode(sCLK,OUTPUT);
  pinMode(sLAT,OUTPUT);
  pinMode(sOE,OUTPUT);

  pinMode(muxA,OUTPUT);
  pinMode(muxB,OUTPUT);
  pinMode(muxC,OUTPUT);
  pinMode(muxOut,INPUT_PULLUP);
  
  pinMode(startBtn,INPUT_PULLUP);
  
  //set init pin states
  digitalWrite(dotLed,HIGH);
  digitalWrite(load1,LOW);
  digitalWrite(load2,LOW);
  
  //clearDisplay form random garbage 
  clearDisplay();
  
  //check errors in wiring
  startErrorCheck();
  
  //wait for start button pressed
  for(int i = 0;i<4;i++)
  {
    displayArray[i]=interstateMsg[i];
  }
  updateDisplay();
  while (digitalRead(startBtn))
  {
  }
  millisNow = millis();
  millisPrew = millisNow;
}

void correctAnswer()
{ 
	for (int i = 0;i<5;i++)
	{
		diffCheck();
		if (diffCounter>2)
		{
			incorrectAnswer();
		}
		delay(5);
	}
	
  audio.asyncPlayVoice(0);//play audio tone
  for(int i = 0;i<4;i++)
  {
    displayArray[i]=interstateMsg[i];
  }
  updateDisplay();
  delay(2000);
  for(int i = 0;i<4;i++)
  {
    displayArray[i]=numArray[password[i]];
  }
  updateDisplay();
  digitalWrite(load2,HIGH);

  while (1)
  {
    digitalWrite(sOE,LOW);
    digitalWrite(dotLed,LOW);
    delay(1000);
    digitalWrite(sOE,HIGH);
    digitalWrite(dotLed,HIGH);
    delay(1000);
  }
}

void incorrectAnswer()
{
	audio.asyncPlayVoice(1);//play audio tone
  clearDisplay();
  digitalWrite(dotLed,HIGH);
  audio.playVoice(1);
  digitalWrite(load2,HIGH);
  delay(500);
  digitalWrite(load2,LOW);
  while(1)
  {
    digitalWrite(load1,HIGH);
  }
}


void clearDisplay()
{
  //clear display
  for(int i = 0;i<4;i++)
  {
    displayArray[i]=255;
    updateDisplay();
  }
  
  
}
void startErrorCheck ()
{
  //read wire state
  muxRead();
  byte wireCounter = 0;
  
  //count number of used wires
  for (int i = 0; i<8;i++)
  {
    wireCounter=wireCounter+wireState[i];
  }
  //check if correct number of wire pins used
  if (wireCounter<4)
  {
    for(int i = 0;i<3;i++)
    {
      displayArray[i]=errorMsg[i];
    }
    displayArray[3]=numArray[1];
    updateDisplay();
    infLoop();
  }
  //check if correct pin used
  if (wireState[correctWire-1]==0)
  {
    for(int i = 0;i<3;i++)
    {
      displayArray[i]=errorMsg[i];
    }
    displayArray[3]=numArray[2];
    updateDisplay();
    infLoop();
  } 
  //display 0 error message
  for(int i = 0;i<3;i++)
  {
    displayArray[i]=errorMsg[i];
  }
  displayArray[3]=numArray[0];
  updateDisplay();
  delay(2000);
  //display number of used wires
  for(int i = 0;i<3;i++)
  {
    displayArray[i]=nmbrMsg[i];
  }
  displayArray[3]=numArray[wireCounter];
  updateDisplay();
  delay(2000);
  //backup wire states
  for(int i = 0;i<=7;i++)
  {
    prevWireState[i]=wireState[i];
  }
}
void infLoop ()
{
  while(1){
    delay(1000);
  }
}

void muxRead()
{
  digitalWrite(muxA,LOW);
  digitalWrite(muxB,LOW);
  digitalWrite(muxC,LOW);//зел
  delay(5);
  wireState[3]=!(digitalRead(muxOut));
  digitalWrite(muxA,LOW);
  digitalWrite(muxB,LOW);
  digitalWrite(muxC,HIGH);//б.оранж
  delay(5);
  wireState[0]=!(digitalRead(muxOut));
  digitalWrite(muxA,LOW);
  digitalWrite(muxB,HIGH);
  digitalWrite(muxC,LOW);//кор
  delay(5);
  wireState[7]=!(digitalRead(muxOut));
  digitalWrite(muxA,LOW);
  digitalWrite(muxB,HIGH);
  digitalWrite(muxC,HIGH);//б.зел
  delay(5);
  wireState[2]=!(digitalRead(muxOut));
  digitalWrite(muxA,HIGH);
  digitalWrite(muxB,LOW);
  digitalWrite(muxC,LOW);//син
  delay(5);
  wireState[5]=!(digitalRead(muxOut));
  digitalWrite(muxA,HIGH);
  digitalWrite(muxB,LOW);
  digitalWrite(muxC,HIGH);//б.кор
  delay(5);
  wireState[6]=!(digitalRead(muxOut));
  digitalWrite(muxA,HIGH);
  digitalWrite(muxB,HIGH);
  digitalWrite(muxC,LOW);//оранж
  delay(5);
  wireState[1]=!(digitalRead(muxOut));
  digitalWrite(muxA,HIGH);
  digitalWrite(muxB,HIGH);
  digitalWrite(muxC,HIGH);//б.син
  delay(5);
  wireState[4]=!(digitalRead(muxOut));  

  
}
void loop() {

  byte minutes = 0;
  byte seconds = 0;
  while(secondsLeft>0)
  {
    millisNow = millis();
    diffCheck();

    Serial.print("ContactWireState");
    Serial.println(wireState[correctWire-1]);
    Serial.print("STATE = ");
  for (int i = 0;i<8;i++)
  {
    Serial.print(wireState[i]);
    Serial.print(",");
  } 
  Serial.println();
    if (prevWireState[correctWire-1]!=wireState[correctWire-1])//correct wire cut
    {
      correctAnswer();
    }
	if (diffCounter>1) //more then two wires cut
    {
      incorrectAnswer();
    }
    if (diffCounter>0)//one wire cut
    {
      interval = 500;
    }
    if(millisNow - millisPrew > interval) 
    { 
      millisPrew = millisNow;
      dotLedState = !(dotLedState);
      digitalWrite(dotLed,dotLedState);
      secondsLeft--;
      minutes = secondsLeft/60;
      seconds = secondsLeft%60;
      displayArray[0]=numArray[minutes/10];
      displayArray[1]=numArray[minutes%10];
      displayArray[2]=numArray[seconds/10];
      displayArray[3]=numArray[seconds%10];
      updateDisplay();
    }
    //delay(200);
  }
  incorrectAnswer();
  muxRead();//debug output
  diffCheck();
  clearDisplay();
  displayArray[3]=numArray[diffCounter];
  updateDisplay();
  Serial.print("ContactWireNumber");
  Serial.println(correctWire-1);
  Serial.print("ContactWireState");
  Serial.println(wireState[correctWire-1]);
  Serial.print("STATE = ");
  for (int i = 0;i<8;i++)
  {
    Serial.print(wireState[i]);
    Serial.print(",");
  } 
  Serial.println();
  delay(1000);
}
