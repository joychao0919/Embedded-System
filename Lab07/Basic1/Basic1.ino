#include <stdio.h>
#include <stdlib.h>
/*
  On

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/On


  Each time the input pin goes from LOW to HIGH (e.g. because of a push-button
  press), the output pin is toggled from LOW to HIGH or HIGH to LOW. There's a
  minimum delay between toggles to debounce the circuit (i.e. to ignore noise).

  The circuit:
  - LED attached from pin 13 to ground
  - pushbutton attached from pin 2 to +5V
  - 10 kilohm resistor attached from pin 2 to ground

  - Note: On most Arduino boards, there is already an LED on the board connected
    to pin 13, so you don't need any extra components for this example.

  created 21 Nov 2006
  by David A. Mellis
  modified 30 Aug 2011
  by Limor Fried
  modified 28 Dec 2012
  by Mike Walters
  modified 30 Aug 2016
  by Arturo Guadalupi

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Debounce
*/
#define initial 0
#define record 1
#define replay 2
#define pressing 3
#define released 4


// constants won't change. They're used here to set pin numbers:
const int btnRed = 7; 
const int btnBlue = 6; 
const int btnGreen = 5;  
const int ledRed = 4; 
const int ledBlue = 3;  
const int ledGreen = 2; 

// Variables will change:
int stateLedRed = HIGH;     
int stateLedBlue = HIGH;
int stateLedGreen = HIGH;
int stateBtnRed;  
int stateBtnBlue;  
int stateBtnGreen;
int stateLastBtnRed = LOW;
int stateLastBtnBlue = LOW;
int stateLastBtnGreen = LOW; 

int redOn[20];
int redOff[20];
int blueOn[20];
int blueOff[20];
int greenOn[20];
int greenOff[20];
int time0 = 0;
int time1 = 0;
int state = 0;
int count = 0;
int event = 4;
int redPress = 0;
int lastRedPress = 0;
int redOnIndex = 0;
int redOffindex = 0;
int blueOnIndex = 0;
int blueOffindex = 0;
int greenOnIndex = 0;
int greenOffindex = 0;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTimeRed = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTimeBlue = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTimeGreen = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
int recordTime;

int recording(int Btn, int LED, int*On, int index, int*OFF, int readbtn, int realTime);
int replaying(int LED, int*On, int*OFF, int index, int realTime);

