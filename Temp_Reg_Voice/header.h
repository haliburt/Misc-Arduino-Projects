#include <BVSP.h>
#include <BVSMic.h>

#define BVS_RUNNING       2
#define BVS_DATA_FWD      3
#define BVS_SRE           4
#define BVS_ACT_PERIOD    5


#define FAN_PIN           6
#define HEAT_PIN          7
#define BLUE_LED          11
#define GREEN_LED         9
#define RED_LED           10
#define BUTTON            11
#define BVSM_AUDIO_INPUT  A0
#define LM35              A1

const unsigned long STATUS_REQUEST_INTERVAL = 2000;
const unsigned long STATUS_REQUEST_TIMEOUT = 1000;
const int AUDIO_BUFFER_SIZE = 64;
const int STRING_BUFFER_SIZE = 64;

byte audioBuffer[AUDIO_BUFFER_SIZE];
char stringBuffer[STRING_BUFFER_SIZE];

float temperature;
float reg_status = 0;
boolean heater_status = 0;
boolean fan_status = 0;
float min_temp;
float max_temp;
int counter = 0;
