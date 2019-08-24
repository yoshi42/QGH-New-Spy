//lazy code for 10-button console:
//buttons connected to pins 3-12 and reading with for-loop
// when button was pushed - its number sends via RS485

#define SerialTxControl 2 //RE+DE outputs on max485 are connected to pin D2

String string_reply;
String a_1 = "1#";

void setup() {
  Serial.begin(9600);

  for(int i = 3; i <= 12; i++)
  {
  	pinMode(i, INPUT_PULLUP); //initiate read mode for pins 3-12
  }

  pinMode(SerialTxControl, OUTPUT);
  digitalWrite(SerialTxControl, HIGH); // tx mode

  Serial.println("console started");
  delay(50);
}

void loop()
{
	digitalWrite(SerialTxControl, LOW); //rx
  delay(50);
  if(Serial.available()) 
    {
      string_reply = "";
      delay(100);
      rs485_recieve();
    }

	for(int i = 3; i <= 12; i++)
  {
  	if(digitalRead(i) == LOW)
  	{
    	delay(50);
      digitalWrite(SerialTxControl, HIGH); // tx mode
  		Serial.print(13-i); // to show button numbers 1 to 10
  		Serial.println("#"); //stop byte
      delay(500);
  	}
  }
}

void rs485_recieve() 
{              
  //recieve something from rs485 inerface
  while (Serial.available())
  {
    char inChar = Serial.read();
    string_reply += inChar;
    if (inChar == '#')
    {
      if (string_reply.equals(a_1))
      {
        digitalWrite(SerialTxControl, HIGH); // tx mode
        delay(100);
        Serial.println("1# recieved");
        delay(50);
      }
      /*
      digitalWrite(SerialTxControl, HIGH); // tx mode
      delay(100);
      Serial.print(string_reply);
      Serial.println(" - ok");
      delay(50);*/
      string_reply = "";
    }
  }
}