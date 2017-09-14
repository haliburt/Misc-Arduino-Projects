#include "header.h"

BVSP bvsp = BVSP();
BVSMic bvsm = BVSMic();

void setup()
{
  Serial.begin(115200);

  pinMode(BVS_RUNNING, OUTPUT);
  pinMode(BVS_SRE, OUTPUT);
  pinMode(BVS_DATA_FWD, OUTPUT);
  pinMode(BVS_ACT_PERIOD, OUTPUT);

  digitalWrite(FAN_PIN, LOW);
  digitalWrite(HEAT_PIN, HIGH);
  digitalWrite(BLUE_LED, LOW);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(HEAT_PIN, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUTTON,INPUT);

  bvsp.begin(Serial, STATUS_REQUEST_TIMEOUT, STATUS_REQUEST_INTERVAL);
  bvsp.frameReceived = BVSP_frameReceived;
  bvsm.begin();
}

void loop()
{
  speech_init();
  temperature_control();
  counter++;
}




