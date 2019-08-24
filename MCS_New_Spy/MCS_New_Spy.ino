/*Quest room Main Control System (MCS)//

Based on Atmega 2560 Arduino Mega stock board plus custom pcb shield + modules directly connected to shield via pls/pbs wires:

Dfplayer mini as main sound module - uses DAC to output sound on 100W 32 dB amplifier
UART-RS485 on MAX385 to connect operator console with MCS
Solid state relay 4-channel module to commutate 220V loads
N-channel MOSFET 3*4-channel modules to comm 12V loads (LED-srips, electromagnetic locks, etc)
A several GPIO pins are used to directly connect sensors or buttons; 
Some devices uses own arduino or atiny controllers and connects to MSC through GPIO or RS485

Power supply is impulse 220-12V 10A
*/

#include <DFPlayer_Mini_Mp3.h>

#define rs485_direction_pin 2   //RS485 Direction control

//12V DC N-channel MOSFETs
#define M_1_LED_table_map 41 //orange+ brown- wire
#define M_2_EML_tumbler1 39 //orange+ brown- wire //HIGH for a 1 sec is opened then make LOW
#define M_3_EML_tumbler2 37 //orange+ brown- wire
#define M_4_EML_door1 35

#define M_5_EML_box_wires 29//brown-, orange+
#define M_6_LED_UV_dance_humans 27 //green+, blue- wire
#define M_7_EML_nude_wires 25 //brown-, orange+
#define M_8_EML_bomb_box 23

#define M_9_EML_door2 36
#define M_10_EML_exit_door1 34
#define M_11_EML_exit_door2 32
#define M_12 30

//Solid state relay AC 70-240V 2A
#define S_1_light_main_1 53
#define S_2_light_main_2 51
#define S_3_projector 49
#define S_4_red_light 47

#define S_5_green_light 22
#define S_6_white_light 24
#define S_7_yellow_light 26
#define S_8_blue_light 28

//analog inputs from sensors
#define B_1_start_but A0 //blue // WORK == LOW state
#define B_2_eyes_sens1 A1 //ok blue  //WAIT == 700, WORK <= 300                  fotoresistors
#define B_3_eyes_sens2 A2 //ok green //WAIT == 600, WORK <= 300                  fotoresistors
#define B_4_GK_wars A3//ok blue - sig, brown - gnd // WORK == LOW state
#define B_5_tumblers1 A4 // green - sig, blue - gnd  // WORK == LOW state
#define B_6_tumblers2 A5 // green - sig, blue - gnd // WORK == LOW state
#define B_7_panel_wires1 A6 //ok green - sig, blue - gnd; brown-,orange+ - 12V supply // WORK == LOW state
#define B_8_laser_aims1 A7 //blue - sig, brown - gnd; //WAIT == 85, WORK <= 30
#define B_9_exit_button A8 //blue - sig, brown - gnd // WORK == LOW state
#define B_10_panel_wires4 A9 //empty
#define B_11_red_but A10 //blue - sig, brown - gnd // WORK == LOW state //!!!!!!!! change button to NO
#define B_12_laser_aims2 A11 //orange - sig, green - gnd; //WAIT == 95, WORK <= 30
#define B_13_nude_wires A12 //green - sig, blue - gnd   //WAIT == 1000, WORK <= 900                 one is ground, other is analog input
#define B_14_bomb_ok A13 // blue-sig, brown - gnd                    from bomb
#define B_15_reset_but A14//green // WORK == LOW state
#define B_16_bomb_not_ok A15 // green - sig, brown - gnd;

//pwm outputss
#define U_0_bomb_start 3 //orange - sig, brown - gnd; work == LOW state
#define U_1_bomb_reset 4 //orangewhite - sig, brown - gnd; work == LOW state
#define U_2_tumbl1_led 5 //blue - gnd, green-white - sig
#define U_3_tumbl2_led 6 //blue - gnd, green-white - sig
#define U_4 7

bool was_red_but_pressed = false;
bool is_bomb_explosed = false;
bool is_wars_right = false;
bool is_wires_right = false;
bool is_final_but_pressed = false;

String string_reply; //variable to store card uids

