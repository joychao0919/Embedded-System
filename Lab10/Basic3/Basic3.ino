#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#define KEY_ROWS 4
#define KEY_COLS 4

LiquidCrystal_I2C lcd(0x3F,16,2);

void LoginTask(void *pvParameters);
void SetupTask(void *pvParameters);
void DisplayTask(void *pvParameters);

TaskHandle_t handleLogin;
TaskHandle_t handleSetup;
TaskHandle_t handleDisplay;

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
int globalMode = 0;
int printFinish = 0;
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

void setup() {
  Serial.begin(9600);
  lcd.init();    // initialize LCD
  lcd.backlight();    // open LCD backlight
  lcd.setCursor(0, 0);  // setting cursor   
  lcd.print("Enter passcode:");  
  lcd.setCursor(0, 1);

   xTaskCreate(LoginTask, (const portCHAR *) "task_login", 128, NULL, 1, &handleLogin);
   xTaskCreate(DisplayTask, (const portCHAR *) "task_display", 128, NULL, 1, &handleDisplay);
   xTaskCreate(SetupTask, (const portCHAR *) "task_setup", 128, NULL, 1, &handleSetup);
   
   //vTaskStartScheduler();
}

void loop(){}

void LoginTask(void *pvParameters) {
  (void) pvParameters;
  


  for(;;){
    if(globalMode == 0){
      key = myKeypad.getKey(); 
    }
    
    
    
    if (key == '#' && globalMode == 0) {
      Serial.println("login");
      if(firstInput == 0){ // enter setup
        for(k=0; k<9; k++){
          inputStr[k] = NULL;
        }
        count = 0;
        j = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Set passcode:");
        lcd.setCursor(0, 1);
        ////�hsetup////
        globalMode = 1;
        //vTaskPrioritySet(handleSetup, 3);
        //vTaskPrioritySet(handleDisplay, 2);
        //vTaskPrioritySet(handleLogin, 1);   
        //SetupFun();
      }
      if(firstInput == 1){
        if (count == passLength && wrong != 1) {
          printMode = 1;
        }
        else if (wrong == 1 || count != passLength) {
          printMode = 2;
        }
        firstInput = 0;
        count = 0;
        i = 0;
        wrong = 0;
      }
    }
    
    else if ((key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || 
    key == '6' || key == '7' || key == '8' || key == '9') && globalMode == 0 ) {
      if (i < 8) {
        Serial.println("login");
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
}

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
      
      /*
      vTaskPrioritySet(handleLogin, 3);       
      vTaskPrioritySet(handleSetup, 1);
      vTaskPrioritySet(handleDisplay, 2);  
      */
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
      /*
      vTaskPrioritySet(handleLogin, 3);
      vTaskPrioritySet(handleSetup, 1);
      vTaskPrioritySet(handleDisplay, 2);
      //vTaskResume(handleLogin);
      */
    }
    else if (printMode == 3) { // 
      Serial.println("printmode3");
      lcd.clear();
      lcd.setCursor(0, 0);  // setting cursor   
      lcd.print("Enter passcode:"); 
      //lcd.setCursor(0, 1);
      
      globalMode = 0;
      printMode = 0;
      /*
      vTaskPrioritySet(handleLogin, 3);
      vTaskPrioritySet(handleSetup, 1);
      vTaskPrioritySet(handleDisplay, 2);
      //vTaskResume(handleLogin);
      */
    }
  }
}

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
          
          /*
          vTaskPrioritySet(handleLogin, 1);
          vTaskPrioritySet(handleDisplay, 3);
          vTaskPrioritySet(handleSetup, 2);
          Serial.println("setup");
          */
          //break;
        }
      }
  }
}
