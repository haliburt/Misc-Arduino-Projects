/*
	This program implements the BitVoicer Server Protocol.
	Copyright (c) 2015 BitSophia Tecnologia Ltda - ME.

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BVSP_h
#define BVSP_h

#include <Arduino.h>

#define BVSP_BUFFER_SIZE 64
#define FRAMED_MODE 0x00
#define STREAM_MODE 0x01
#define DATA_TYPE_NA 0x00
#define DATA_TYPE_BYTE 0x01
#define DATA_TYPE_INT16 0x03
#define DATA_TYPE_INT32 0x05
#define DATA_TYPE_BINARY 0x07
#define DATA_TYPE_STRING 0x08
#define DATA_TYPE_STATUS 0x0F

typedef void (*ModeChangedPtr)(void);
typedef void (*FrameReceivedPtr)(byte, int);
typedef void (*StreamReceivedPtr)(int);

class BVSP
{
public:
	BVSP();
	void begin(Stream &serialPort, unsigned long statusRequestTimeout, 
		unsigned long statusRequestInterval);
	void setInboundMode(byte mode);
	void reset();
	byte inboundMode;
	byte outboundMode;
	volatile int usedMemory;

	void requestStatus();
	void keepAlive();
	boolean hasStatusTimedOut();
	boolean isBVSRunning();
	boolean isSREAvailable();
	boolean isDataFwdRunning();

	void send(byte data);
	void send(int data);
	void send(long data);
	void send(byte *data, int count);
	void send(byte *data, int offset, int count);
	void send(char *data);
	void sendStream(byte *data, int count);

	void receive();
	byte getReceivedByte();
	int getReceivedInt16();
	long getReceivedInt32();
	int getReceivedBytes(byte *buffer, int bufferSize);
	int getReceivedBytes(byte *buffer, int bufferSize, int offset, int count);
	int getReceivedString(char *buffer, int bufferSize);
	int getReceivedStream(byte *buffer, int bufferSize);
	int getReceivedStream(short *buffer, int bufferSize);
	int getReceivedStream(byte *buffer, int bufferSize, int offset, int count);
	int getReceivedStream(short *buffer, int bufferSize, int offset, int count);
	boolean isReceiving;
	ModeChangedPtr modeChanged;
	FrameReceivedPtr frameReceived;
	StreamReceivedPtr streamReceived;
private:
	byte buildByteOne(byte dataType, int payloadSize);
	Stream *mSerial;

	void updateStatus(unsigned long receiveTime);
	boolean mBVSStatus;
	boolean mSREStatus;
	boolean mDataFwdStatus;
	byte mAleatoryNumber;
	unsigned long mStatusRequestTimeout;
	unsigned long mStatusRequestInterval;
	unsigned long mStatusRequestTime;
	boolean mStatusTimedOut;
	boolean mStatusReceived;

	bool readByteOne();
	bool readByteTwo();
	void resetFrame();
	int getDataStart();
	int getStreamStart();
	byte mBuffer[BVSP_BUFFER_SIZE];
	volatile int mWritePos;
	volatile int mReadPos;
	byte mCurrFrameByte;
	int mFrameStart;
	byte mFrameType;
	byte mDataType;
	int mPayloadSize;
	int mPayloadPos;
	byte mModeChangeSignalPos;
	int mCurrStreamSize;
};

#endif