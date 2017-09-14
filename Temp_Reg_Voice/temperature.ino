void temperature_control(void) {

  if (digitalRead(BUTTON) == HIGH) {
    if (digitalRead(BLUE_LED) == LOW) {
      digitalWrite(BLUE_LED, HIGH);
      reg_status = HIGH;
    }
    else if (digitalRead(BLUE_LED) == HIGH) {
      digitalWrite(BLUE_LED, LOW);
      reg_status = LOW;
    }
  }
  
  //reg_status = digitalRead(BLUE_LED);


  temperature = analogRead(LM35) * 500 / 1023;
  min_temp = 23;
  max_temp = 25;
  delay(1000);

  // if temp monitoring is on
  if (reg_status == HIGH) {

    // status is 1 when it not at the desired temp, 0 when it is too hot
    heater_status = temperature < ( (max_temp + min_temp) / 2);
    fan_status = temperature > ( (max_temp + min_temp) / 2);

    // if the heater is already on and the temp is not at average
    if ( (HEAT_PIN == LOW) && (heater_status) == 1) {

    }


    // if the fan is already on and the temp is not at average
    else if ( (FAN_PIN == HIGH) && (fan_status) == 1) {
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
}
