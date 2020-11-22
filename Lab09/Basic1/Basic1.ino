const int pResistor1 = A0;
const int pResistor2 = A1;

int pins[8] = {2, 3, 4, 5, 6, 7, 13, 12};//pins to 7-seg.
boolean data[3][8] = { 
  {true, true, true, true, true, true, false, false}, // 0
  {false, true, true, false, false, false, false, false}, // 1
  {true, true, false, true, true, false, true, false} // 2
};

int value1;
int value2;

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
}   

int i;


void loop(){
  for(i=0; i<8; i++){
    digitalWrite(pins[i], data[2][i] == true ? HIGH : LOW);  
  }

  value1 = analogRead(pResistor1);
  value2 = analogRead(pResistor2);

  if (value1 < 600 && value2 < 500){
    for(i=0; i<8; i++){
      digitalWrite(pins[i], data[0][i] == true ? HIGH : LOW);  
    }
  }
  else if (value1 > 600 && value2 > 500){
    for(i=0; i<8; i++){
      digitalWrite(pins[i], data[2][i] == true ? HIGH : LOW);  
    }
  }
  else {
    for(i=0; i<8; i++){
      digitalWrite(pins[i], data[1][i] == true ? HIGH : LOW);  
    }
  }
  Serial.print("A0:");
  Serial.print(value1);

  Serial.print("   A1:");
  Serial.println(value2);
}
