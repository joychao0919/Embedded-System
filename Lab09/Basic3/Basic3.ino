#include <Arduino_FreeRTOS.h>
#include <Stepper.h>
#define STEPS 2048 

void TaskA0(void *pvParameters);
void TaskA1(void *pvParameters);
void TaskSeven(void *pvParameters);
void TaskSensor(void *pvParameters);
void TaskFlag(void *pvParameters);

Stepper stepper(STEPS, 16, 18, 17, 19);

const int pResistor1 = A0;
const int pResistor2 = A1;
const int trigPin = 9;
const int echoPin = 8;

int pins[8] = {2, 3, 4, 5, 6, 7, 13, 12};//pins to 7-seg.
boolean data[3][8] = { 
  {true, true, true, true, true, true, false, false}, // 0
  {false, true, true, false, false, false, false, false}, // 1
  {true, true, false, true, true, false, true, false} // 2
};

int value1;
int value2;

int dir;
// 0:left 1:right
int trueStep;
long duration, distance;
long last_distance;
int motor_state = 0;
// 0:initial 1:empty(left) 2:full(third position)
int last_state = 0;
int flag = 0;

int i;

void setup(){
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(pResistor1, INPUT);
  pinMode(pResistor2, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  stepper.setSpeed(10);

  for(i=0; i<8; i++){
    digitalWrite(pins[i], data[2][i] == true ? HIGH : LOW);  
  }

  xTaskCreate(TaskA0, (const portCHAR *) "task_A0", 128, NULL, 1, NULL);
  xTaskCreate(TaskA1, (const portCHAR *) "task_A1", 128, NULL, 1, NULL);
  xTaskCreate(TaskSeven, (const portCHAR *) "task_Seven", 128, NULL, 1, NULL);
  xTaskCreate(TaskSensor, (const portCHAR *) "task_Sensor", 128, NULL, 1, NULL);
  xTaskCreate(TaskFlag, (const portCHAR *) "task_Flag", 128, NULL, 1, NULL);
}   


void TaskA0(void *pvParameters) {
  (void) pvParameters;

  for(;;){
    value1 = analogRead(pResistor1);
    vTaskDelay(20);
  }
}

void TaskA1(void *pvParameters) {
  (void) pvParameters;

  for(;;){
    value2 = analogRead(pResistor2);
    vTaskDelay(20);
  }
}


void TaskSeven(void *pvParameters) {
  (void) pvParameters;

  for(;;){
      if (value1 < 700 && value2 < 600){
        for(i=0; i<8; i++){
          digitalWrite(pins[i], data[0][i] == true ? HIGH : LOW);  
        }
      }
      else if (value1 > 700 && value2 > 600){
        for(i=0; i<8; i++){
          digitalWrite(pins[i], data[2][i] == true ? HIGH : LOW);  
        }
      }
      else {
        for(i=0; i<8; i++){
          digitalWrite(pins[i], data[1][i] == true ? HIGH : LOW);  
        }
      }
      //Serial.print("A0:");
      //Serial.print(value1);
    
      //Serial.print("   A1:");
      //Serial.println(value2);
      vTaskDelay(20);
  }
}

void TaskSensor(void *pvParameters) {
  (void) pvParameters;

  for(;;){
    digitalWrite(trigPin, LOW); // Clears the trigPin
    delayMicroseconds(2);
    /* Sets the trigPin on HIGH state for 10 ms */
    digitalWrite(trigPin, HIGH);    
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    /* Reads Echo pin, returns sound travel time in ms */
    duration = pulseIn(echoPin, HIGH);
    /* Calculating the distance */
    distance = duration*0.034/2; 
    Serial.print("Distance:");
    Serial.println(distance);
    vTaskDelay(20);
  }
}

void TaskFlag(void *pvParameters) {
  (void) pvParameters;

  for(;;){
    // full and car is coming 3rd direction
    if (value1 < 700 && value2 < 600 && distance < 15 && flag == 0){
      last_distance = 1024;
      stepper.step(1024);
      flag = 1;
      Serial.println("case 1");
    }
    // empty and car is coming or left is empty - turn left
    else if ((value1 >= 700 && value2 >= 600 && distance < 15 && flag == 0) || (value2 < 600 && value1 >= 700 && distance < 15 && flag == 0)){
      last_distance = 512;
      stepper.step(512);
      flag = 1;
      Serial.println("case 2");
    }
    // right is empty - turn right
    else if (value1 < 700 && value2 >= 600 && distance < 15 && flag == 0){
      last_distance = -512;
      stepper.step(-512);
      flag = 1;
      Serial.println("case 3");
    }
    else if (distance > 15) {
      stepper.step(-last_distance);
      last_distance = 0;
      flag = 0;
    }
    vTaskDelay(20);
  }
}


void loop(){
}
