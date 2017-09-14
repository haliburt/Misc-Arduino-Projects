#include <BVSP.h>

// Defines the Arduino pins that will be used to control LEDs
#define BVS_RUNNING     3
#define BVS_SRE         5
#define BVS_DATA_FWD    4

// Defines the constants that will be passed as parameters to 
// the BVSP.begin function
const unsigned long STATUS_REQUEST_INTERVAL = 2000;
const unsigned long STATUS_REQUEST_TIMEOUT = 1000;

// Initializes a new global instance of the BVSP class
BVSP bvsp = BVSP();

void setup() 
{
  // Starts serial communication at 115200 bps
  Serial.begin(115200);
  
  // Sets the Arduino pin modes
  pinMode(BVS_RUNNING, OUTPUT);
  pinMode(BVS_SRE, OUTPUT);
  pinMode(BVS_DATA_FWD, OUTPUT);
  
  // Sets the Arduino serial port that will be used for 
  // communication, how long it will take before a status request 
  // times out and how often status requests should be sent to 
  // BitVoicer Server
  bvsp.begin(Serial, STATUS_REQUEST_TIMEOUT, 
    STATUS_REQUEST_INTERVAL);
}

void loop() 
{
  // Checks if the status request interval has elapsed and if it 
  // has, sends a status request to BitVoicer Server
  bvsp.keepAlive();
  
  // Checkes if there is data available at the serial port buffer 
  // and processes its content according to the specifications 
  // of the BitVoicer Server Protocol
  bvsp.receive();
  
  // Gets the respective status from the BVSP class and sets 
  // the LEDs on or off
  digitalWrite(BVS_RUNNING, bvsp.isBVSRunning());
  digitalWrite(BVS_DATA_FWD, bvsp.isDataFwdRunning());
  digitalWrite(BVS_SRE, bvsp.isSREAvailable());
}

