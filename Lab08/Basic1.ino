const int LED_R = 5;
const int LED_G = 3;
const int LED_B = 6;

const int joyStick_int = 0;     
const int button = 2;
const int xAxis = A0,     yAxis = A1;

void setup(){
  Serial.begin(9600);
  
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(button, INPUT_PULLUP);  //return LOW when down
  attachInterrupt(joyStick_int, handle_click, FALLING);
  
  cli(); // stop interrupts, atomic access to reg.
  // initialize timer1
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC
  TCCR1B |= (1 << CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 7812;  // target for counting
  //TIMSK1 |= (1<< OCIE1A); // enable timer compare int.
  sei(); // enable all interrupts
}

int ledOn = 0;
int blinkOn = 0;
int xVal;
int yVal;
int xVal_now;
int yVal_now;
int blueVal = 0;

int isPress = 0;
ISR(TIMER1_COMPA_vect){
  if(ledOn==0){
    analogWrite(LED_G, 0);
    analogWrite(LED_R, 0);
    analogWrite(LED_B, 0);
    ledOn = 1;
  }
  else if(ledOn==1){
    analogWrite(LED_R, (yVal-2)/4);
    analogWrite(LED_G, (xVal-2)/4);  
    analogWrite(LED_B, blueVal);
    ledOn = 0;
  }
}

void loop(){
  xVal_now = analogRead(xAxis);
  yVal_now = analogRead(yAxis);
  xVal = xVal_now;
  yVal = yVal_now;

  //Serial.println(blinkOn);
  //Serial.println(ledOn);
  
  if(abs(xVal_now-yVal_now)<=100) {
    blueVal = (xVal_now+yVal_now-4)/8;
  }

  if(abs(xVal_now)+abs(yVal_now)>=1023-100) {
    blueVal = (xVal_now-yVal_now+1024)/8;
  }

  if (blinkOn == 0) {
    analogWrite(LED_R, (yVal-2)/4);
    analogWrite(LED_G, (xVal-2)/4);  
    analogWrite(LED_B, blueVal);   
  }
  
}

void handle_click() { // button debouncing, toggle LED
  static unsigned long last_int_time = 0;
  unsigned long int_time = millis(); // Read the clock
  
  if (int_time - last_int_time > 200 ) {  
    // Ignore when < 200 msec
      Serial.println("hi");
      
      blinkOn = !blinkOn;
      Serial.println(blinkOn);
      TIMSK1 |= (1<<OCIE1A);
    /*
    if (blinkOn == 1) {
      blinkOn = 0;
      Serial.println(blinkOn);
      TIMSK1 &= !(1<<OCIE1A);
    }
    */
  }

  last_int_time = int_time;

}
