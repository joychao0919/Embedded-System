#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <queue.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#define KEY_ROWS 4
#define KEY_COLS 4

LiquidCrystal_I2C lcd(0x3F,16,2);
QueueHandle_t Global_Queue_Handle = 0;

void controlTask(void *pvParameters);
void displayTask(void *pvParameters);

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
  lcd.setCursor(0, 0);  // setting cursor   
  lcd.print("A");

  xTaskCreate(controlTask, (const portCHAR *) "task_control", 128, NULL, 1, NULL);
  xTaskCreate(displayTask, (const portCHAR *) "task_display", 128, NULL, 1, NULL);

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
      lcd.print("A");      
    }
  }
}