//Card uids
String A_1 = "1#";
String A_2 = "2#";
String A_3 = "3#";
String A_4 = "4#";
String A_5 = "5#";
String A_6 = "6#";
String A_7 = "7#";
String A_8 = "8#";
String A_9 = "9#";
String A_10 = "10#";

bool a_1_start_flag = false; //start button
bool a_2_flag = false; //eyes sens -> map_led
bool a_3_flag = false;
bool a_4_flag = false;
bool a_5_flag = false;
bool a_6_reset_flag = false;  //reset_button
bool a_7_flag = false;
bool a_8_flag = false;
bool a_9_flag = false;
bool a_10_flag = false;

void setup()
{
	Serial.begin(9600);
	Serial1.begin(9600); //dfplayer serial
	Serial2.begin(9600); //rs485 serial

  mp3_set_serial(Serial1);
  mp3_stop(); // stop any track playing after controller reset 
  mp3_set_volume(0); //just for sure, that music isn't plays

  pinMode(rs485_direction_pin, OUTPUT); 
  digitalWrite(rs485_direction_pin, LOW); //receive mode

  pinMode(B_1_start_but, INPUT_PULLUP);
  pinMode(B_2_eyes_sens1, INPUT_PULLUP);
  pinMode(B_3_eyes_sens2, INPUT_PULLUP);
  pinMode(B_4_GK_wars, INPUT_PULLUP);
  pinMode(B_5_tumblers1, INPUT_PULLUP);
  pinMode(B_6_tumblers2, INPUT_PULLUP);
  pinMode(B_7_panel_wires1, INPUT_PULLUP);
  pinMode(B_8_laser_aims1, INPUT_PULLUP);
  pinMode(B_9_exit_button, INPUT_PULLUP);
  pinMode(B_10_panel_wires4, INPUT_PULLUP);
  pinMode(B_11_red_but, INPUT_PULLUP);
  pinMode(B_12_laser_aims2, INPUT_PULLUP);
  pinMode(B_13_nude_wires, INPUT_PULLUP);
  pinMode(B_14_bomb_ok, INPUT_PULLUP);
  pinMode(B_15_reset_but, INPUT_PULLUP);
  pinMode(B_16_bomb_not_ok, INPUT_PULLUP);

  pinMode(M_1_LED_table_map, OUTPUT);
  pinMode(M_2_EML_tumbler1, OUTPUT);
  pinMode(M_3_EML_tumbler2, OUTPUT);
  pinMode(M_4_EML_door1, OUTPUT);
  pinMode(M_5_EML_box_wires, OUTPUT);
  pinMode(M_6_LED_UV_dance_humans, OUTPUT);
  pinMode(M_7_EML_nude_wires, OUTPUT);
  pinMode(M_8_EML_bomb_box, OUTPUT);
  pinMode(M_9_EML_door2, OUTPUT);
  pinMode(M_10_EML_exit_door1, OUTPUT);
  pinMode(M_11_EML_exit_door2, OUTPUT);
  pinMode(M_12, OUTPUT);

  digitalWrite(M_1_LED_table_map, LOW);
  digitalWrite(M_2_EML_tumbler1, LOW);
  digitalWrite(M_3_EML_tumbler2, LOW);
  digitalWrite(M_4_EML_door1, HIGH);
  digitalWrite(M_5_EML_box_wires, HIGH);
  digitalWrite(M_6_LED_UV_dance_humans, LOW);
  digitalWrite(M_7_EML_nude_wires, HIGH);
  digitalWrite(M_8_EML_bomb_box, HIGH);
  digitalWrite(M_9_EML_door2, HIGH);
  digitalWrite(M_10_EML_exit_door1, HIGH);
  digitalWrite(M_11_EML_exit_door2, HIGH);
  digitalWrite(M_12, LOW);

  pinMode(S_1_light_main_1, OUTPUT);
  pinMode(S_2_light_main_2, OUTPUT);
  pinMode(S_3_projector, OUTPUT);
  pinMode(S_4_red_light, OUTPUT);
  pinMode(S_5_green_light, OUTPUT);
  pinMode(S_6_white_light, OUTPUT);
  pinMode(S_7_yellow_light, OUTPUT);
  pinMode(S_8_blue_light, OUTPUT);
  digitalWrite(S_1_light_main_1, HIGH);
  digitalWrite(S_2_light_main_2, HIGH);
  digitalWrite(S_3_projector, LOW);
  digitalWrite(S_4_red_light, LOW);
  digitalWrite(S_5_green_light, HIGH); //LOW-level trigger
  digitalWrite(S_6_white_light, HIGH); //LOW-level trigger
  digitalWrite(S_7_yellow_light, HIGH); //LOW-level trigger
  digitalWrite(S_8_blue_light, HIGH); //LOW-level trigger

  pinMode(U_0_bomb_start, OUTPUT);
  pinMode(U_1_bomb_reset, OUTPUT);
  pinMode(U_2_tumbl1_led, OUTPUT);
  pinMode(U_3_tumbl2_led, OUTPUT);
  pinMode(U_4, OUTPUT);
  digitalWrite(U_0_bomb_start, HIGH);
  digitalWrite(U_1_bomb_reset, HIGH);
  digitalWrite(U_2_tumbl1_led, LOW);
  digitalWrite(U_3_tumbl2_led, LOW);
  digitalWrite(U_4, LOW);

  reset_state(); //reset timer and all locks
  Serial.println("MCS_New_Spy v1.0 has been started");
  //check_SSR();
}

