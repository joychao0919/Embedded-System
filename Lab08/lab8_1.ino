const int LED_R = 5;
const int LED_G = 10;
const int LED_B = 6;

//void setup(){
//  Serial.begin(9600);
//  
//  pinMode(LED_R, OUTPUT); // R
//  pinMode(LED_G, OUTPUT); // G
//  pinMode(LED_B, OUTPUT); // B
//
//  randomSeed(analogRead(A0));
//  
//  cli(); // stop interrupts, atomic access to reg.
//  
//  // initialize timer1 
//  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
//  TCCR1B |= (1 << WGM12); // turn on CTC
//  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
////  OCR1A = 156;  // target for counting
//  OCR1A = 1564;  // target for counting
////  OCR1A = 31;  // target for counting
//  TIMSK1 |= (1<<OCIE1A); // enable timer compare int.
//
//  // initialize timer0
//  TCCR0A = 0;    TCCR0B = 0;    TCNT0 = 0;
//  TCCR0B |= (1 << WGM12); // turn on CTC
//  TCCR0B |= (1 << CS12) | (1<<CS10); // 1024 prescaler
//  OCR0A = 156;  // target for counting
//  TIMSK0 |= (1<<OCIE0A); // enable timer compare int.
//
//  sei(); // enable all interrupts
//
//}
//
//void loop(){ }
//
//int intensity = -1;
//void led(int intens){
//  intensity = intens;
//}
//
//int counter = 0;
//ISR(TIMER0_COMPA_vect) {
//  counter++;
//  if(counter==100){
//    int randomIntens = random(0, 6);
//    led(randomIntens);
//    counter = 0;
//  }
//}
//
//
//int ledCounter = 0;
//ISR(TIMER1_COMPA_vect) {
//  if(ledCounter<5-intensity){
//    digitalWrite(LED_R, LOW);
//  }
//   else{
//    digitalWrite(LED_R, HIGH);
//  }
//  ledCounter++;
//  ledCounter = ledCounter%5;
//}





//////////////////////////////////////////////////

int toggle;

void setup() {
  pinMode(LED_R, OUTPUT);
  Serial.begin(9600);
  analogWrite(LED_R, 100);
  randomSeed(analogRead(A0));

  cli(); // stop interrupts, atomic access to reg.
  // initialize timer2
  TCCR2A = 0;    TCCR2B = 0;    TCNT2 = 0;
  TCCR2B |= (1 << WGM12); // turn on CTC
  TCCR2B |= (1 << CS12) | (1<<CS10); // 1024 prescaler
  OCR2A = 255;  // target for counting
  TIMSK2 |= (1<< OCIE2A); // enable timer compare int.
  sei(); // enable all interrupts
}

void loop() {}

int counter = 0;
ISR(TIMER2_COMPA_vect) {
  counter++;
  if(counter==500){
    Serial.println("count");
    int randomIntens = random(0, 256);
    analogWrite(LED_R, randomIntens);
    counter = 0;
  }
}
