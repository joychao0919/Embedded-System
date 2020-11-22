#include <Arduino_FreeRTOS.h>
#include "AnalogMatrixKeypad.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//#include <Keypad.h>
//#define KEY_ROWS 4
//#define KEY_COLS 4
const int LED_R = 6;
const int LED_G = 5;
const int LED_B = 3;
int buzzer = 9;


LiquidCrystal_I2C lcd(0x3f,16,2);
AnalogMatrixKeypad keypad(A0);


void taskKeypad(void *pvParameters);
void taskLEDTone(void *pvParameters);
//void DisplayTask(void *pvParameters);

TaskHandle_t handleKeypad;
TaskHandle_t handleLEDTone;

char storedPass[9] = {'8', '0', '8', '0', '\0', '\0', '\0', '\0', '\0'};
char inputStr[10];
int passLength = 4;
int firstInput = 0;
int correct = 0;
int wrong = 0;
int count = 0;
int finish = 0;
int i, j = 0, k;
int printMode = 0; 
int globalMode = 0;
int printFinish = 0;
int ledmode = 0;
//char keymap[KEY_ROWS][KEY_COLS] = {
//  {'1', '2', '3', 'A'},
//  {'4', '5', '6', 'B'},
//  {'7', '8', '9', 'C'},
//  {'*', '0', '#', 'D'}
//};
// Column pin 1~4
//byte colPins[KEY_COLS] = {9, 8, 7, 6};
// Column pin 1~4
//byte rowPins[KEY_ROWS] = {13, 12, 11, 10};
//Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, KEY_ROWS, KEY_COLS);
char key; 

void setup() {
  Serial.begin(9600);
  Serial.print("9600");
  lcd.init();    // initialize LCD
  lcd.backlight();    // open LCD backlight
  lcd.setCursor(0, 0);  // setting cursor   
  lcd.print("Input:");  
  lcd.setCursor(6, 0);
  pinMode(buzzer, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

   xTaskCreate(taskKeypad, (const portCHAR *) "task_keypad", 128, NULL, 1, &handleKeypad);
   xTaskCreate(taskLEDTone, (const portCHAR *) "task_ledtone", 128, NULL, 1, &handleLEDTone);
   Serial.print("2600");
   //xTaskCreate(SetupTask, (const portCHAR *) "task_setup", 128, NULL, 1, &handleSetup);
   
   //vTaskStartScheduler();
}

void loop(){
}

void taskKeypad(void *pvParameters) {
  (void) pvParameters;
  for(;;){
    if(globalMode == 0){
      key = keypad.readKey(); 
    }
    
    if (key == '#' && globalMode == 0) {
      globalMode = 1;
      
      Serial.println("login");
      if (count == passLength && wrong != 1) {
       ledmode = 1;
       lcd.clear();
       lcd.setCursor(0, 0);
       lcd.print("Correct");
       delay(1000);
       lcd.clear();
       lcd.setCursor(0, 0);  // setting cursor   
       lcd.print("Input:");  
       for(k=0; k<10; k++){
         inputStr[k] = NULL;
       }
       globalMode = 0;
       i = 0;
      }
      else if (wrong == 1 || count != passLength) {
       ledmode = 2;
       lcd.clear();
       lcd.setCursor(0, 0);
       lcd.print("Wrong!");
       lcd.setCursor(6, 0);
       lcd.print(inputStr);
       delay(1000);
       lcd.clear();
       lcd.setCursor(0, 0);  // setting cursor   
       lcd.print("Input:");  
       for(k=0; k<10; k++){
         inputStr[k] = NULL;
       }
       globalMode = 0;
      }
      count = 0;
      wrong = 0;
      i = 0;
      ledmode = 0;
    }
    
    else if ((key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || 
    key == '6' || key == '7' || key == '8' || key == '9') && globalMode == 0 ) {
      if (i < 9) {
        Serial.println(key);
        inputStr[i++] = key;
        lcd.clear();
        lcd.setCursor(0, 0); 
        lcd.print("Input:");
        lcd.setCursor(6, 0); 
        lcd.print(inputStr);
        if(key == storedPass[i-1]){
          count++;
        }
        else{
          wrong = 1;
        }
      }
    }
  }
}
void taskLEDTone(void *pvParameters) {
  (void) pvParameters;
  while(1){
    if(ledmode==1) {
      analogWrite(LED_G, 255);
      tone(buzzer, 500);
    }
    else if(ledmode==2) {
      analogWrite(LED_R, 255);
      tone(buzzer, 1000);
    }
    else {
      noTone(buzzer);
      analogWrite(LED_R, 0);
      analogWrite(LED_G, 0);
      analogWrite(LED_B, 0);
    }
  }
  

  
}
/*
void DisplayTask(void *pvParameters) {
  (void) pvParameters;
  
  for(;;){
    //Serial.println("display");
    if (printMode == 1) { // correct
      Serial.println("printmode1");
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("Correct!");
      lcd.setCursor(0, 1); 
      lcd.print(inputStr);
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);  // setting cursor   
      lcd.print("Enter passcode:"); 
      ////�^login////
      for(k=0; k<9; k++){
        inputStr[k] = NULL;
      }
      i = 0;
      printMode = 0;
      
      /*      vTaskPrioritySet(handleLogin, 3);       
      vTaskPrioritySet(handleSetup, 1);
      vTaskPrioritySet(handleDisplay, 2);  

    }
    else if (printMode == 2) { // wrong
      Serial.println("printmode2");
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("Wrong!");
      lcd.setCursor(0, 1); 
      lcd.print(inputStr);
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);  // setting cursor   
      lcd.print("Enter passcode:");  
      ////�^login////
      
      for(k=0; k<9; k++){
        inputStr[k] = NULL;
      }
      i = 0;
      printMode = 0; 

      vTaskPrioritySet(handleLogin, 3);
      vTaskPrioritySet(handleSetup, 1);
      vTaskPrioritySet(handleDisplay, 2);
      //vTaskResume(handleLogin);

    }
    else if (printMode == 3) { // 
      Serial.println("printmode3");
      lcd.clear();
      lcd.setCursor(0, 0);  // setting cursor   
      lcd.print("Enter passcode:"); 
      //lcd.setCursor(0, 1);
      
      globalMode = 0;
      printMode = 0;

      vTaskPrioritySet(handleLogin, 3);
      vTaskPrioritySet(handleSetup, 1);
      vTaskPrioritySet(handleDisplay, 2);
      //vTaskResume(handleLogin);

    }
  }
}
*/
/*
void SetupTask(void *pvParameters) {
  (void) pvParameters;

  for(;;){
    if(globalMode == 1){
      key = myKeypad.getKey(); 
    }
      if ((key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || 
      key == '6' || key == '7' || key == '8' || key == '9') && globalMode == 1 ) {
        Serial.println("setup");
        if (j < 8) {
          
          Serial.println(key);
          inputStr[j++] = key;
          lcd.setCursor(0, 1); 
          lcd.print(inputStr);
        }
      }
      else if (key == '#' && globalMode == 1) {
        Serial.println("setup");
        Serial.println(key);
        if (j > 3 && j <= 8) {
          Serial.print("j=");
          Serial.println(j);
          for(k=0; k<j; k++){
            storedPass[k] = inputStr[k];
          }
          for(k=0; k<9; k++){
            inputStr[k] = NULL;
          }
          passLength = j;
          printMode = 3;
          
          
          vTaskPrioritySet(handleLogin, 1);
          vTaskPrioritySet(handleDisplay, 3);
          vTaskPrioritySet(handleSetup, 2);
          Serial.println("setup");
          
          //break;
        }
      }
  }
}
*/
