/*
	Copyright (c) 2016 BitSophia Tecnologia Ltda - ME
	Copyright (c) 2011 Arduino
	Copyright (c) ???? Brett Hagman
	Copyright (c) 2005-2006 David A. Mellis

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

	Library Version: 1.0.1
*/

#ifndef BVSMic_h
#define BVSMic_h

#include <Arduino.h>

#define BVSM_BUFFER_SIZE 128

class BVSMic
{
public:
	BVSMic();
	void begin();
	void setAudioInput(byte pin, byte analogReference);
	void startRecording();
	void stopRecording();
	void resetBuffer();
	void write(byte sample);
	int read(byte *buffer, int bufferSize);
	int read(byte *buffer, int bufferSize, int offset, int count);

	unsigned long getEOCInterrupt();
	byte getAnalogPin();

	boolean isRecording;
	int available;
private:
	byte mBuffer[BVSM_BUFFER_SIZE];
	int mWritePos;
	int mReadPos;

	byte mAnalogReference;
	byte mPin;
	unsigned long mEOCInterrupt;
};

#endif