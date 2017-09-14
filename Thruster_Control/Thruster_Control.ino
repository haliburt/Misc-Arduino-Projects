#include <Servo.h>
// kill switch pin
#define KILL_PIN 2

// default pinouts for mc
#define HOR_PWM_L 20
#define HOR_PWM_R 21

#define VER_PWM_L 22
#define VER_PWM_R 23

#define STR_PWM_L 5
#define STR_PWM_R 6

Servo ver_l;//blue robotics
Servo ver_r;//blue robotics
Servo hor_l;//seabotix
Servo hor_r;//seabotix
Servo str_l;//seabotix
Servo str_r;//seabotix

void setup () {
  ver_l.attach(VER_PWM_L);
  ver_r.attach(VER_PWM_R);
  hor_l.attach(HOR_PWM_L);
  hor_r.attach(HOR_PWM_R);
  str_l.attach(STR_PWM_L);
  str_r.attach(STR_PWM_R);
  pinMode(KILL_PIN, INPUT);
  //ver_l.writeMicroseconds(1500); // send "stop" signal to bluerobotics ESC.
  //ver_r.writeMicroseconds(1500);
  delay(1000); // delay to allow the ESC to recognize the stopped signal
}

void loop() {
  // SEABOTIX
  // Max Reverse: 0
  // Stopped: 90
  // Max Forward: 180

  // BLUEROBOTICS
  // Max Reverse: 1100
  // Stopped: 1500
  // Max Forward: 1900
  int thrust_speed;
  if (digitalRead(KILL_PIN)) {
    thrust_speed = 10; //Percentage from -100% to 100%
  }
  else {
    thrust_speed = 0;
  }

  int bluerobotics = map(thrust_speed, -100, 100, 1100, 1900);
  int seabotix = map(thrust_speed, -100, 100, 0, 180);

  ver_l.write(bluerobotics);
  ver_r.write(bluerobotics);
  hor_l.write(seabotix + 5); //fix offset from motor controller
  hor_r.write(seabotix + 5);
  str_l.write(seabotix);
  str_r.write(seabotix * 1.05); // slow thruster
}

