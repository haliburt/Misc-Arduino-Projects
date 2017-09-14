#include <BVSP.h>

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
    
  // Sets the Arduino serial port that will be used for 
  // communication, how long it will take before a status request 
  // times out and how often status requests should be sent to 
  // BitVoicer Server
  bvsp.begin(Serial, STATUS_REQUEST_TIMEOUT, 
    STATUS_REQUEST_INTERVAL);
    
  // Sets the function that will handle the frameReceived 
  // event
  bvsp.frameReceived = BVSP_frameReceived;
}

void loop() 
{
  // Checkes if there is data available at the serial port buffer 
  // and processes its content according to the specifications 
  // of the BitVoicer Server Protocol
  bvsp.receive();
}

// Handles the frameReceived event
void BVSP_frameReceived(byte dataType, int payloadSize)
{
  int bytesRead = 0;
  
  // Performs the appropriate actions based on the frame 
  // data type
  switch (dataType)
  {
    case DATA_TYPE_BYTE:
      // Gets the received byte and sends it back to 
      // BitVoicer Server
      bvsp.send(bvsp.getReceivedByte());
      
      break;
    case DATA_TYPE_INT16:
      // Gets the received int16 and sends it back to 
      // BitVoicer Server
      bvsp.send(bvsp.getReceivedInt16());
      
      break;
    case DATA_TYPE_INT32:
      // Gets the received int32 and sends it back to 
      // BitVoicer Server
      bvsp.send(bvsp.getReceivedInt32());
      
      break;
    case DATA_TYPE_BINARY:
      byte byteBuffer[64];
      
      // Retrieves the binary data from the frame
      bytesRead = bvsp.getReceivedBytes(byteBuffer, 64);
      
      // Sends the binary data back to BitVoicer Server
      bvsp.send(byteBuffer, 0, bytesRead);
      
      break;
    case DATA_TYPE_STRING:
      char charBuffer[64];
      
      // Retrieves the string from the frame
      bvsp.getReceivedString(charBuffer, 64);
      
      // Sends the string back to BitVoicer Server
      bvsp.send(charBuffer);
      
      break;
  }
}
