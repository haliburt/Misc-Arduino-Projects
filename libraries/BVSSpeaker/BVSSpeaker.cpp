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

#include <BVSSpeaker.h>

const int SAMPLE_RATE = 8000;
const unsigned long TAG = 0x10000000;

void BVSSpeaker::begin(DACClass &dac)
{
	mQueueCount = 0;
	mQueueWritePos = 0;
	mQueueReadPos = 0;

	mIsPlaying = false;
	mFinishPlaying = false;
	mDac = &dac;
	mDacBufferWritePos = 0;
	mCurrDacBuffer = mDacBuffer0;
}

void BVSSpeaker::startDac()
{
	mDac->begin(VARIANT_MCK / (SAMPLE_RATE * 2));
}

void BVSSpeaker::stopDac()
{
	mDac->end();
}

void BVSSpeaker::play()
{
	if (mIsPlaying)
	{
		if (mQueueCount == 0 && !mDac->isPlaying())
		{
			mIsPlaying = false;
			stopDac();
		}
		else
			playQueue();
	}
	else
	{
		if (mQueueCount != 0)
		{
			startDac();
			mIsPlaying = true;
			playQueue();
		}
	}
}

void BVSSpeaker::playQueue()
{
	if (mDac->canQueue() && mQueueCount != 0)
	{
		int bytesToRead;

		if (mQueueCount < BVSSPEAKER_DAC_BUFFER_SIZE - mDacBufferWritePos)
			bytesToRead = mQueueCount;
		else
			bytesToRead = BVSSPEAKER_DAC_BUFFER_SIZE - mDacBufferWritePos;

		for (int i = 0; i < bytesToRead; i++)
		{
			mCurrDacBuffer[mDacBufferWritePos] = prepareSample(mQueueBuffer[mQueueReadPos]);
			mDacBufferWritePos++;
			mQueueReadPos++;

			if (mQueueReadPos == BVSSPEAKER_QUEUE_SIZE)
				mQueueReadPos = 0;
		}

		mQueueCount -= bytesToRead;
	}

	if (mDac->canQueue() && mDacBufferWritePos != 0)
	{
		if (mDacBufferWritePos == BVSSPEAKER_DAC_BUFFER_SIZE || mFinishPlaying)
		{
			if (mCurrDacBuffer == mDacBuffer0)
			{
				mDac->queueBuffer(mDacBuffer0, mDacBufferWritePos);
				mCurrDacBuffer = mDacBuffer1;
			}
			else
			{
				mDac->queueBuffer(mDacBuffer1, mDacBufferWritePos);
				mCurrDacBuffer = mDacBuffer0;
			}

			mDacBufferWritePos = 0;

			if (mFinishPlaying && mQueueCount == 0)
				mFinishPlaying = false;
		}
	}
}

unsigned long BVSSpeaker::prepareSample(byte sample)
{
	short sSample = (short)sample;

	if (sSample >= 128)
	{
		sSample -= 127;
		sSample *= 16;
		sSample += 2047;
	}
	else
	{
		sSample -= 128;
		sSample *= 16;
		sSample += 2048;
	}

	return ((unsigned long)sSample) | TAG;
}

void BVSSpeaker::finishPlaying()
{
	mFinishPlaying = true;
}

boolean BVSSpeaker::isPlaying()
{
	return mIsPlaying;
}

int BVSSpeaker::enqueue(byte *data, int count)
{
	int enqueued = count > BVSSPEAKER_QUEUE_SIZE - mQueueCount ? 
		BVSSPEAKER_QUEUE_SIZE - mQueueCount : count;

	for (int i = 0; i < enqueued; i++)
	{
		mQueueBuffer[mQueueWritePos] = data[i];
		mQueueWritePos++;

		if (mQueueWritePos == BVSSPEAKER_QUEUE_SIZE)
			mQueueWritePos = 0;
	}

	mQueueCount += enqueued;
	return enqueued;
}

int BVSSpeaker::getQueueCount()
{
	return mQueueCount;
}