#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <queue.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#define KEY_ROWS 4
#define KEY_COLS 4

byte minesweeper[8] = {B01110, B01110, B10100, B11111, B00101, B01110, B01010, B11011};
byte unchecked[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111};
byte exploded[8] = {B00100, B10101, B01110, B11111, B11111, B01110, B10101, B00100};
byte succeeded[8] = {B00100, B00110, B00111, B00110, B00100, B00100, B01110, B11111};

LiquidCrystal_I2C lcd(0x3F,16,2);
QueueHandle_t Global_Queue_Handle = 0;

TaskHandle_t TaskHandle_Map;

void controlTask(void *pvParameters);
void displayTask(void *pvParameters);
void mapTask(void *pvParameters);

int mine_i = 0;
int mine_j = 0;
int mineRemoverCnt = 10;

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
  Global_Queue_Handle = xQueueCreate(3, sizeof(int));

  lcd.init();    // initialize LCD
  lcd.backlight();    // open LCD backlight
  lcd.createChar(0, minesweeper);
  lcd.createChar(1, unchecked);
  lcd.createChar(2, exploded);
  lcd.createChar(3, succeeded);
  lcd.setCursor(0, 0);
  lcd.write(0);

  for(i=0; i<2; i++) {
    for(j=0; j<16; j++) {
      if(i==0 && j==0){}
      else{
        lcd.setCursor(j, i);
        lcd.write(1);
      }
    }
  }

  xTaskCreate(controlTask, (const portCHAR *) "task_control", 128, NULL, 1, NULL);
  xTaskCreate(displayTask, (const portCHAR *) "task_display", 128, NULL, 1, NULL);
  xTaskCreate(mapTask, (const portCHAR *) "task_map", 128, NULL, 1, &TaskHandle_Map);

  vTaskStartScheduler();
}

void loop(){}

void controlTask(void *pvParameters) {
  (void) pvParameters;
  int dir_send_row = 0; // 0 1
  int dir_send_col = 0; // 0~15
  for(;;){
    
    key = myKeypad.getKey();
    if(key == '2'){ // up
      if(dir_send_row == 1){ // 2nd row
        dir_send_row -= 1;
        xQueueSend(Global_Queue_Handle, &dir_send_row, 1000);
        xQueueSend(Global_Queue_Handle, &dir_send_col, 1000);
      }
    }
    else if(key == '4'){ // left
      if(dir_send_col > 0){
        dir_send_col -= 1;
        xQueueSend(Global_Queue_Handle, &dir_send_row, 1000);
        xQueueSend(Global_Queue_Handle, &dir_send_col, 1000);
      }
    }
    else if(key == '6'){ // right
      if(dir_send_col < 15){
        dir_send_col += 1;
        xQueueSend(Global_Queue_Handle, &dir_send_row, 1000);
        xQueueSend(Global_Queue_Handle, &dir_send_col, 1000);
      }
    }
    else if(key == '8'){
      if(dir_send_row == 0){
        dir_send_row += 1;
        xQueueSend(Global_Queue_Handle, &dir_send_row, 1000);
        xQueueSend(Global_Queue_Handle, &dir_send_col, 1000);
      }
    }
    else if(key == '5'){
      
    }
    
  }
}

void displayTask(void *pvParameters) {
  (void) pvParameters;
  int dir_recv_row = 0; //  0~15 16~31
  int dir_recv_col = 0;

  for(;;){
    if(xQueueReceive(Global_Queue_Handle, &dir_recv_row, 1000) && xQueueReceive(Global_Queue_Handle, &dir_recv_col, 1000)){
      lcd.clear();
      Serial.print("row:");
      Serial.print(dir_recv_row);
      Serial.print("   col:");
      Serial.println(dir_recv_col);
      lcd.setCursor(dir_recv_col, dir_recv_row);  // setting cursor   
      lcd.write(0);
      for(i=0; i<2; i++){
        for(j=0; j<16; j++){
          if(i==dir_recv_row && j==dir_recv_col){}
          else{
            lcd.setCursor(j, i);
            lcd.write(1);
          }
        }
      }
    }
  }
}

void mapTask(void *pvParameters) {
  (void) pvParameters;
  for(;;){
    mine_i = random(0, 2);
    mine_j = random(0, 16);
    vTaskSuspend(TaskHandle_Map);
  }
}
