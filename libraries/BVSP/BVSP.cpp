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

#include <Arduino.h>
#include <BVSP.h>

const int MAX_PAYLOAD_SIZE = 1023;

const byte START_OF_FRAME = 0xAB;
const byte END_OF_FRAME = 0x57;
const byte STATUS_REQUEST_BYTE_1 = 0x44;
const byte STATUS_REQUEST_BYTE_2 = 0x01;
const byte FRAME_TYPE_DATA_TRANSFER = 0x00;
const byte FRAME_TYPE_MODE_CHANGE = 0x02;
const byte MODE_CHANGE_FRAME[] = { START_OF_FRAME, 0x80, 0x00, END_OF_FRAME };
const byte MODE_CHANGE_FRAME_SIZE = 4;
const byte MODE_CHANGE_SIGNAL[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF };
const byte MODE_CHANGE_SIGNAL_SIZE = 9;

BVSP::BVSP()
{
	reset();
	mSerial = NULL;
	mStatusRequestTimeout = 0;
	modeChanged = NULL;
	frameReceived = NULL;
	streamReceived = NULL;
}

void BVSP::begin(Stream &serialPort, unsigned long statusRequestTimeout, 
	unsigned long statusRequestInterval)
{
	mStatusRequestTimeout = statusRequestTimeout;
	mStatusRequestInterval = statusRequestInterval;
	mSerial = &serialPort;
	mStatusRequestTime = millis();
}

void BVSP::setInboundMode(byte mode)
{
	switch (mode)
	{
		case FRAMED_MODE:
			mSerial->write(MODE_CHANGE_SIGNAL, MODE_CHANGE_SIGNAL_SIZE);
			inboundMode = FRAMED_MODE;
			break;
		case STREAM_MODE:
			mSerial->write(MODE_CHANGE_FRAME, MODE_CHANGE_FRAME_SIZE);
			inboundMode = STREAM_MODE;
			break;
	}
}

void BVSP::reset()
{
	inboundMode = FRAMED_MODE;
	outboundMode = FRAMED_MODE;

	mBVSStatus = false;
	mSREStatus = false;
	mDataFwdStatus = false;
	mAleatoryNumber = 0;
	mStatusRequestTime = millis();
	mStatusTimedOut = false;
	mStatusReceived = true;

	isReceiving = false;
	usedMemory = 0;
	mWritePos = 0;
	mReadPos = 0;
	mCurrFrameByte = 0;
	mFrameStart = 0;
	mFrameType = 0;
	mDataType = 0;
	mPayloadSize = 0;
	mPayloadPos = 0;
	mModeChangeSignalPos = 0;
	mCurrStreamSize = 0;
}

void BVSP::requestStatus()
{
	mStatusTimedOut = false;
	mStatusReceived = false;
	mAleatoryNumber = (byte)random(255);
	mSerial->write(START_OF_FRAME);
	mSerial->write(STATUS_REQUEST_BYTE_1);
	mSerial->write(STATUS_REQUEST_BYTE_2);
	mSerial->write(mAleatoryNumber);
	mSerial->write(END_OF_FRAME);
	mStatusRequestTime = millis();
}

void BVSP::keepAlive()
{
	if (millis() - mStatusRequestTime > mStatusRequestInterval)
	{
		if (inboundMode == STREAM_MODE)
		{
			setInboundMode(FRAMED_MODE);
			requestStatus();
			setInboundMode(STREAM_MODE);
		}
		else
		{
			requestStatus();
		}
	}
}

void BVSP::updateStatus(unsigned long receiveTime)
{
	if (mDataType != DATA_TYPE_STATUS)
		return;

	int readPos = getDataStart();
	byte aleatoryNumber = mBuffer[readPos];
	readPos++;
	byte status = readPos >= BVSP_BUFFER_SIZE ? mBuffer[0] : mBuffer[readPos];

	if (aleatoryNumber == mAleatoryNumber)
	{
		if (receiveTime - mStatusRequestTime > mStatusRequestTimeout)
		{
			mStatusReceived = true;
			mStatusTimedOut = true;
		}
		else
		{
			mStatusReceived = true;
			mStatusTimedOut = false;
			mBVSStatus = status & 0x01; 
			mSREStatus = status & 0x02;
			mDataFwdStatus = status & 0x04;
			return;
		}
	}
	else
	{
		if (receiveTime - mStatusRequestTime > mStatusRequestTimeout)
		{
			mStatusReceived = false;
			mStatusTimedOut = true;
		}
		else
		{
			mStatusReceived = false;
			mStatusTimedOut = false;
		}
	}

	mBVSStatus = false;
	mSREStatus = false;
	mDataFwdStatus = false;
}

