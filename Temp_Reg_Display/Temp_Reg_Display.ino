#define FAN_PIN           6
#define HEAT_PIN          7
#define AUTO_TEMP         11
#define BUTTON            2
#define BVSM_AUDIO_INPUT  A0
#define LM35              A1

long int temperature=0;
char tempout[] = {0, 0};
volatile int reg_status = 0;
boolean heater_status = 0;
boolean fan_status = 0;
float min_temp = 23;
float max_temp = 25;
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 200;
int counterlimit = 7;
int tempcounter = 0;

const int strobe = 4;
const int clock = 5;
const int data = 8;
int digit[5] = {NULL};
int dispcounter = counterlimit;
int tempcheck = 0;
int enter = 1;

int LED_disp[11] = {0x3F, 0x30, 0x5B, 0x79, 0x74, 0x6D, 0x6F , 0x38, 0x7F, 0x7C, 0x8F};


void reset()
{
  sendCommand(0x40); // set auto increment mode
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc0);   // set starting address to 0
  for (uint8_t i = 0; i < 16; i++)
  {
    shiftOut(data, clock, LSBFIRST, 0x00);
  }
  digitalWrite(strobe, HIGH);
}

void setup() {
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(HEAT_PIN, HIGH);
  digitalWrite(AUTO_TEMP, LOW);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(HEAT_PIN, OUTPUT);
  pinMode(AUTO_TEMP, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON), state, LOW);

  pinMode(strobe, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, OUTPUT);

  sendCommand(0x8f);  // activate
  reset();
}

void state() {
  if ( (millis() - lastDebounceTime) > debounceDelay) {
    if (digitalRead(AUTO_TEMP) == LOW) {
      digitalWrite(AUTO_TEMP, HIGH);
      reg_status = 1;
      tempcounter = counterlimit;
    }

    else if (digitalRead(AUTO_TEMP) == HIGH) {
      digitalWrite(AUTO_TEMP, LOW);
      reg_status = 0;
      digitalWrite(HEAT_PIN, HIGH);
      digitalWrite(FAN_PIN, LOW);
    }
    lastDebounceTime = millis();
  }
}

void sendCommand(uint8_t value)
{
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, value);
  digitalWrite(strobe, HIGH);
}

void loop() {
  temperature = analogRead(LM35) *500/ 1023;
  delay(1000);
  // if temp monitoring is on
  if (reg_status == 1 && tempcounter == counterlimit) {
    
    if (digitalRead(FAN_PIN) == HIGH){
      if(temperature < min_temp+1){
       digitalWrite(FAN_PIN, LOW);
      }
    }
    
    else if (digitalRead(HEAT_PIN) == LOW){
      if(temperature > max_temp-1){
       digitalWrite(HEAT_PIN, HIGH);
      }
    }
    
    else if (temperature > max_temp) { // set to cool down
      digitalWrite(FAN_PIN, HIGH);
      digitalWrite(HEAT_PIN, HIGH);
    }

    else if (temperature < min_temp) { // set to turn up heat
      digitalWrite(HEAT_PIN, LOW);
      digitalWrite(FAN_PIN, LOW);
    }

    else { // turn everything off
      digitalWrite(HEAT_PIN, HIGH);
      digitalWrite(FAN_PIN, LOW);
    }
  }
  
  tempcounter++;
  if(tempcounter>counterlimit){
    tempcounter = 0;
  }

  sendCommand(0x44);  // set single address

  if (temperature > 19) {
    digit[0] = 2;
  }
  else {
    digit[0] = 1;
  }

  for (int i = 0; i < 10; i++) {
    if ((digit[0] * 10 + i) == temperature) {
      digit[1] = i;
    }
  }
  
  if (enter || abs(tempcheck-temperature)<3) {
    digitalWrite(strobe, LOW);
    shiftOut(data, clock, LSBFIRST, 0xca);
    shiftOut(data, clock, LSBFIRST, LED_disp[digit[1]]);
    digitalWrite(strobe, HIGH);

    digitalWrite(strobe, LOW);
    shiftOut(data, clock, LSBFIRST, 0xcc);
    shiftOut(data, clock, LSBFIRST, LED_disp[digit[0]]);
    digitalWrite(strobe, HIGH);

    digitalWrite(strobe, LOW);
    shiftOut(data, clock, LSBFIRST, 0xc8);
    shiftOut(data, clock, LSBFIRST, LED_disp[10]);
    digitalWrite(strobe, HIGH);
    enter = 0;
  }
  tempcheck = temperature;
}
