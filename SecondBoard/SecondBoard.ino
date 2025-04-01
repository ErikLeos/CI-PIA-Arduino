#include <Wire.h>

int x = 0;

void setup() {
  Serial.begin(9600);
  for(int i=2; i<=10; i++){
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }

  Wire.begin(1);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

}

void requestEvent(){
  Wire.write("REQUEST");
}

void receiveEvent(int bytes){
  x = Wire.read();
  Serial.println(x);

  if(x < 10){
    digitalWrite(x + 2, HIGH);
  }
  else{
    digitalWrite(x - 8, LOW);
  }

}

void loop() {
  
}
