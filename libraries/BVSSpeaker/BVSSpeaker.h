/*
	Copyright (c) 2015 BitSophia Tecnologia Ltda - ME.
	Copyright (c) 2012 by Cristian Maglie <c.maglie@arduino.cc>
	Copyright (c) 2012 Arduino LLC. All right reserved.

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

#ifndef BVSSpeaker_h
#define BVSSpeaker_h

#include <Arduino.h>
#include <DAC.h>

#define BVSSPEAKER_QUEUE_SIZE 8192
#define BVSSPEAKER_DAC_BUFFER_SIZE 512

class BVSSpeaker
{
public:
	void begin(DACClass &dac);
	int enqueue(byte *data, int count);
	int getQueueCount();

	void play();
	boolean isPlaying();
	void finishPlaying();
private:
	byte mQueueBuffer[BVSSPEAKER_QUEUE_SIZE];
	int volatile mQueueCount;
	int mQueueWritePos;
	int mQueueReadPos;

	void startDac();
	void stopDac();
	void playQueue();
	unsigned long prepareSample(byte sample);
	boolean mIsPlaying;
	boolean mFinishPlaying;
	DACClass *mDac;
	unsigned long mDacBuffer0[BVSSPEAKER_DAC_BUFFER_SIZE];
	unsigned long mDacBuffer1[BVSSPEAKER_DAC_BUFFER_SIZE];
	int mDacBufferWritePos;
	unsigned long *mCurrDacBuffer;
};

#endif