void loop()
{
  //read_all_but();

  digitalWrite(rs485_direction_pin, LOW); //rx
  delay(50);
  if(Serial.available()) 
    {
      string_reply = "";
      delay(100);
      rs485_recieve();
    }

  if(digitalRead(B_1_start_but) == LOW || a_1_start_flag == true)
  {
    delay(100);
    digitalWrite(U_0_bomb_start, LOW); //start a bomb timer
    delay(500);
    digitalWrite(U_0_bomb_start, HIGH);
    mp3_set_serial(Serial1);
    mp3_set_volume(25);
    mp3_play(1);
    a_1_start_flag = false;
  }

  if((analogRead(B_2_eyes_sens1) <= 300 && analogRead(B_3_eyes_sens2) <= 300) || a_2_flag == true)
  {
    delay(100);
    digitalWrite(M_1_LED_table_map, HIGH);
    a_2_flag = false;
  }

  if(digitalRead(B_4_GK_wars) == LOW && is_wars_right == false)
  {
    delay(100);
    digitalWrite(M_1_LED_table_map, LOW);
    digitalWrite(M_2_EML_tumbler1, HIGH);
    digitalWrite(M_3_EML_tumbler2, HIGH);
    delay(500);
    digitalWrite(M_2_EML_tumbler1, LOW);
    digitalWrite(M_3_EML_tumbler2, LOW);
    is_wars_right = true;
  }
  if(digitalRead(B_4_GK_wars) == HIGH)
  {
    is_wars_right = false;
  }

  //tumblers are done
  if(digitalRead(B_5_tumblers1) == LOW)
  {
    delay(100);
    digitalWrite(U_2_tumbl1_led, HIGH);
  }
  else
  {
    delay(100);
    digitalWrite(U_2_tumbl1_led, LOW);
  }

  //tumblers are done
  if(digitalRead(B_6_tumblers2) == LOW)
  {
    delay(100);
    digitalWrite(U_3_tumbl2_led, HIGH);
  }
  else
  {
    delay(100);
    digitalWrite(U_3_tumbl2_led, LOW);
  }

  if(digitalRead(B_5_tumblers1) == LOW && digitalRead(B_6_tumblers2) == LOW)
  {
    delay(100);
    digitalWrite(M_5_EML_box_wires, LOW);
    digitalWrite(M_9_EML_door2, LOW);
  }
  else if(digitalRead(B_5_tumblers1) == HIGH || digitalRead(B_6_tumblers2) == HIGH)
  {
    delay(100);
    digitalWrite(M_5_EML_box_wires, HIGH);
    digitalWrite(M_9_EML_door2, HIGH);
  }

  //all 4 panel wires at a right places
  if(digitalRead(B_7_panel_wires1) == LOW && is_wires_right == false)
  {
    delay(100);
    digitalWrite(S_2_light_main_2, LOW);
    digitalWrite(S_3_projector, HIGH);
    is_wires_right = true;
  }

  else if(digitalRead(B_7_panel_wires1) == HIGH && is_wires_right == true)
  {
    delay(100);
    digitalWrite(S_2_light_main_2, HIGH);
    digitalWrite(S_3_projector, LOW);
    is_wires_right = false;
  }

  //red button is pressed
  if(digitalRead(B_11_red_but) == LOW && was_red_but_pressed == false)
  {
    delay(100);
    digitalWrite(S_1_light_main_1, LOW);
    digitalWrite(S_2_light_main_2, LOW);
    digitalWrite(S_3_projector, LOW);
    digitalWrite(M_6_LED_UV_dance_humans, HIGH);
    was_red_but_pressed = true;
  }

  //red button is released
  else if(digitalRead(B_11_red_but) == HIGH && was_red_but_pressed == true) 
  {
    delay(100);
    digitalWrite(S_1_light_main_1, HIGH);
    digitalWrite(S_2_light_main_2, HIGH);
    digitalWrite(M_6_LED_UV_dance_humans, LOW);
    was_red_but_pressed = false;
  }

  //laser aims is shouted by lasers
  if(analogRead(B_8_laser_aims1) <= 30 && analogRead(B_12_laser_aims2) <= 30)
  {
    delay(30);
    digitalWrite(M_7_EML_nude_wires, LOW);

    for(int i=0;i<=2;i++)
    {
      digitalWrite(S_4_red_light, HIGH);
      delay(200);
      digitalWrite(S_4_red_light, LOW);
      delay(200);
      digitalWrite(S_5_green_light, LOW);
      delay(200);
      digitalWrite(S_5_green_light, HIGH);
      delay(200);
      digitalWrite(S_7_yellow_light, LOW);
      delay(200);
      digitalWrite(S_7_yellow_light, HIGH);
      delay(200);
      digitalWrite(S_4_red_light, HIGH);
      digitalWrite(S_5_green_light, LOW);
      delay(200);
      digitalWrite(S_4_red_light, LOW);
      digitalWrite(S_5_green_light, HIGH);
      delay(200);
      digitalWrite(S_7_yellow_light, LOW);
      delay(200);
      digitalWrite(S_7_yellow_light, HIGH);
      delay(200);
      digitalWrite(S_5_green_light, LOW);
      delay(200);
      digitalWrite(S_5_green_light, HIGH);
      delay(200);
      digitalWrite(S_8_blue_light, LOW);
      delay(200);
      digitalWrite(S_8_blue_light, HIGH);
    }
    delay(200);
    digitalWrite(S_7_yellow_light, LOW);//turn on and still
    delay(200);
    digitalWrite(S_4_red_light, HIGH);//turn on and still
  }

  //circle is shorted by human body
  if(analogRead(B_13_nude_wires) <= 900)
  {
    delay(100);
    digitalWrite(M_8_EML_bomb_box, LOW);
  }
  else if(analogRead(B_13_nude_wires) >= 900)
  {
    delay(100);
    digitalWrite(M_8_EML_bomb_box, HIGH);
  }

  //right wire is cutted
  if(digitalRead(B_14_bomb_ok) == LOW) //signal from bomb, that's all right
  {
    //nothing happens
  }

  //wrong wire is cutted
  if(analogRead(B_16_bomb_not_ok) <= 300 && is_bomb_explosed == false)
  {
    delay(100);
    mp3_set_serial(Serial1);
    mp3_set_volume(25);
    mp3_play(3); //loose music

    digitalWrite(M_10_EML_exit_door1, LOW); //open exit door1
    digitalWrite(M_11_EML_exit_door2, LOW); //open exit door2
    is_bomb_explosed = true;
  }

  //exit button is pressed
  if(digitalRead(B_9_exit_button) == LOW && is_final_but_pressed == false)
  {
    delay(100);
    mp3_set_serial(Serial1);
    mp3_set_volume(25);
    mp3_play(2); //win music

    digitalWrite(M_10_EML_exit_door1, LOW); //open exit door1
    digitalWrite(M_11_EML_exit_door2, LOW); //open exit door2
    is_final_but_pressed = true;
  }
  else if (digitalRead(B_9_exit_button) == HIGH && is_final_but_pressed == true)
  {
    is_final_but_pressed = false;
  }

  if(digitalRead(B_15_reset_but) == LOW || a_6_reset_flag == true)
  {
    delay(100);
    mp3_set_serial(Serial1);
    mp3_stop(); // stop any track playing after controller reset 
    mp3_set_volume(0); //just for sure, that music isn't plays
    reset_state();
    a_6_reset_flag = false;
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
      if (string_reply.equals(A_1)){a_1_start_flag = true;}

      if (string_reply.equals(A_2)){a_2_flag = true;}

      if (string_reply.equals(A_3)){a_3_flag = true;}

      if (string_reply.equals(A_4)){a_4_flag = true;}

      if (string_reply.equals(A_5)){a_5_flag = true;}

      if (string_reply.equals(A_6)){a_6_reset_flag = true;}

      if (string_reply.equals(A_7)){a_7_flag = true;}

      if (string_reply.equals(A_8)){a_8_flag = true;}

      if (string_reply.equals(A_9)){a_9_flag = true;}

      if (string_reply.equals(A_10)){a_10_flag = true;}

      /*
      digitalWrite(rs485_direction_pin, HIGH); // tx mode
      delay(100);
      Serial.print(string_reply);
      Serial.println(" - ok");
      delay(50);*/
      string_reply = "";
    }
  }
}

