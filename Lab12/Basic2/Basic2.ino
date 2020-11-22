#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <string.h>
#include "AnalogMatrixKeypad.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
//#include <Keypad.h>
//#define KEY_ROWS 4
//#define KEY_COLS 4
#define RST_PIN         A1           // Configurable, see typical pin layout above
#define SS_PIN          10           // Configurable, see typical pin layout above
#define IRQ_PIN         2           // Configurable, depends on hardware
const int LED_R = 6;
const int LED_G = 5;
const int LED_B = 3;
int buzzer = 9;

SemaphoreHandle_t gatekeeper = 0;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key1;

volatile bool bNewInt = false;
byte regVal = 0x7F;
void activateRec(MFRC522 mfrc522);
void clearInt(MFRC522 mfrc522);

LiquidCrystal_I2C lcd(0x3f,16,2);
AnalogMatrixKeypad keypad(A0);


void taskKeypad(void *pvParameters);
void taskLEDTone(void *pvParameters);
void taskRFID(void *pvParameters);

TaskHandle_t handleKeypad;
TaskHandle_t handleLEDTone;
TaskHandle_t handleRFID;

char storedPass[9] = {'8', '0', '8', '0', '\0', '\0', '\0', '\0', '\0'};
char inputStr[10];
byte storedID[4] = {0x13, 0xBB, 0x8B, 0xAB};
int same = 5;
//char inpurID[9];
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
  while (!Serial);      // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)  
  SPI.begin();          // Init SPI bus

  mfrc522.PCD_Init(); // Init MFRC522 card

  Serial.print(F("Ver: 0x"));
  byte readReg = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.println(readReg, HEX);

  pinMode(IRQ_PIN, INPUT_PULLUP);

  regVal = 0xA0; //rx irq
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, regVal);

  bNewInt = false; //interrupt flag
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), readCard, FALLING);
  bNewInt = false;

  Serial.println(F("End setup"));

  gatekeeper = xSemaphoreCreateMutex();
  /* create tasks with priority 1 for both users */

  Serial.println("test");
  

  
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
   xTaskCreate(taskRFID, (const portCHAR *) "task_rfid", 128, NULL, 1, &handleRFID);
   
   vTaskStartScheduler();
}

void loop(){
}

void taskKeypad(void *pvParameters) {
  (void) pvParameters;
  for(;;){
    //if(xSemaphoreTake(gatekeeper, 100)){
      //Serial.println("User 1 got access");
      if(globalMode == 0){
        key = keypad.readKey(); 
      }

      if(isdigit(key)){
        xSemaphoreTake(gatekeeper, 10);
      }
    
      if (key == '#' && globalMode == 0) {
        globalMode = 1;
        
        //Serial.println("login");
        if (count == passLength && wrong != 1) {
         ledmode = 1;
         lcd.clear();
         lcd.setCursor(0, 0);
         lcd.print("Correct");
         vTaskDelay(1000/portTICK_PERIOD_MS);//delay 3 sec 
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
         vTaskDelay(1000/portTICK_PERIOD_MS);//delay 3 sec 
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
        xSemaphoreGive(gatekeeper);
      }
      
      else if ((key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || 
      key == '6' || key == '7' || key == '8' || key == '9') && globalMode == 0 ) {
        if (i < 9) {
          //Serial.println(key);
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
      //vTaskDelay(500);
      //xSemaphoreGive(gatekeeper);
    //}
    //else {
      //Serial.println("User 1 cannot access in 1000 ms");
    //}
    vTaskDelay(5);
  }
}
void taskLEDTone(void *pvParameters) {
  (void) pvParameters;
  while(1){
    if(ledmode==1 || same==0) {
      analogWrite(LED_G, 255);
      tone(buzzer, 500);
    }
    else if(ledmode==2 || same!=0 && same!=5) {
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

void taskRFID(void *pvParameters) {
  (void) pvParameters;
  
  for(;;){
    //if(xSemaphoreTake(gatekeeper, 100)){
      //Serial.println("User 2 got access");
      if (bNewInt) { //new read interrupt
        //Serial.print(F("Interrupt. "));
        mfrc522.PICC_ReadCardSerial(); //read the tag data
        // Show some details of the PICC (that is: the tag/card)
        Serial.print(F("Card UID:"));
        dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
        
        for(byte i=0; i<mfrc522.uid.size; i++) {
          same = memcmp (mfrc522.uid.uidByte[i], storedID[i], 4);
          if(same!=0) break;
        }

        if(xSemaphoreTake(gatekeeper, 10)){     
          lcd.clear();
          lcd.setCursor(0, 0);
          if(same == 0) {
            lcd.print("Correct");
          }
          else {
            lcd.print("Wrong!");
            lcd.setCursor(6, 0);
            for(byte i=0; i<mfrc522.uid.size; i++) {
              //Serial.println(i);
              lcd.print(mfrc522.uid.uidByte[i], HEX);
              lcd.setCursor(8+2*i, 0);
            }
          }
          vTaskDelay(1000/portTICK_PERIOD_MS);
          lcd.clear();
          lcd.setCursor(0, 0);  // setting cursor   
          lcd.print("Input:"); 
          same = 5;
          xSemaphoreGive(gatekeeper);
        }
        vTaskDelay(1000/portTICK_PERIOD_MS);
        same = 5;
        Serial.println();
    
        clearInt(mfrc522);
        mfrc522.PICC_HaltA();
        bNewInt = false;
      }
      activateRec(mfrc522);
      delay(100);  
      vTaskDelay(5);
  }
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    //Serial.println(buffer[i], HEX);
  }

}
/**
 * MFRC522 interrupt serving routine
 */
void readCard() {
  bNewInt = true;
}

/*
 * The function sending to the MFRC522 the needed commands to activate the reception
 */
void activateRec(MFRC522 mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
  mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);
}

/*
 * The function to clear the pending interrupt bits after interrupt serving routine
 */
void clearInt(MFRC522 mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.ComIrqReg, 0x7F);
}
