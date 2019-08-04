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
#define M_2_EML_tumbler1 39 //orange+ brown- wire
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

#define S_5_light 22
#define S_6_light 24
#define S_7_light 26
#define S_8_light 28

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
#define B_11_red_but A10 //blue - sig, brown - gnd // WORK == LOW state
#define B_12_laser_aims2 A11 //orange - sig, green - gnd; //WAIT == 95, WORK <= 30
#define B_13_nude_wires A12 //green - sig, blue - gnd   //WAIT == 1000, WORK <= 900                 one is ground, other is analog input
#define B_14_bomb_ok A13 //                     from bomb
#define B_15_reset_but A14//green // WORK == LOW state
#define B_16_bomb_not_ok A15

//pwm outputss
#define U_0 3
#define U_1_bomb_start 4
#define U_2_bomb_reset 5
#define U_3 6
#define U_4 7

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
  digitalWrite(M_4_EML_door1, LOW);
  digitalWrite(M_5_EML_box_wires, LOW);
  digitalWrite(M_6_LED_UV_dance_humans, LOW);
  digitalWrite(M_7_EML_nude_wires, LOW);
  digitalWrite(M_8_EML_bomb_box, LOW);
  digitalWrite(M_9_EML_door2, LOW);
  digitalWrite(M_10_EML_exit_door1, LOW);
  digitalWrite(M_11_EML_exit_door2, LOW);
  digitalWrite(M_12, LOW);

  pinMode(S_1_light_main_1, OUTPUT);
  pinMode(S_2_light_main_2, OUTPUT);
  pinMode(S_3_projector, OUTPUT);
  pinMode(S_4_red_light, OUTPUT);
  pinMode(S_5_light, OUTPUT);
  pinMode(S_6_light, OUTPUT);
  pinMode(S_7_light, OUTPUT);
  pinMode(S_8_light, OUTPUT);
  digitalWrite(S_1_light_main_1, HIGH);
  digitalWrite(S_2_light_main_2, HIGH);
  digitalWrite(S_3_projector, LOW);
  digitalWrite(S_4_red_light, LOW);
  digitalWrite(S_5_light, HIGH); //LOW-level trigger
  digitalWrite(S_6_light, HIGH); //LOW-level trigger
  digitalWrite(S_7_light, HIGH); //LOW-level trigger
  digitalWrite(S_8_light, HIGH); //LOW-level trigger

  pinMode(U_1_bomb_start, OUTPUT);
  pinMode(U_2_bomb_reset, OUTPUT);
  pinMode(U_3, OUTPUT);
  pinMode(U_4, OUTPUT);
  digitalWrite(U_1_bomb_start, HIGH);
  digitalWrite(U_2_bomb_reset, HIGH);
  digitalWrite(U_3, LOW);
  digitalWrite(U_4, LOW);

  //reset_state(); //reset timer and all locks
  Serial.println("MCS_New_Spy v1.0 has been started");
}

