#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#define KEY_ROWS 4
#define KEY_COLS 4

LiquidCrystal_I2C lcd(0x3F,16,2);

void LoginFun();
void SetupFun();
void DisplayTask();



char storedPass[8] = {'1', '2', '3', '4', '\0', '\0', '\0', '\0'};
char inputStr[9];
int passLength = 4;
int firstInput = 0;
int correct = 0;
int wrong = 0;
int count = 0;
int finish = 0;
int i, j = 0, k;
int printMode = 0; 
char keymap[KEY_ROWS][KEY_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
// Column pin 1~4
byte colPins[KEY_COLS] = {9, 8, 7, 6};
// Column pin 1~4
byte rowPins[KEY_ROWS] = {13, 12, 11, 10};
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, KEY_ROWS, KEY_COLS);
char key; 

void setup(){
  Serial.begin(9600);
  lcd.init();    // initialize LCD
  lcd.backlight();    // open LCD backlight
  lcd.setCursor(0, 0);  // setting cursor   
  lcd.print("Enter passcode:");  
  delay(1000); 
  //lcd.clear(); // clear all 
  lcd.setCursor(0, 1);  
  //lcd.print("Type to display");
}

void loop(){
  LoginFun();
  delay(100);
}

void LoginFun(){
  key = myKeypad.getKey(); 
  Serial.println(key);
  if (key == '#') {
    Serial.println(key);
    Serial.print("count=");
    Serial.println(count);
    Serial.print("wrong=");
    Serial.println(wrong);
    if(firstInput == 0){ // enter setup
      for(k=0; k<9; k++){
        inputStr[k] = NULL;
      }
      count = 0;
      SetupFun();
    }
    if(firstInput == 1){
      if (count == passLength && wrong != 1) {
        //Serial.println("hi");
        printMode = 1;
        DisplayTask();
      }
      else if (wrong == 1 || count != passLength) {
        //Serial.println(count);
        printMode = 2;
        DisplayTask();
        
      }
      firstInput = 0;
      count = 0;
      i = 0;
      for(int j=0; j<9; j++){
        inputStr[j] = NULL;
      }
      wrong = 0;
    }
  }
  
  else if (key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9' ) {
    if (i < 8) {
      if(firstInput == 0){
        firstInput = 1;
      }
      Serial.println(key);
      inputStr[i++] = key;
      lcd.setCursor(0, 1); 
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

void SetupFun() {
  j = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set passcode:");
  key = myKeypad.getKey();
  while(1){
    key = myKeypad.getKey();
    if (key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9' ) {
      if (j < 8) {
        Serial.println(key);
        inputStr[j++] = key;
        lcd.setCursor(0, 1); 
        lcd.print(inputStr);
      }
    }
    else if (key == '#') {
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
        DisplayTask();
        LoginFun();
        break;
      }
    }
  }
}

void DisplayTask() {
  if (printMode == 1) {
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("Correct!");
    lcd.setCursor(0, 1); 
    lcd.print(inputStr);
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);  // setting cursor   
    lcd.print("Enter passcode:");  
  }
  else if (printMode == 2) {
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("Wrong!");
    lcd.setCursor(0, 1); 
    lcd.print(inputStr);
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);  // setting cursor   
    lcd.print("Enter passcode:");  
  }
  else if (printMode == 3) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter passcode:");
    lcd.setCursor(0, 1);
  }
  return;
}
