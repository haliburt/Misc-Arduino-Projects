#include <DAC.h>
#include <BVSP.h>
#include <BVSMic.h>
#include <BVSSpeaker.h>

// Defines the Arduino pins that will be used to control
// LEDs and capture audio
#define BVS_RUNNING       3
#define BVS_SRE           5
#define BVS_DATA_FWD      4
#define BVS_ACT_PERIOD    6
#define BVSM_AUDIO_INPUT  0

// Defines the constants that will be passed as parameters to 
// the BVSP.begin function
const unsigned long STATUS_REQUEST_INTERVAL = 4000;
const unsigned long STATUS_REQUEST_TIMEOUT = 2000;

// Defines the size of the mic buffer
const int MIC_BUFFER_SIZE = 64;

// Defines the size of the speaker buffer
const int SPEAKER_BUFFER_SIZE = 128;

// Initializes a new global instance of the BVSP class
BVSP bvsp = BVSP();

// Initializes a new global instance of the BVSMic class
BVSMic bvsm = BVSMic();

// Initializes a new global instance of the BVSSpeaker class
BVSSpeaker bvss = BVSSpeaker();

// Creates a buffer that will be used to read recorded samples 
// from the BVSMic class
byte micBuffer[MIC_BUFFER_SIZE];

// Creates a buffer that will be used to write audio samples 
// into the BVSSpeaker class
byte speakerBuffer[SPEAKER_BUFFER_SIZE];

// Creates a global variable that indicates whether the 
// Arduino is connected to BitVoicer Server
boolean connected = false;

void setup() 
{
  // Starts serial communication at 115200 bps
  Serial.begin(115200);
  
  // Sets the Arduino pin modes and turns all leds off
  pinMode(BVS_RUNNING, OUTPUT);
  pinMode(BVS_SRE, OUTPUT);
  pinMode(BVS_DATA_FWD, OUTPUT);
  pinMode(BVS_ACT_PERIOD, OUTPUT);
  AllLEDsOff();
  
  // Sets the Arduino serial port that will be used for 
  // communication, how long it will take before a status request 
  // times out and how often status requests should be sent to 
  // BitVoicer Server
  bvsp.begin(Serial, STATUS_REQUEST_TIMEOUT, 
    STATUS_REQUEST_INTERVAL);
    
  // Sets the function that will handle the frameReceived 
  // event
  bvsp.frameReceived = BVSP_frameReceived;
  
  // Sets the function that will handle the modeChanged 
  // event
  bvsp.modeChanged = BVSP_modeChanged;
  
  // Sets the function that will handle the streamReceived 
  // event
  bvsp.streamReceived = BVSP_streamReceived;
  
  // Prepares the BVSMic class timer
  bvsm.begin();
  
  // Sets the DAC that will be used by the BVSSpeaker class
  bvss.begin(DAC);
}

void loop() 
{
  // If it is not connected to the server, opens a TCP/IP 
  // connection, sets connected to true and resets the BVSP 
  // class
  if (!connected)
  {
    Connect(Serial);
    connected = true;
    bvsp.reset();
  }
  
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
      bvsm.setAudioInput(BVSM_AUDIO_INPUT, DEFAULT);
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
      int bytesRead = bvsm.read(micBuffer, MIC_BUFFER_SIZE);
      
      // Sends the audio stream to BitVoicer Server
      bvsp.sendStream(micBuffer, bytesRead);
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
  
  // Plays the audio samples available in the internal buffer
  bvss.play();
  
  // If the status has timed out, the connection is considered 
  // lost
  if (bvsp.hasStatusTimedOut())
  {
    // If the BVSMic is recording, stops it
    if (bvsm.isRecording)
      bvsm.stopRecording();
    
    // Closes the TCP/IP connection
    Disconnect(Serial);
    
    AllLEDsOff();
    connected = false;
  }
}

// Handles the modeChanged event
void BVSP_modeChanged()
{
  // If the outboundMode (Server --> Device) has turned to 
  // FRAMED_MODE, no audio stream is supposed to be received.
  // Tells the BVSSpeaker class to finish playing when its 
  // internal buffer become empty.
  if (bvsp.outboundMode == FRAMED_MODE)
    bvss.finishPlaying();
}

// Handles the streamReceived event
void BVSP_streamReceived(int size)
{
  // Gets the received stream from the BVSP class  
  int bytesRead = bvsp.getReceivedStream(speakerBuffer, 
    SPEAKER_BUFFER_SIZE);
    
  // Enqueues the received stream for reproduction
  bvss.enqueue(speakerBuffer, bytesRead);
}

// Handles the frameReceived event
void BVSP_frameReceived(byte dataType, int payloadSize)
{
  // Performs the appropriate actions based on the frame 
  // data type
  switch (dataType)
  {
    case DATA_TYPE_BYTE:
      // Turns the ACT_PERIOD LED on or off based on the 
      // the value of the received byte 
      digitalWrite(BVS_ACT_PERIOD, bvsp.getReceivedByte());
      break;
  }
}

// Opens a TCP/IP connection with the BitVoicer Server
void Connect(HardwareSerial &serialPort)
{
  serialPort.print("$$$");
  delay(500);
  
  // Use the IP address of the server and the TCP port set 
  // in the server properties
  serialPort.println("open 192.168.0.117 4194");
  
  delay(1000);
  serialPort.println("exit");
  delay(500);
}

// Closes the TCP/IP connection with the BitVoicer Server
void Disconnect(HardwareSerial &serialPort)
{
  serialPort.print("$$$");
  delay(500);
  serialPort.println("close");
  delay(1000);
  serialPort.println("exit");
  delay(500);
}

// Turns all LEDs off
void AllLEDsOff()
{
  digitalWrite(BVS_RUNNING, LOW);
  digitalWrite(BVS_SRE, LOW);
  digitalWrite(BVS_DATA_FWD, LOW);
  digitalWrite(BVS_ACT_PERIOD, LOW);
}