void loop()
{
  //check_all_dev();
  //read_all_but();

  if(digitalRead(B_1_start_but) == LOW)
  {
    delay(100);
    digitalWrite(U_1_bomb_start, HIGH); //start a bomb timer
    mp3_set_serial(Serial1);
    mp3_set_volume(20);
    mp3_play(1);
  }

  if(analogRead(B_2_eyes_sens1 <= 300) && analogRead(B_3_eyes_sens2 <= 300))
  {
    delay(100);
    digitalWrite(M_1_LED_table_map, HIGH);
  }

  if(digitalRead(B_4_GK_wars) == LOW)
  {
    delay(100);
    digitalWrite(M_1_LED_table_map, LOW);
    digitalWrite(M_2_EML_tumbler1, LOW);
    digitalWrite(M_3_EML_tumbler2, LOW);
  }

  //tumblers are done
  if(digitalRead(B_5_tumblers1) == LOW)
  {
    delay(100);
    digitalWrite(M_4_EML_door1, LOW);
  }

  //tumblers are done
  if(digitalRead(B_6_tumblers2) == LOW)
  {
    delay(100);
    digitalWrite(M_5_EML_box_wires, LOW);
  }

  //all 4 panel wires at a right places
  if(digitalRead(B_7_panel_wires1) == LOW)
  {
    delay(100);
    digitalWrite(S_1_light_main_1, LOW);
    digitalWrite(S_3_projector, HIGH);
  }

  //red button is pressed
  if(digitalRead(B_11_red_but) == LOW)
  {
    delay(100);
    digitalWrite(S_1_light_main_1, LOW);
    digitalWrite(S_2_light_main_2, LOW);
    digitalWrite(S_3_projector, LOW);
    digitalWrite(M_6_LED_UV_dance_humans, HIGH);
  }

  //red button is released
  else if(digitalRead(B_11_red_but) == HIGH) 
  {
    delay(100);
    digitalWrite(S_1_light_main_1, HIGH);
    digitalWrite(S_2_light_main_2, HIGH);
    digitalWrite(M_6_LED_UV_dance_humans, LOW);
  }

  //laser aims is shouted by lasers
  if(analogRead(B_8_laser_aims1) <= 500 && analogRead(B_12_laser_aims2) <= 500)
  {
    delay(100);
    digitalWrite(M_7_EML_nude_wires, LOW);
    digitalWrite(S_4_red_light, HIGH);
  }

  //circle is shorted by human body
  if(analogRead(B_13_nude_wires) <= 300)
  {
    delay(100);
    digitalWrite(M_8_EML_bomb_box, LOW);
  }

  //right wire is cutted
  if(digitalRead(B_14_bomb_ok)) //signal from bomb, that's all right
  {
    digitalWrite(S_4_red_light, LOW);
    digitalWrite(M_9_EML_door2, LOW);
  }

  //wrong wire is cutted
  if(digitalRead(B_16_bomb_not_ok) == LOW)
  {
    delay(100);
    mp3_set_serial(Serial1);
    mp3_set_volume(25);
    mp3_play(3); //loose music

    digitalWrite(M_10_EML_exit_door1, LOW); //open exit door1
    digitalWrite(M_11_EML_exit_door2, LOW); //open exit door2
  }

  //exit button is pressed
  if(digitalRead(B_9_exit_button) == LOW)
  {
    delay(100);
    mp3_set_serial(Serial1);
    mp3_set_volume(25);
    mp3_play(2); //win music

    digitalWrite(M_10_EML_exit_door1, LOW); //open exit door1
    digitalWrite(M_11_EML_exit_door2, LOW); //open exit door2
  }

  if(digitalRead(B_15_reset_but) == LOW)
  {
    delay(100);
    mp3_set_serial(Serial1);
    mp3_stop(); // stop any track playing after controller reset 
    mp3_set_volume(0); //just for sure, that music isn't plays
    reset_state();
  }
}

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
  delay(500);
  digitalWrite(S_1_light_main_1, HIGH);
  delay(500);
  digitalWrite(S_2_light_main_2, HIGH);
  delay(500);
  digitalWrite(S_3_projector, HIGH);
  delay(500);
  digitalWrite(S_4_red_light, HIGH);
  delay(500);
  digitalWrite(S_5_light, LOW);
  delay(500);
  digitalWrite(S_6_light, LOW);
  delay(500);
  digitalWrite(S_7_light, LOW);
  delay(500);
  digitalWrite(S_8_light, LOW);
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

  digitalWrite(S_1_light_main_1, LOW);
  digitalWrite(S_2_light_main_2, LOW);
  digitalWrite(S_3_projector, LOW);
  digitalWrite(S_4_red_light, LOW);
  digitalWrite(S_5_light, HIGH);
  digitalWrite(S_6_light, HIGH);
  digitalWrite(S_7_light, HIGH);
  digitalWrite(S_8_light, HIGH);
}

void reset_state()
{
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
  delay(500);
  digitalWrite(M_1_LED_table_map, LOW);
  digitalWrite(M_2_EML_tumbler1, HIGH);
  digitalWrite(M_3_EML_tumbler2, HIGH);
  digitalWrite(M_4_EML_door1, HIGH);
  digitalWrite(M_5_EML_box_wires, HIGH);
  digitalWrite(M_6_LED_UV_dance_humans, LOW);
  digitalWrite(M_7_EML_nude_wires, HIGH);
  digitalWrite(M_8_EML_bomb_box, HIGH);
  digitalWrite(M_9_EML_door2, HIGH);
  digitalWrite(M_10_EML_exit_door1, HIGH);
  digitalWrite(M_11_EML_exit_door2, HIGH);
  digitalWrite(M_12, HIGH);

  digitalWrite(U_2_bomb_reset, LOW);
  delay(500);
  digitalWrite(U_2_bomb_reset, HIGH);
}
/*
Питання по алгоритму Нового Шпіона:

Оператор заводить обидві команди в темні кімнати, після чого нажимає біля дверей кнопку "старт" - включається світло, грає музика, запускається бомба.
Такий алгоритм старту - ок?

Над бомбою - 4 лампочки кольорових. Підводжу живлення тільки до одної із них (220В), щоб в впевний момент загорілася, показуючи колір проводу, 
який треба перерізати - ок? чи треба опціональну можливість керувати всіма 4ма?.

В самій бомбі теж є лампочка яка мигає коли неправильно зробив завдання з бомбою - можна її вбудувати в коробку з бомбою

Адміністраторський пульт управління (для пропуску завдань якщо гравці не можуть щось зробити) можемо зробити провідним - коробочка на столі
Або - аналогічно, як в піраті безпровідним. Провідний не можна взяти з собою наверх, але він надійніший і не загубиться. Беспровідний може не добивати з адмінки
*/