boolean BVSP::hasStatusTimedOut()
{
	if (mStatusTimedOut == false && mStatusReceived == false)
	{
		if (millis() - mStatusRequestTime > mStatusRequestTimeout)
		{
			mStatusTimedOut = true;
			mBVSStatus = false;
			mSREStatus = false;
			mDataFwdStatus = false;
		}
	}

	return mStatusTimedOut;
}

boolean BVSP::isBVSRunning()
{
	if (mStatusTimedOut == false && mStatusReceived == false)
	{
		if (millis() - mStatusRequestTime > mStatusRequestTimeout)
		{
			mStatusTimedOut = true;
			mBVSStatus = false;
			mSREStatus = false;
			mDataFwdStatus = false;
			return mBVSStatus;
		}
		else
			return mBVSStatus;
	}
	else
		return mBVSStatus;
}

boolean BVSP::isSREAvailable()
{
	if (mStatusTimedOut == false && mStatusReceived == false)
	{
		if (millis() - mStatusRequestTime > mStatusRequestTimeout)
		{
			mStatusTimedOut = true;
			mBVSStatus = false;
			mSREStatus = false;
			mDataFwdStatus = false;
			return mSREStatus;
		}
		else
			return mSREStatus;
	}
	else
		return mSREStatus;
}

boolean BVSP::isDataFwdRunning()
{
	if (mStatusTimedOut == false && mStatusReceived == false)
	{
		if (millis() - mStatusRequestTime > mStatusRequestTimeout)
		{
			mStatusTimedOut = true;
			mBVSStatus = false;
			mSREStatus = false;
			mDataFwdStatus = false;
			return mDataFwdStatus;
		}
		else
			return mDataFwdStatus;
	}
	else
		return mDataFwdStatus;
}

byte BVSP::buildByteOne(byte dataType, int payloadSize)
{
	byte byteOne = 0x00;
    byteOne |= dataType;
    byteOne <<= 2;
    payloadSize >>= 8;
    byteOne |= (byte)payloadSize;
    return byteOne;
}

void BVSP::send(byte data)
{
	mSerial->write(START_OF_FRAME);
	mSerial->write(buildByteOne(DATA_TYPE_BYTE, 0x01));
	mSerial->write(0x01);
	mSerial->write(data);
	mSerial->write(END_OF_FRAME);
}

void BVSP::send(int data)
{
	mSerial->write(START_OF_FRAME);
	mSerial->write(buildByteOne(DATA_TYPE_INT16, 0x02));
	mSerial->write(0x02);
	mSerial->write((byte)(data & 0xFF));
	mSerial->write((byte)((data >> 8) & 0xFF));
	mSerial->write(END_OF_FRAME);
}

void BVSP::send(long data)
{
	mSerial->write(START_OF_FRAME);
	mSerial->write(buildByteOne(DATA_TYPE_INT32, 0x04));
	mSerial->write(0x04);
	mSerial->write((byte)(data & 0xFF));
	mSerial->write((byte)((data >>= 8) & 0xFF));
	mSerial->write((byte)((data >>= 8) & 0xFF));
	mSerial->write((byte)((data >>= 8) & 0xFF));
	mSerial->write(END_OF_FRAME);
}

void BVSP::send(byte *data, int count)
{
	if (count > MAX_PAYLOAD_SIZE)
		count = MAX_PAYLOAD_SIZE;

	mSerial->write(START_OF_FRAME);
	mSerial->write(buildByteOne(DATA_TYPE_BINARY, count));
	mSerial->write((byte)count);
	mSerial->write(data, count);
	mSerial->write(END_OF_FRAME);
}

void BVSP::send(byte *data, int offset, int count)
{
	if (count > MAX_PAYLOAD_SIZE)
		count = MAX_PAYLOAD_SIZE;

	mSerial->write(START_OF_FRAME);
	mSerial->write(buildByteOne(DATA_TYPE_BINARY, count));
	mSerial->write((byte)count);

	for (int i = 0; i < count; i++)
		mSerial->write(data[offset + i]);

	mSerial->write(END_OF_FRAME);
}