void setup() {
  Serial.begin(9600);
  pinMode(btnRed, INPUT);
  pinMode(btnBlue, INPUT);
  pinMode(btnGreen, INPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  pinMode(ledGreen, OUTPUT);

  // set initial LED state
  digitalWrite(ledRed, stateLedRed);
  digitalWrite(ledBlue, stateLedBlue);
  digitalWrite(ledGreen, stateLedGreen);
  
  // initialize timer1 
  noInterrupts(); // atomic access to timer reg.
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 1562;  // give 0.1 sec at 16 MHz/1024 *0.1 -1.
  interrupts(); // enable all interrupts

  state = initial;
  recordTime = 0;    
}

int pressTime;
int releaseTime;
int pressingtime;
int pressingtime2;
int realTime = 0;
int endTime;

void loop() {
    int readR = digitalRead(btnRed);
    int readG = digitalRead(btnGreen);
    int readB = digitalRead(btnBlue);
  
  Serial.println(readR);

    if (TIFR1 & (1 << OCF1A)) { // wait for time up
        realTime++;
    switch(state){
      case initial:
        Serial.println("initial");
      if(readR==1 && stateLastBtnRed==0){ 
        count++;
        if(count==1){
          time0 = millis(); //record first press time.
          }
          if(count>=2){
          time1 = millis();//start record time;
          if(time1 - time0 > 1000){
          time0 = time1 = 0;//reset time.
          count = 0; 
          }
          else{
          state = record;
          digitalWrite(ledRed, LOW);
          digitalWrite(ledGreen, LOW);
          digitalWrite(ledBlue, LOW);
          Serial.println("ALL OFF");
          realTime = 0; //start count time.
          count = 0;
          break;
          }
          }
      }
            lastRedPress = redPress; 

        if (readR != stateLastBtnRed) {
        lastDebounceTimeRed = millis();
        }
      if (readG != stateLastBtnGreen) {
        lastDebounceTimeGreen = millis();
        }
      if (readB != stateLastBtnBlue) {
        lastDebounceTimeBlue = millis();
        }

      if ((millis() - lastDebounceTimeRed) > debounceDelay) {
        if (readR != stateBtnRed) {
          stateBtnRed = readR;

          if (stateBtnRed == HIGH) {
            stateLedRed = !stateLedRed;
            lastRedPress = redPress;
            redPress++;
          }
        }
      }

        if ((millis() - lastDebounceTimeGreen) > debounceDelay) {
          if (readG != stateBtnGreen) {
              stateBtnGreen = readG;
              if (stateBtnGreen == HIGH) {
                stateLedGreen = !stateLedGreen;
              }
          }
        }
      
        if ((millis() - lastDebounceTimeBlue) > debounceDelay) {
          if (readB != stateBtnBlue) {
              stateBtnBlue = readB;
              if (stateBtnBlue == HIGH) {
                stateLedBlue = !stateLedBlue;
              }
          }
        }

        stateLastBtnRed = readR;
        stateLastBtnGreen = readG;
        stateLastBtnBlue = readB;
            break;
  
            case record:
        redOnIndex = recording(btnRed, ledRed, redOn, redOnIndex, redOff, readR, realTime, stateLastBtnRed);
        blueOnIndex = recording(btnBlue, ledBlue, blueOn,blueOnIndex, blueOff,readB, realTime, stateLastBtnBlue);
        greenOnIndex = recording(btnGreen, ledGreen, greenOn,greenOnIndex, greenOff,readG, realTime, stateLastBtnGreen);
        if(readR==HIGH)stateLastBtnRed = 3;//pressing.
        else stateLastBtnRed = 4;
        if(readB==HIGH)stateLastBtnBlue = 3;//pressing.
        else stateLastBtnBlue = 4;
        if(readG==HIGH)stateLastBtnGreen = 3;//pressing.
        else stateLastBtnGreen = 4;                       
        if(readR == HIGH && readB == HIGH && readG == HIGH){
            digitalWrite(ledRed,LOW);
            digitalWrite(ledBlue,LOW);
            digitalWrite(ledGreen,LOW);
            redOnIndex = 0;
            blueOnIndex = 0;
            greenOnIndex = 0;
            endTime = realTime;//record end time.
            state = replay;
            realTime = 0;//reset.
          }
        break;
      
      case replay:
          redOnIndex = replaying(ledRed, redOn, redOff, redOnIndex, realTime);
          greenOnIndex = replaying(ledGreen, greenOn, greenOff, greenOnIndex, realTime);
          blueOnIndex = replaying(ledBlue, blueOn, blueOff, blueOnIndex, realTime);
            break;
        }
        TIFR1 = (1<<OCF1A); 
  } 
}

int recording(int Btn, int LED, int*On, int index, int*OFF, int readbtn, int realTime, int event){ //event == lastBTNstate
  if(event == pressing){
    if(readbtn==HIGH){
        digitalWrite(LED, HIGH);
    }
    else if(readbtn == LOW){ //press->released: keep this point.
        digitalWrite(LED, LOW);
        event = released;
        Serial.println(index);
        OFF[index++] = realTime;
      }
  }
    else{ //released
        if(readbtn == HIGH){ //release to press:remember this point.
            digitalWrite(LED, HIGH);
            Serial.print("We record ON index:");
            Serial.println(index);
            On[index] = realTime;
            event = pressing;  
        }
        else if(readbtn == LOW){//still released. do nothing.
            digitalWrite(LED, LOW); 
        }
    }
    return index;    
}

int replaying(int LED, int*On, int*OFF, int index, int realTime){
    if(realTime<=endTime){
        if(realTime == On[index]){
            digitalWrite(LED,HIGH);
            Serial.println(index);            
            On[index] = 0; //reset On array.
        }
        else if(realTime == OFF[index]){
            digitalWrite(LED,LOW);
            Serial.println(index);
            OFF[index++] = 0; //reset On array.
        }
    }
    else{
        index = 0;
        realTime = 0;
        state = initial;
        event = released;
    }
    return index;
}
