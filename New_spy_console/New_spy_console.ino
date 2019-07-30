#define SerialTxControl 2

void setup() {
  Serial.begin(9600);

  for(int i = 3; i <= 12; i++)
  {
  	pinMode(i, INPUT_PULLUP); //initiate read mode for pins 3-12
  }

  pinMode(SerialTxControl, OUTPUT);
  digitalWrite(SerialTxControl, HIGH); // tx mode

  Serial.println("console started");
}

void loop()
{
	digitalWrite(SerialTxControl, LOW); //rx - custom mode
	for(int i = 3; i <= 12; i++)
  {
  	if(digitalRead(i) == LOW)
  	{
  		delay(100);
		digitalWrite(SerialTxControl, HIGH); // tx mode
		Serial.print(13-i); // to show button numbers 1 to 10
		Serial.println("#"); //stop byte
  	}
  }
}