void BVSP::send(char *data)
{
	int stringLength = strlen(data);

	if (stringLength > MAX_PAYLOAD_SIZE)
		stringLength = MAX_PAYLOAD_SIZE;

	mSerial->write(START_OF_FRAME);
	mSerial->write(buildByteOne(DATA_TYPE_STRING, stringLength));
	mSerial->write((byte)stringLength);

	for (int i = 0; i < stringLength; i++)
		mSerial->write(data[i]);

	mSerial->write(END_OF_FRAME);
}

void BVSP::sendStream(byte *data, int count)
{
	mSerial->write(data, count);
}

void BVSP::receive()
{
	if (isReceiving || usedMemory == BVSP_BUFFER_SIZE)
		return;

	int available = 0;
	isReceiving = true;

	while (mSerial->available())
	{
		mBuffer[mWritePos] = mSerial->read();
		mWritePos++;

		if (mWritePos == BVSP_BUFFER_SIZE)
			mWritePos = 0;

		available++;
		usedMemory++;

		if (usedMemory == BVSP_BUFFER_SIZE)
			break;
	}

	if (available == 0)
	{
		isReceiving = false;
		return;
	}

	for (int i = 0; i < available; i++)
	{
		if (outboundMode == FRAMED_MODE)
		{
			if (mCurrFrameByte == 0)
			{
				if (mBuffer[mReadPos] == START_OF_FRAME)
				{
					mFrameStart = mReadPos;
					mCurrFrameByte = 1;
					mReadPos++;

					if (mReadPos == BVSP_BUFFER_SIZE)
						mReadPos = 0;
				}
				else
				{
					mReadPos++;

					if (mReadPos == BVSP_BUFFER_SIZE)
						mReadPos = 0;

					usedMemory--;
				}
			}
			else if (mCurrFrameByte == 1)
			{
				if (readByteOne())
				{
					mCurrFrameByte = 2;
					mReadPos++;

					if (mReadPos == BVSP_BUFFER_SIZE)
						mReadPos = 0;
				}
				else
				{
					i = -1;
					available = mReadPos - mFrameStart;

					if (available < 0)
						available = (available + BVSP_BUFFER_SIZE);

					mReadPos = mFrameStart + 1;

					if (mReadPos == BVSP_BUFFER_SIZE)
						mReadPos = 0;

					usedMemory--;
					resetFrame(); 
				}
			}
			else if (mCurrFrameByte == 2)
			{
				if (readByteTwo())
				{
					if (mFrameType == FRAME_TYPE_DATA_TRANSFER)
						mCurrFrameByte = 3;
					else
						mCurrFrameByte = 4;

					mReadPos++;

					if (mReadPos == BVSP_BUFFER_SIZE)
						mReadPos = 0;
				}
				else
				{
					i = -1;
					available = mReadPos - mFrameStart;

					if (available < 0)
						available = (available + BVSP_BUFFER_SIZE);

					mReadPos = mFrameStart + 1;

					if (mReadPos == BVSP_BUFFER_SIZE)
						mReadPos = 0;

					usedMemory--;
					resetFrame();
				}
			}
			else if (mCurrFrameByte == 3)
			{
				mPayloadPos++;

				if (mPayloadPos == mPayloadSize)
					mCurrFrameByte = 4;

				mReadPos++;

				if (mReadPos == BVSP_BUFFER_SIZE)
					mReadPos = 0;
			}
			else if (mCurrFrameByte == 4)
			{
				if (mBuffer[mReadPos] == END_OF_FRAME)
				{
					if (mFrameType == FRAME_TYPE_MODE_CHANGE)
					{
						mReadPos++;

						if (mReadPos == BVSP_BUFFER_SIZE)
							mReadPos = 0;

						resetFrame();
						usedMemory -= 4;
						outboundMode = STREAM_MODE;

						if (modeChanged != NULL)
							(*modeChanged)();
					}
					else
					{
						if (mDataType == DATA_TYPE_STATUS)
							updateStatus(millis());
						else
							if (frameReceived != NULL)
								(*frameReceived)(mDataType, mPayloadSize);

						usedMemory -= 4 + mPayloadSize;
						resetFrame();
						mReadPos++;

						if (mReadPos == BVSP_BUFFER_SIZE)
							mReadPos = 0;
					}
				}
				else
				{
					i = -1;
					available = mReadPos - mFrameStart;

					if (available < 0)
						available = (available + BVSP_BUFFER_SIZE);

					mReadPos = mFrameStart + 1;

					if (mReadPos == BVSP_BUFFER_SIZE)
						mReadPos = 0;

					usedMemory--;
					resetFrame();
				}
			}
		}
		else
		{
			if (mBuffer[mReadPos] == MODE_CHANGE_SIGNAL[mModeChangeSignalPos])
			{
				mModeChangeSignalPos++;

				if (mModeChangeSignalPos == MODE_CHANGE_SIGNAL_SIZE)
				{
					if (mCurrStreamSize != 0)
					{
						if (streamReceived != NULL)
							(*streamReceived)(mCurrStreamSize);

						usedMemory -= mCurrStreamSize;
						mCurrStreamSize = 0;
					}

					mModeChangeSignalPos = 0;
					usedMemory -= MODE_CHANGE_SIGNAL_SIZE;
					outboundMode = FRAMED_MODE;
					mReadPos++;

						if (mReadPos == BVSP_BUFFER_SIZE)
							mReadPos = 0;

					if (modeChanged != NULL)
						(*modeChanged)();
				}
				else
				{
					mReadPos++;

					if (mReadPos == BVSP_BUFFER_SIZE)
						mReadPos = 0;
				}
			}
			else
			{
				if (mModeChangeSignalPos != 0)
				{
					mCurrStreamSize = mModeChangeSignalPos + 1;
					mModeChangeSignalPos = 0;
				}
				else
					mCurrStreamSize++;

				mReadPos++;

				if (mReadPos == BVSP_BUFFER_SIZE)
					mReadPos = 0;
			}
		}
	}

	if (outboundMode == STREAM_MODE && mCurrStreamSize != 0 && mModeChangeSignalPos == 0)
	{
		if (streamReceived != NULL)
			(*streamReceived)(mCurrStreamSize);

		usedMemory -= mCurrStreamSize;
		mCurrStreamSize = 0;
	}

	isReceiving = false;
}

