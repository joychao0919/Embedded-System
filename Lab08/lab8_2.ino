const int LED_R = 5;
const int LED_G = 10;
const int LED_B = 6;

const int joyStick_int = 0;     
const int button = 2;
const int xAxis = A0,     yAxis = A1;

void setup(){
  Serial.begin(9600);
  pinMode(button, INPUT_PULLUP);  //return LOW when down
  pinMode(LED_R, OUTPUT);
  pinMode(LED_B, OUTPUT);
  
  attachInterrupt(joyStick_int, handle_click, RISING);
  
  cli(); // stop interrupts, atomic access to reg.
  // initialize timer1
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC
  TCCR1B |= (1 << CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 3910;  // target for counting
  TIMSK1 |= (1<< OCIE1A); // enable timer compare int.
  
  // initialize timer2
  TCCR2A = 0;    TCCR2B = 0;    TCNT2 = 0;
  TCCR2B |= (1 << WGM12); // turn on CTC
  TCCR2B |= (1 << CS12) | (1<<CS10); // 1024 prescaler
  OCR2A = 128;  // target for counting
  TIMSK2 |= (1<< OCIE2A); // enable timer compare int.

  sei(); // enable all interrupts
}

void loop(){}

int ledOn = 1;
int blinkOn = 0;
int xVal;
int yVal;


ISR(TIMER1_COMPA_vect){
  if(blinkOn==0){
    ledOn = 1;
  }
  else{
    if(ledOn==0){
      ledOn = 1;
    }
    else{
      ledOn = 0;
    }
  }
}

int isPress = 0;
ISR(TIMER2_COMPA_vect){
  int xVal_now = analogRead(xAxis);
  int yVal_now = analogRead(yAxis);
  if(ledOn==0){
    analogWrite(LED_B, 0);
  }
  else if(ledOn==1){
    xVal = xVal_now;
    analogWrite(LED_B, (xVal-1)/4);
  }
  
  if(ledOn==0){
    analogWrite(LED_R, 0);
  }
  if(ledOn==1){
    yVal = yVal_now;
    analogWrite(LED_R, (yVal-1)/4);
  }
//  if(isPress == 0) Serial.println("Button is pressed.");
//  else Serial.println("Button is not pressed.");
}

void handle_click() { // button debouncing, toggle LED
  int isPress_now = digitalRead(button);

  if(isPress == 0 && isPress_now==1){
    if(blinkOn==0){
      blinkOn = 1;
    }
    else{
      blinkOn = 0;
    }
    Serial.println(blinkOn);
    isPress = 1;
  }
  else if(isPress == 1 && isPress_now==0){
    isPress = 0;
  }
}
