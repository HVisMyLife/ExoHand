#include <Servo.h>

int RPWM = PB9;
int LPWM = PB8;
int L_EN = PB3;
int R_EN = PB4;
int servo = PA8;

Servo hand;
float pos, armPosition;
float speedMove, speedMove2;
float pwmSig;

int pushButton = PB14;
int pot = PB1;
int potPos = PB0;
int keyPad = PA7;

int stage = 0;

int key = 0;
char keyCharBuffer;
char keyChar;
bool keyPadUsage = false, keyStartInput = false, keyStopInput = false;


bool dir, enMove = false, input;

long long unsigned int startTime, closeTime, startTime2, openTime, moveTime, keyStopTime, keyStartTime, moveDeltaTime;

void WriteMove();
void ReadPush();
void ReadKey();

void setup() {
  // put your setup code here, to run once:
  pinMode(PB9, OUTPUT);
  pinMode(PB8, OUTPUT);
  pinMode(PB3, OUTPUT);
  pinMode(PB4, OUTPUT);
  pinMode(PC13, OUTPUT);
  pinMode(pushButton, INPUT_PULLUP);
  pinMode(pot, INPUT);
  pinMode(potPos, INPUT);
  pinMode(keyPad, INPUT);

  hand.attach(servo);

  dir = true;

  digitalWrite(R_EN,HIGH);
  digitalWrite(L_EN,HIGH);
  
  delay(250);
  Serial.begin(115200);
  }

void loop() {
  WriteMove();
  ReadKey();
  ReadPush();
  
  delay(1);
}

void WriteMove()
{
  armPosition = analogRead(potPos);
  if (armPosition > 3000) armPosition = 3000;
  if (armPosition < 1500) armPosition = 1500;
  armPosition = map(armPosition, 1500, 3000, 1024, 0);
  Serial.println(armPosition);
  //Serial.print(" || ");
  //Serial.println(pos);
  speedMove = map(analogRead(pot), 0, 4096, 0, 1000);

  pwmSig = armPosition - pos;
  if (pwmSig < 0) pwmSig = -pwmSig;
  if (pwmSig > 255) pwmSig = 255;
  speedMove2 = map(speedMove, 0, 1000, 400, 75);
  speedMove2 /= 100;
    
  if(enMove && armPosition > 0 && dir) 
  {
    if( micros() - moveDeltaTime > speedMove) { pos -= speedMove2; moveDeltaTime = micros();}
    if (pos < 0) pos = 0;
    hand.write(map(pos, 0, 1024, 0, 255));
    if(armPosition > pos + 25) {analogWrite(LPWM, pwmSig); analogWrite(RPWM, 0);}
    else if(armPosition < pos - 25) {analogWrite(LPWM, 255); analogWrite(RPWM, 255);}
    else {analogWrite(LPWM, 0);analogWrite(RPWM, 0);}
  }
  else if (enMove && armPosition < 1024 && !dir) 
  {
    if( micros() - moveDeltaTime > speedMove ) { pos += speedMove2; moveDeltaTime = micros();}
    if (pos > 1024) pos = 1024;
    hand.write(map(pos, 0, 1024, 0, 255));
    if(armPosition < pos - 25) {analogWrite(LPWM, 0); analogWrite(RPWM, pwmSig);}
    else if(armPosition > pos + 25) {analogWrite(LPWM, 255); analogWrite(RPWM, 255);}
    else {analogWrite(LPWM, 0);analogWrite(RPWM, 0);}
  }
  else
  {
    analogWrite(LPWM, 0);
    analogWrite(RPWM, 0);
  }

  //if ( armPosition <= 50 && dir && enMove || armPosition >= 1998 && !dir && enMove ) {stage = 0; enMove = false;}
}

void ReadPush()
{
  input = digitalRead(pushButton);
  digitalWrite(PC13, input);
   
  switch (stage){
    case 0:
      if ( !input ) {startTime = millis(); stage = 1;}
    break;
    case 1:
      if (input && millis() - startTime < 100) stage = 0;
      else if (!input && millis() - startTime >= 100) stage = 2;
    break;
    case 2:
      if (input) {startTime2 = millis(); stage = 3; }
    break;
    case 3:
      if (!input && millis() - startTime2 < 50) stage = 2;
      else if (!input && millis() - startTime2 >= 50) stage = 4;                              //double
      else if (input && millis() - startTime2 >= 750) { stage = 5; enMove = true; dir = false; moveTime = millis(); moveDeltaTime = micros();}   //single
    break;
    case 4:
      if (input) { stage = 5; enMove = true; dir = true; moveTime = millis(); moveDeltaTime = micros();}
    break;
    case 5:
      if (!input && millis() - moveTime > 25) {stage = 6; enMove = false;}
    break;
    case 6:
      if (input) stage = 0;
    break;
    default:
      
    break;
  }
}

void ReadKey()
{
  key = ((key * 19) + analogRead(keyPad)) / 20;
  //key = analogRead(keyPad);
  
  if (key < 100) keyCharBuffer = 'X';
  else if (key < 220) keyCharBuffer = '*';
  else if (key < 235 ) keyCharBuffer = '0';
  else if (key < 250 ) keyCharBuffer = '#';
  else if (key < 270 ) keyCharBuffer = 'D';
  else if (key < 305 ) keyCharBuffer = '7';
  else if (key < 330 ) keyCharBuffer = '8';
  else if (key < 360 ) keyCharBuffer = '9';
  else if (key < 400 ) keyCharBuffer = 'C';
  else if (key < 490 ) keyCharBuffer = '4';
  else if (key < 550 ) keyCharBuffer = '5';
  else if (key < 650 ) keyCharBuffer = '6';
  else if (key < 800 ) keyCharBuffer = 'B';
  else if (key < 1100 ) keyCharBuffer = '1';
  else if (key < 1700 ) keyCharBuffer = '2';
  else if (key < 3000 ) keyCharBuffer = '3';
  else  keyCharBuffer = 'A';

  if ( keyCharBuffer != 'X' && !keyStartInput && !keyPadUsage) { keyStartTime = millis(); keyStartInput = true;}
  else if ( keyStartInput && millis() - keyStartTime > 100 && !keyPadUsage ) { keyPadUsage = true; keyStartInput = false; keyChar = keyCharBuffer;}
  else if ( keyStartInput && keyCharBuffer == 'X' ) keyStartInput = false;
  else if ( keyCharBuffer == 'X' && keyPadUsage && !keyStopInput) { keyStopTime = millis(); keyStopInput = true; }
  else if ( keyStopInput && millis() - keyStopTime > 100 && keyPadUsage ) { keyPadUsage = false; keyStopInput = false; }
  else if ( keyStopInput && keyCharBuffer != 'X' ) keyStopInput = false;
  
  if ( !keyPadUsage ) keyChar = 'X';
  
  //Serial.println(keyChar);
}