bool BVSP::readByteOne()
{
	mFrameType = mBuffer[mReadPos] >> 6;
	mDataType = (mBuffer[mReadPos] & 0x3C) >> 2;
	mPayloadSize = mBuffer[mReadPos] & 0x03;

	if (mFrameType != FRAME_TYPE_DATA_TRANSFER &&
        mFrameType != FRAME_TYPE_MODE_CHANGE)
		return false;
	else
	{
		if (mFrameType == FRAME_TYPE_MODE_CHANGE)
        {
            if (mDataType != DATA_TYPE_NA ||
                mPayloadSize != 0x00)
                return false;
        }
		else
		{
			if (mDataType != DATA_TYPE_BYTE &&
                mDataType != DATA_TYPE_INT16 &&
                mDataType != DATA_TYPE_INT32 &&
                mDataType != DATA_TYPE_BINARY &&
                mDataType != DATA_TYPE_STRING &&
				mDataType != DATA_TYPE_STATUS)
				return false;
		}
	}

	return true;
}

bool BVSP::readByteTwo()
{
	mPayloadSize = (mPayloadSize << 8) | mBuffer[mReadPos];

	if (mFrameType == FRAME_TYPE_MODE_CHANGE
		&& mPayloadSize != 0)
		return false;
	else
	{
		if (mDataType == DATA_TYPE_BYTE && mPayloadSize != 1)
            return false;
        else if (mDataType == DATA_TYPE_INT16 && mPayloadSize != 2)
            return false;
        else if (mDataType == DATA_TYPE_INT32 && mPayloadSize != 4)
            return false;
	}

    return true;
}

void BVSP::resetFrame()
{
	mCurrFrameByte = 0;
	mFrameType = 0;
	mDataType = 0;
	mPayloadSize = 0;
	mPayloadPos = 0;
}

byte BVSP::getReceivedByte()
{
	if (mDataType != DATA_TYPE_BYTE)
		return 0;

	return mBuffer[getDataStart()];
}

int BVSP::getReceivedInt16()
{
	if (mDataType != DATA_TYPE_INT16)
		return 0;

	int readPos = getDataStart();

	if (readPos + 1 >= BVSP_BUFFER_SIZE)
		return mBuffer[readPos] | (mBuffer[0] << 8);
	else
		return mBuffer[readPos] | (mBuffer[readPos + 1] << 8);
}

