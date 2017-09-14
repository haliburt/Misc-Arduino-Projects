#include <BVSP.h>
#include <BVSMic.h>

// Defines the Arduino pins that will be used to control
// LEDs and capture audio
#define BVS_RUNNING       3
#define BVS_SRE           5
#define BVS_DATA_FWD      4
#define BVS_ACT_PERIOD    6
#define BVSM_AUDIO_INPUT  0

// Defines the constants that will be passed as parameters to 
// the BVSP.begin function
const unsigned long STATUS_REQUEST_INTERVAL = 2000;
const unsigned long STATUS_REQUEST_TIMEOUT = 1000;

// Defines the size of the audio buffer
const int AUDIO_BUFFER_SIZE = 64;

// Defines the size of the string buffer
const int STRING_BUFFER_SIZE = 64;

// Initializes a new global instance of the BVSP class
BVSP bvsp = BVSP();

// Initializes a new global instance of the BVSMic class
BVSMic bvsm = BVSMic();

// Creates a buffer that will be used to read recorded samples 
// from the BVSMic class
byte audioBuffer[AUDIO_BUFFER_SIZE];

// Creates a string buffer (char array) to retrieve strings
// sent from BitVoicer Server
char stringBuffer[STRING_BUFFER_SIZE];

void setup() 
{
  // Starts serial communication at 115200 bps
  Serial.begin(115200);
  
  // Sets the Arduino pin modes
  pinMode(BVS_RUNNING, OUTPUT);
  pinMode(BVS_SRE, OUTPUT);
  pinMode(BVS_DATA_FWD, OUTPUT);
  pinMode(BVS_ACT_PERIOD, OUTPUT);
  
  // Sets the Arduino serial port that will be used for 
  // communication, how long it will take before a status request 
  // times out and how often status requests should be sent to 
  // BitVoicer Server
  bvsp.begin(Serial, STATUS_REQUEST_TIMEOUT, 
    STATUS_REQUEST_INTERVAL);
    
  // Sets the function that will handle the frameReceived 
  // event
  bvsp.frameReceived = BVSP_frameReceived;
  
  // Prepares the BVSMic class timer
  bvsm.begin();
}

void loop() 
{
  // Checks if the status request interval has elapsed and if it 
  // has, sends a status request to BitVoicer Server
  bvsp.keepAlive();
  
  // Checks if there is data available at the serial port buffer 
  // and processes its content according to the specifications 
  // of the BitVoicer Server Protocol
  bvsp.receive();
  
  // Gets the respective status from the BVSP class and sets 
  // the LEDs on or off
  digitalWrite(BVS_RUNNING, bvsp.isBVSRunning());
  digitalWrite(BVS_DATA_FWD, bvsp.isDataFwdRunning());
  
  // Checks if there is a SRE assigned to the Arduino
  if (bvsp.isSREAvailable())
  {
    // Turns on the SRE available LED
    digitalWrite(BVS_SRE, HIGH);
    
    // If the BVSMic class is not recording, sets up the audio 
    // input and starts recording
    if (!bvsm.isRecording)
    {
      bvsm.setAudioInput(BVSM_AUDIO_INPUT, EXTERNAL);
      bvsm.startRecording();
    }
    
    // Checks if the BVSMic class has available samples
    if (bvsm.available)
    {
      // Makes sure the inbound mode is STREAM_MODE before 
      // transmitting the stream
      if (bvsp.inboundMode == FRAMED_MODE)
        bvsp.setInboundMode(STREAM_MODE);
      
      // Reads the audio samples from the BVSMic class
      int bytesRead = bvsm.read(audioBuffer, AUDIO_BUFFER_SIZE);
      
      // Sends the audio stream to BitVoicer Server
      bvsp.sendStream(audioBuffer, bytesRead);
    }
  }
  else
  {
    // There is no SRE available
    // Turns off the SRE and ACT_PERIOD LEDs
    digitalWrite(BVS_SRE, LOW);
    digitalWrite(BVS_ACT_PERIOD, LOW);
    
    // If the BVSMic class is recording, stops it
    if (bvsm.isRecording)
      bvsm.stopRecording();
  }
}

// Handles the frameReceived event
void BVSP_frameReceived(byte dataType, int payloadSize)
{
  // Performs the appropriate actions based on the frame 
  // data type
  switch (dataType)
  {
    case DATA_TYPE_BYTE:
      // Turns the ACT_PERIOD LED on or off depending on the 
      // the value of the received byte 
      digitalWrite(BVS_ACT_PERIOD, bvsp.getReceivedByte());
      break;
    case DATA_TYPE_STRING:
      // Performs further actions only if characters are retrieved
      // from the frame
      if (bvsp.getReceivedString(stringBuffer, STRING_BUFFER_SIZE) != 0);
      {
        // If the received string is equal to blink, turns 
        // all LEDs off and on 3 times
        if (strcmp(stringBuffer, "blink") == 0)
        {
          AllLEDsOff();
          delay(100);
          AllLEDsOn();
          delay(100);
          AllLEDsOff();
          delay(100);
          AllLEDsOn();
          delay(100);
          AllLEDsOff();
          delay(100);
          AllLEDsOn();
        }
      }
      
      break;  
  }
}

// Turns all LEDs on
void AllLEDsOn()
{
  digitalWrite(BVS_RUNNING, HIGH);
  digitalWrite(BVS_SRE, HIGH);
  digitalWrite(BVS_DATA_FWD, HIGH);
  digitalWrite(BVS_ACT_PERIOD, HIGH);
}

// Turns all LEDs off
void AllLEDsOff()
{
  digitalWrite(BVS_RUNNING, LOW);
  digitalWrite(BVS_SRE, LOW);
  digitalWrite(BVS_DATA_FWD, LOW);
  digitalWrite(BVS_ACT_PERIOD, LOW);
}
