void setup() {
  for (int pin = 2; pin<=13; pin++){
    pinMode(pin, OUTPUT);
  }
}

void loop() {
  for (int white = 11; white < 14; white++) {
    digitalWrite(white, HIGH);
    for (int blue = 2; blue < 11; blue++) {
      digitalWrite(blue, HIGH);
      delay(100);
      digitalWrite(blue, LOW);
    }
    delay(100);
    digitalWrite(white, LOW);
  }
  for(int i = 0; i<6; i++){
  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);

  digitalWrite(2, HIGH);
  digitalWrite(10, HIGH);
  delay(70);

  digitalWrite(2, LOW);
  digitalWrite(10, LOW);
  digitalWrite(3, HIGH);
  digitalWrite(9, HIGH);
  delay(70);

  digitalWrite(3, LOW);
  digitalWrite(9, LOW);
  digitalWrite(8, HIGH);
  digitalWrite(4, HIGH);
  delay(70);

  digitalWrite(8, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, HIGH);
  digitalWrite(7, HIGH);
  delay(70);

  digitalWrite(5, LOW);
  digitalWrite(7, LOW);
  digitalWrite(2, HIGH);
  digitalWrite(10,HIGH);
  delay(60);
  }

  for (int i = 2; i<11; i++){
    digitalWrite(i,HIGH);
  }

  for (int j = 11; j<14; j++){
    digitalWrite(j, HIGH);
    delay(70);
    digitalWrite(j,LOW);
  }
  
  for (int j = 13; j>10; j--){
    digitalWrite(j, HIGH);
    delay(70);
    digitalWrite(j,LOW);
  }

}