long BVSP::getReceivedInt32()
{
	if (mDataType != DATA_TYPE_INT32)
		return 0;

	int readPos = getDataStart() + 3;

	if (readPos >= BVSP_BUFFER_SIZE)
		readPos = readPos - BVSP_BUFFER_SIZE;

	long result = 0;

	for (int i = 0; i < 3; i++)
	{
		result = result | mBuffer[readPos];
		result <<= 8;
		readPos--;

		if (readPos == 0)
			readPos = BVSP_BUFFER_SIZE - 1;
	}

	return result | mBuffer[readPos];
}

int BVSP::getReceivedBytes(byte *buffer, int bufferSize)
{
	if (mDataType != DATA_TYPE_BINARY)
		return 0;

	int count = bufferSize > mPayloadSize ? mPayloadSize : bufferSize;
	int readPos = getDataStart();

	for (int i = 0; i < count; i++)
	{
		buffer[i] = mBuffer[readPos];
		readPos++;

		if (readPos == BVSP_BUFFER_SIZE)
			readPos = 0;
	}

	return count;
}

int BVSP::getReceivedBytes(byte *buffer, int bufferSize, int offset, int count)
{
	if (mDataType != DATA_TYPE_BINARY ||
		offset + count > bufferSize)
		return 0;

	if (count > mPayloadSize)
		count = mPayloadSize;

	int readPos = getDataStart();

	for (int i = 0; i < count; i++)
	{
		buffer[offset + i] = mBuffer[readPos];
		readPos++;

		if (readPos == BVSP_BUFFER_SIZE)
			readPos = 0;
	}

	return count;
}

int BVSP::getReceivedString(char *buffer, int bufferSize)
{
	if (mDataType != DATA_TYPE_STRING)
		return 0;

	int count = bufferSize < (mPayloadSize + 1) ? (bufferSize - 1) : mPayloadSize;
	int readPos = getDataStart();

	for (int i = 0; i < count; i++)
	{
		buffer[i] = mBuffer[readPos];
		readPos++;

		if (readPos == BVSP_BUFFER_SIZE)
			readPos = 0;
	}

	buffer[count] = 0;
	return count;
}

int BVSP::getReceivedStream(byte *buffer, int bufferSize)
{
	if (mCurrStreamSize == 0)
		return 0;

	int count = bufferSize > mCurrStreamSize ? mCurrStreamSize : bufferSize;
	int readPos = getStreamStart();

	for (int i = 0; i < count; i++)
	{
		buffer[i] = mBuffer[readPos];
		readPos++;

		if (readPos == BVSP_BUFFER_SIZE)
			readPos = 0;
	}

	return count;
}

int BVSP::getReceivedStream(short *buffer, int bufferSize)
{
	if (mCurrStreamSize == 0)
		return 0;

	int count = bufferSize > mCurrStreamSize ? mCurrStreamSize : bufferSize;
	int readPos = getStreamStart();

	for (int i = 0; i < count; i++)
	{
		buffer[i] = (short)mBuffer[readPos];
		readPos++;

		if (readPos == BVSP_BUFFER_SIZE)
			readPos = 0;
	}

	return count;
}


int BVSP::getReceivedStream(byte *buffer, int bufferSize, int offset, int count)
{
	if (mCurrStreamSize == 0 ||
		offset + count > bufferSize)
		return 0;

	if (count > mCurrStreamSize)
		count = mCurrStreamSize;

	int readPos = getStreamStart();

	for (int i = 0; i < count; i++)
	{
		buffer[offset + i] = mBuffer[readPos];
		readPos++;

		if (readPos == BVSP_BUFFER_SIZE)
			readPos = 0;
	}

	return count;
}

int BVSP::getReceivedStream(short *buffer, int bufferSize, int offset, int count)
{
	if (mCurrStreamSize == 0 ||
		offset + count > bufferSize)
		return 0;

	if (count > mCurrStreamSize)
		count = mCurrStreamSize;

	int readPos = getStreamStart();

	for (int i = 0; i < count; i++)
	{
		buffer[offset + i] = (short)mBuffer[readPos];
		readPos++;

		if (readPos == BVSP_BUFFER_SIZE)
			readPos = 0;
	}

	return count;
}

int BVSP::getDataStart()
{
	int dataStart = mFrameStart + 3;

	if (dataStart >= BVSP_BUFFER_SIZE)
		dataStart -= BVSP_BUFFER_SIZE;

	return dataStart;
}

int BVSP::getStreamStart()
{
	int streamStart = mReadPos - mCurrStreamSize;

	if (streamStart < 0)
		streamStart += BVSP_BUFFER_SIZE;

	return streamStart;
}