void reset_state()
{
  digitalWrite(rs485_direction_pin, HIGH);
  Serial2.println("RS485 -> MCS_New_Spy v1.0 has been started"); //print it to rs485 port
  delay(100);
  digitalWrite(rs485_direction_pin, LOW);

  was_red_but_pressed = false;
  is_bomb_explosed = false;
  is_wars_right = false;
  is_wires_right = false;
  is_final_but_pressed = false;

  pinMode(B_5_tumblers1, INPUT_PULLUP);
  pinMode(B_6_tumblers2, INPUT_PULLUP);

  digitalWrite(M_1_LED_table_map, LOW);
  digitalWrite(M_2_EML_tumbler1, HIGH);
  digitalWrite(M_3_EML_tumbler2, HIGH);
  digitalWrite(M_4_EML_door1, LOW);
  digitalWrite(M_5_EML_box_wires, LOW);
  digitalWrite(M_6_LED_UV_dance_humans, LOW);
  digitalWrite(M_7_EML_nude_wires, LOW);
  digitalWrite(M_8_EML_bomb_box, LOW);
  digitalWrite(M_9_EML_door2, LOW);
  digitalWrite(M_10_EML_exit_door1, LOW);
  digitalWrite(M_11_EML_exit_door2, LOW);
  digitalWrite(M_12, LOW);
  delay(500);
  digitalWrite(M_1_LED_table_map, LOW);
  digitalWrite(M_2_EML_tumbler1, LOW);
  digitalWrite(M_3_EML_tumbler2, LOW);
  digitalWrite(M_4_EML_door1, HIGH);
  digitalWrite(M_5_EML_box_wires, HIGH);
  digitalWrite(M_6_LED_UV_dance_humans, LOW);
  digitalWrite(M_7_EML_nude_wires, HIGH);
  digitalWrite(M_8_EML_bomb_box, HIGH);
  digitalWrite(M_9_EML_door2, HIGH);
  digitalWrite(M_10_EML_exit_door1, HIGH);
  digitalWrite(M_11_EML_exit_door2, HIGH);
  digitalWrite(M_12, LOW);

  digitalWrite(U_1_bomb_reset, LOW);
  delay(500);
  digitalWrite(U_1_bomb_reset, HIGH);

  digitalWrite(S_1_light_main_1, HIGH);
  digitalWrite(S_2_light_main_2, HIGH);
  digitalWrite(S_3_projector, LOW);
  digitalWrite(S_4_red_light, LOW);
  digitalWrite(S_5_green_light, HIGH); //LOW-level trigger
  digitalWrite(S_6_white_light, HIGH); //LOW-level trigger
  digitalWrite(S_7_yellow_light, HIGH); //LOW-level trigger
  digitalWrite(S_8_blue_light, HIGH); //LOW-level trigger
}
/*
void read_all_but()
{
  Serial.print("B1=");
  Serial.print(analogRead(B_1_start_but));
  Serial.print(";B2=");
  Serial.print(analogRead(B_2_eyes_sens1));
  Serial.print(";B3=");
  Serial.print(analogRead(B_3_eyes_sens2));
  Serial.print(";B4=");
  Serial.print(analogRead(B_4_GK_wars));
  Serial.print(";B5=");
  Serial.print(analogRead(B_5_tumblers1));
  Serial.print(";B6=");
  Serial.print(analogRead(B_6_tumblers2));
  Serial.print(";B7=");
  Serial.print(analogRead(B_7_panel_wires1));
  Serial.print(";B8=");
  Serial.print(analogRead(B_8_laser_aims1));
  Serial.print(";B9=");
  Serial.print(analogRead(B_9_exit_button));
  Serial.print(";B10=");
  Serial.print(analogRead(B_11_red_but));
  Serial.print(";B11=");
  Serial.print(analogRead(B_11_red_but));
  Serial.print(";B12=");
  Serial.print(analogRead(B_12_laser_aims2));
  Serial.print(";B13=");
  Serial.print(analogRead(B_13_nude_wires));
  Serial.print(";B14=");
  Serial.print(analogRead(B_14_bomb_ok));
  Serial.print(";B15=");
  Serial.print(analogRead(B_15_reset_but));
  Serial.print(";B16=");
  Serial.println(analogRead(B_16_bomb_not_ok));
}

void check_all_dev ()
{
  digitalWrite(M_1_LED_table_map, HIGH);
  delay(500);
  digitalWrite(M_2_EML_tumbler1, HIGH);
  delay(500);
  digitalWrite(M_3_EML_tumbler2, HIGH);
  delay(500);
  digitalWrite(M_4_EML_door1, HIGH);
  delay(500);
  digitalWrite(M_5_EML_box_wires, HIGH);
  delay(500);
  digitalWrite(M_6_LED_UV_dance_humans, HIGH);
  delay(500);
  digitalWrite(M_7_EML_nude_wires, HIGH);
  delay(500);
  digitalWrite(M_8_EML_bomb_box, HIGH);
  delay(500);
  digitalWrite(M_9_EML_door2, HIGH);
  delay(500);
  digitalWrite(M_10_EML_exit_door1, HIGH);
  delay(500);
  digitalWrite(M_11_EML_exit_door2, HIGH);
  delay(500);
  digitalWrite(M_12, HIGH);
  delay(2000);

  digitalWrite(M_1_LED_table_map, LOW);
  digitalWrite(M_2_EML_tumbler1, LOW);
  digitalWrite(M_3_EML_tumbler2, LOW);
  digitalWrite(M_4_EML_door1, LOW);
  digitalWrite(M_5_EML_box_wires, LOW);
  digitalWrite(M_6_LED_UV_dance_humans, LOW);
  digitalWrite(M_7_EML_nude_wires, LOW);
  digitalWrite(M_8_EML_bomb_box, LOW);
  digitalWrite(M_9_EML_door2, LOW);
  digitalWrite(M_10_EML_exit_door1, LOW);
  digitalWrite(M_11_EML_exit_door2, LOW);
  digitalWrite(M_12, LOW);
  
}

void check_SSR()
{  
  digitalWrite(S_1_light_main_1, HIGH);
  delay(500);
  digitalWrite(S_2_light_main_2, HIGH);
  delay(500);
  digitalWrite(S_3_projector, HIGH);
  delay(500);
  digitalWrite(S_4_red_light, HIGH);
  delay(500);
  digitalWrite(S_5_green_light, LOW);
  delay(500);
  digitalWrite(S_6_white_light, LOW);
  delay(500);
  digitalWrite(S_7_yellow_light, LOW);
  delay(500);
  digitalWrite(S_8_blue_light, LOW);

  delay(2000);
  digitalWrite(S_1_light_main_1, LOW);
  digitalWrite(S_2_light_main_2, LOW);
  digitalWrite(S_3_projector, LOW);
  digitalWrite(S_4_red_light, LOW);
  digitalWrite(S_5_green_light, HIGH);
  digitalWrite(S_6_white_light, HIGH);
  digitalWrite(S_7_yellow_light, HIGH);
  digitalWrite(S_8_blue_light, HIGH);
}
*/