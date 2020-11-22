#include <stdio.h>
#include <stdlib.h>
//補debounce
/*
  Blink

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

  http://www.arduino.cc/en/Tutorial/Blink
*/
/*
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(6, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(6, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(6, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
*/
/*
  Debounce

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
#define question 0
#define answer 1
#define ON 0
#define OFF 1

// constants won't change. They're used here to set pin numbers:
const int btnRed = 7;    // the number of the pushbutton pin
const int btnBlue = 6;    // the number of the pushbutton pin
const int btnGreen = 5;    // the number of the pushbutton pin
const int ledRed = 4;      // the number of the LED pin
const int ledBlue = 3;      // the number of the LED pin
const int ledGreen = 2;      // the number of the LED pin

// Variables will change:
int stateLedRed = HIGH;         // the current state of the output pin
int stateLedBlue = HIGH;
int stateLedGreen = HIGH;
int stateBtnRed;             // the current reading from the input pin
int stateBtnBlue;             // the current reading from the input pin
int stateBtnGreen;             // the current reading from the input pin
int stateLastBtnRed;   // the previous reading from the input pin
int stateLastBtnBlue;   // the previous reading from the input pin
int stateLastBtnGreen;   // the previous reading from the input pin


int state = 0;
int event; //ON / OFF
int wrong = 0;
int count = 0; //count the time btn is pressed in a second.

int lastInput = 0;
int output = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; 

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
int record_time;
//
int Q[5];//question sequence.
int A[5];//answer sequence.
int index; //index for Q and A.
int randomNum; //randon number for led.

int debounce(int input){
  if(input != lastInput){
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    output = lastInput;
  }
  lastInput = input;
  return output;
}

void setup() {
  pinMode(btnRed, INPUT);
  pinMode(btnBlue, INPUT);
  pinMode(btnGreen, INPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  pinMode(ledGreen, OUTPUT);

  // set initial LED state
  digitalWrite(ledRed, LOW);
  digitalWrite(ledBlue, LOW);
  digitalWrite(ledGreen, LOW);
  
  // initialize timer1 
  noInterrupts(); // atomic access to timer reg.
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 15624;  // give 0.1 sec at 16 MHz/1024 *1 -1.
  interrupts(); // enable all interrupts

  state = question;
  index = 0;
  event = OFF;
  Serial.begin(9600);
}
int press_time;
int release_time;
int pressingtime;
int pressingtime2;
int realTime = 0;
int Endtime;
int btnPush = 0;
const int pushRed = 1;
const int pushGreen = 2;
const int pushBlue = 3;

//int index;//store the time when LED should blink.
//int off_index;
void loop() {
     // read the state of the switch into a local variable:
    int readR = digitalRead(btnRed);
    int readG = digitalRead(btnGreen);
    int readB = digitalRead(btnBlue);
  int readPush = 0;
  
  if(readR){
    readPush = pushRed;
  }
  else if(readG){
    readPush = pushGreen;
  }
  else if(readB){
    readPush = pushBlue;
  }
    
  btnPush = debounce(readPush);

    if (TIFR1 & (1 << OCF1A)) { // wait for time up
        realTime++;

    switch(state){
      case question:
      if(event == ON){
        digitalWrite(ledRed, LOW);
        digitalWrite(ledGreen, LOW);
        digitalWrite(ledBlue, LOW);   
        event = OFF;
      }
      else{  
        Serial.print(index);
          Serial.print(": ");
        if(index==5){
          Serial.println("answer mode");
          state = answer;
          realTime = 0;
          index = 0; 
          wrong = 0;
          break;   
        }
        randomNum = random(2,5);
        if(randomNum == 4){//blink Red LED and keep Q[index] = 2.
          digitalWrite(ledRed, HIGH);
          Serial.println("Red");
          Q[index] = 4;
          index++;
        }
        else if(randomNum == 3){
          digitalWrite(ledBlue, HIGH);
          Serial.println("Blue");        
          Q[index] = 3;
          index++;
        }
        else if(randomNum == 2){
          digitalWrite(ledGreen,HIGH);
          Serial.println("Green");
          Q[index] = 2;
          index++;
        }
        event = ON;
      }
      break;
    
      case answer:
      if(index==5){
        for(int i=0; i<=4; i++){
          if(Q[i] != A[i]){ 
            Serial.println("wrong");
            state = question;
            index = 0;
            wrong = 1;
            break;            
          }
        }
        if(wrong == 0){
          Serial.println("Right");
          index = 0;
          state = question;
        }
      }
      if(realTime >= 8){
        Serial.println("Time's Up");
        index = 0;
        state = question;
      }
      if(btnPush == pushRed){ //user press Red btn.
        digitalWrite(ledRed, HIGH);
        digitalWrite(ledBlue, LOW);
        digitalWrite(ledGreen, LOW);
        A[index] = 4;
        index++;
        Serial.println("Press: Red");
        realTime = 0; //reset time.
      }
      else if(btnPush == pushBlue){
        digitalWrite(ledRed, LOW);
        digitalWrite(ledBlue, HIGH);
        digitalWrite(ledGreen, LOW);
        A[index] = 3;
        index++;
        Serial.println("Press: Blue");
        realTime = 0; //reset time.
      }
      else if(btnPush == pushGreen){
        digitalWrite(ledRed, LOW);
        digitalWrite(ledBlue, LOW);
        digitalWrite(ledGreen, HIGH);
        A[index] = 2;
        index++;
        Serial.println("Press: Green");
        realTime = 0; //reset time.
      }
      else {
        digitalWrite(ledRed, LOW);
        digitalWrite(ledBlue, LOW);
        digitalWrite(ledGreen, LOW);
      }
      break;
    }
    TIFR1 = (1<<OCF1A); 
    }
}
