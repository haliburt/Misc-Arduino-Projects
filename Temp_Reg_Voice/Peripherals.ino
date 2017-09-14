void fan(String fan_state) {
  if (fan_state == "on") {
    digitalWrite(FAN_PIN, HIGH);
    digitalWrite(BLUE_LED, LOW);
  }
  else {
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(BLUE_LED, LOW);
  }
}

void heater(String heat_state) {
  if (heat_state == "on") {
    digitalWrite(HEAT_PIN, LOW);
    digitalWrite(BLUE_LED, LOW);
  }
  else {
    digitalWrite(HEAT_PIN, HIGH);
    digitalWrite(BLUE_LED, LOW);
  }
}

void auto_reg(String reg_state) {
  if (reg_state == "on") {
    digitalWrite(BLUE_LED, HIGH);
  }
  else {
    digitalWrite(BLUE_LED, LOW);
  }
}
