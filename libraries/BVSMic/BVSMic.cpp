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

#include <Arduino.h>
#include <wiring_private.h>
#include <BVSMic.h>

const unsigned int SAMPLE_RATE = 8000;

BVSMic *globalBVSMic = NULL;

BVSMic::BVSMic()
{
	mAnalogReference = 0;
	isRecording = false;
	resetBuffer();
	globalBVSMic = this;
}

void BVSMic::begin()
{
#if defined __SAM3X8E__ || __SAM3X8H__ || __SAM3U4E__
	pmc_enable_periph_clk(ID_TC5);
	TC_Configure(TC1, 2, 
		TC_CMR_TCCLKS_TIMER_CLOCK1 |	// Clock at MCR/2
		TC_CMR_WAVE |
		TC_CMR_WAVSEL_UP_RC);			// Counter running up and reset when equals to RC
	const uint32_t TC = (F_CPU / 2) / SAMPLE_RATE;
	TC_SetRC(TC1, 2, TC);
#else
	byte prescaler = 0x01;
	unsigned long ocr = (F_CPU / (1L * SAMPLE_RATE)) - 1;

	if (ocr > 255)
	{
		prescaler = 0x02;
		ocr = (F_CPU / (8L * SAMPLE_RATE)) - 1;

		if (ocr > 255)
		{
			prescaler = 0x03;
			ocr = (F_CPU / (32L * SAMPLE_RATE)) - 1;

			if (ocr > 255)
			{
				prescaler = 0x04;
				ocr = (F_CPU / (64L * SAMPLE_RATE)) - 1;

				if (ocr > 255)
				{
					prescaler = 0x05;
					ocr = (F_CPU / (128L * SAMPLE_RATE)) - 1;

					if (ocr > 255)
					{
						prescaler = 0x06;
						ocr = (F_CPU / (256L * SAMPLE_RATE)) - 1;

						if (ocr > 255)
						{
							prescaler = 0x07;
							ocr = (F_CPU / (1024L * SAMPLE_RATE)) - 1;
						}
					}
				}
			}
		}
	}
#endif

#if defined(__AVR_ATmega32U4__)
	#if defined(TCCR3B)
		TCCR3B = prescaler;
		sbi(TCCR3B, WGM32);
	#endif
	#if defined(OCR3A)
		OCR3A = ocr;
	#endif
#else
	#if defined(TCCR2A)
		sbi(TCCR2A, WGM21);
	#endif
	#if defined(TCCR2B)
		TCCR2B = prescaler;
	#endif
	#if defined(OCR2A)
		OCR2A = ocr;
	#endif
#endif
}

void BVSMic::setAudioInput(byte pin, byte analogReference)
{
#if defined __SAM3X8E__ || __SAM3X8H__ || __SAM3U4E__
	mPin = pin;
#endif

#if defined(analogPinToChannel)
#if defined(__AVR_ATmega32U4__)
	if (pin >= 18) pin -= 18; // allow for channel or pin numbers
#endif
	pin = analogPinToChannel(pin);
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	if (pin >= 54) pin -= 54; // allow for channel or pin numbers
#elif defined(__AVR_ATmega32U4__)
	if (pin >= 18) pin -= 18; // allow for channel or pin numbers
#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
	if (pin >= 24) pin -= 24; // allow for channel or pin numbers
#else
	if (pin >= 14) pin -= 14; // allow for channel or pin numbers
#endif

#if defined(ADCSRB) && defined(MUX5)
	// the MUX5 bit of ADCSRB selects whether we're reading from channels
	// 0 to 7 (MUX5 low) or 8 to 15 (MUX5 high).
	ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((pin >> 3) & 0x01) << MUX5);
#endif

mAnalogReference = analogReference;

#if defined(ADMUX)
	// set the analog reference (high two bits of ADMUX) and select the
	// channel (low 4 bits) and sets ADLAR to left-adjust
	//the reslt (writes 1 to ADLAR)
	ADMUX = ((mAnalogReference << 6) | 0x20) | (pin & 0x07);
#endif
		
#if defined(ADCSRA)
	//Sets the ADC clock to a faster clock
	//Since an 8-bit resolution is sufficient, it does not affect
	//the ADC accuracy
	sbi(ADCSRA, ADPS2);
	sbi(ADCSRA, ADPS1);  
	cbi(ADCSRA, ADPS0);
#endif
}

void BVSMic::startRecording()
{
#if defined __SAM3X8E__ || __SAM3X8H__ || __SAM3U4E__
	uint32_t ulChannel;

	if (mPin < A0)
		mPin += A0;

	ulChannel = g_APinDescription[mPin].ulADCChannelNumber;
#endif

#if defined __SAM3U4E__
	switch (g_APinDescription[mPin].ulAnalogChannel)
	{
		// Handling ADC 10 bits channels
		case ADC0:
		case ADC1:
		case ADC2:
		case ADC3:
		case ADC4:
		case ADC5:
		case ADC6:
		case ADC7:
			// Enable the corresponding channel
			adc_enable_channel(ADC, (adc_channel_num_t)ulChannel);
			break;

		// Handling ADC 12 bits channels
		case ADC8:
		case ADC9:
		case ADC10:
		case ADC11:
		case ADC12:
		case ADC13:
		case ADC14:
		case ADC15:
			// Enable the corresponding channel
			adc12b_enable_channel(ADC12B, (adc_channel_num_t)ulChannel);
			break;

		// Compiler could yell because we don't handle DAC pins
		default :
			break;
	}
#endif

#if defined __SAM3X8E__ || defined __SAM3X8H__
	static uint32_t latestSelectedChannel = -1;
	switch (g_APinDescription[mPin].ulAnalogChannel)
	{
		// Handling ADC 12 bits channels
		case ADC0:
		case ADC1:
		case ADC2:
		case ADC3:
		case ADC4:
		case ADC5:
		case ADC6:
		case ADC7:
		case ADC8:
		case ADC9:
		case ADC10:
		case ADC11:

			// Enable the corresponding channel
			if (ulChannel != latestSelectedChannel) 
			{
				adc_enable_channel(ADC, (adc_channel_num_t)ulChannel);
				if (latestSelectedChannel != (uint32_t)-1)
					adc_disable_channel(ADC, (adc_channel_num_t)latestSelectedChannel);
				latestSelectedChannel = ulChannel;
			}

			break;

		// Compiler could yell because we don't handle DAC pins
		default :
			break;
	}
#endif

#if defined __SAM3X8E__ || __SAM3X8H__ || __SAM3U4E__
	// Enable ADC interrupt
	NVIC_EnableIRQ(ADC_IRQn);

	// Set ADC interrupt on selected pin
	ADC->ADC_IDR = 0xFFFFFFFF ;   // disable all interrupts

	switch (g_APinDescription[mPin].ulAnalogChannel)
	{
		case ADC0:
			ADC->ADC_IER = ADC_IER_EOC7;
			mEOCInterrupt = ADC_IER_EOC7;
			break;
		case ADC1:
			ADC->ADC_IER = ADC_IER_EOC6;
			mEOCInterrupt = ADC_IER_EOC6;
			break;
		case ADC2:
			ADC->ADC_IER = ADC_IER_EOC5;
			mEOCInterrupt = ADC_IER_EOC5;
			break;
		case ADC3:
			ADC->ADC_IER = ADC_IER_EOC4;
			mEOCInterrupt = ADC_IER_EOC4;
			break;
		case ADC4:
			ADC->ADC_IER = ADC_IER_EOC3;
			mEOCInterrupt = ADC_IER_EOC3;
			break;
		case ADC5:
			ADC->ADC_IER = ADC_IER_EOC2;
			mEOCInterrupt = ADC_IER_EOC2;
			break;
		case ADC6:
			ADC->ADC_IER = ADC_IER_EOC1;
			mEOCInterrupt = ADC_IER_EOC1;
			break;
		case ADC7:
			ADC->ADC_IER = ADC_IER_EOC0;
			mEOCInterrupt = ADC_IER_EOC0;
			break;
		case ADC8:
			ADC->ADC_IER = ADC_IER_EOC10;
			mEOCInterrupt = ADC_IER_EOC10;
			break;
		case ADC9:
			ADC->ADC_IER = ADC_IER_EOC11;
			mEOCInterrupt = ADC_IER_EOC11;
			break;
		case ADC10:
			ADC->ADC_IER = ADC_IER_EOC12;
			mEOCInterrupt = ADC_IER_EOC12;
			break;
		case ADC11:
			ADC->ADC_IER = ADC_IER_EOC13;
			mEOCInterrupt = ADC_IER_EOC13;
			break;
		default :
			break;
	}

	/* turn on the timer clock in the power management controller */
	pmc_set_writeprotect(false);		// disable write protection for pmc registers
	pmc_enable_periph_clk(ID_TC5);		// enable peripheral clock TC7

	// enable timer interrupts on the timer
	TC1->TC_CHANNEL[2].TC_IER=TC_IER_CPCS;   // IER = interrupt enable register
	TC1->TC_CHANNEL[2].TC_IDR=~TC_IER_CPCS;  // IDR = interrupt disable register

	/* Enable the interrupt in the nested vector interrupt controller */
	NVIC_EnableIRQ(TC5_IRQn);

	//Starts the timer
	TC_Start(TC1, 2);
#endif

#if defined(__AVR_ATmega32U4__)
	#if defined(TCNT3)
		TCNT3 = 0x0000;
	#endif
#else
	#if defined(TCNT2)
		TCNT2 = 0x00;
	#endif
#endif

#if defined(ADCSRA)
	//Sets the ADC clock to a faster clock
	//Since an 8-bit resolution is sufficient, it does not affect
	//the ADC accuracy
	sbi(ADCSRA, ADPS2);
	sbi(ADCSRA, ADPS1);  
	cbi(ADCSRA, ADPS0);

	//Enables ADC interrupt
	sbi(ADCSRA, ADIE);
#endif

#if defined(__AVR_ATmega32U4__)
	#if defined(TIMSK3)
		sbi(TIMSK3, OCIE3A);
	#endif
#else
	#if defined(TIMSK2)
		sbi(TIMSK2, OCIE2A);
	#endif
#endif

	isRecording = true;
}

void BVSMic::stopRecording()
{
#if defined __SAM3X8E__ || __SAM3X8H__ || __SAM3U4E__
	uint32_t ulChannel;

	if (mPin < A0)
		mPin += A0;

	ulChannel = g_APinDescription[mPin].ulADCChannelNumber;

	//Stops the timer
	TC_Stop(TC1, 2);

	// Disables timer interrupts on the timer
	TC1->TC_CHANNEL[2].TC_IER=~TC_IER_CPCS;   // IER = interrupt enable register
	TC1->TC_CHANNEL[2].TC_IDR=TC_IER_CPCS;  // IDR = interrupt disable register

	/* Disables the interrupt in the nested vector interrupt controller */
	NVIC_DisableIRQ(TC5_IRQn);

	// Disable ADC interrupt
	NVIC_DisableIRQ(ADC_IRQn);

	// Set ADC interrupts
	ADC->ADC_IDR = 0xFFFFFFFF ;   // disable all interrupts
#endif

#if defined(__AVR_ATmega32U4__)
	#if defined(TIMSK3)
		cbi(TIMSK3, OCIE3A);
	#endif
#else
	#if defined(TIMSK2)
		cbi(TIMSK2, OCIE2A);
	#endif
#endif

#if defined(ADCSRA)
	//Disables ADC interrupt
	cbi(ADCSRA, ADIE);

	sbi(ADCSRA, ADPS2);
	sbi(ADCSRA, ADPS1);
	sbi(ADCSRA, ADPS0);
#endif

	resetBuffer();
	isRecording = false;
}

void BVSMic::resetBuffer()
{
	mWritePos = 0;
	mReadPos = 0;
	available = 0;
}

void BVSMic::write(byte sample)
{
	if (available == BVSM_BUFFER_SIZE)
		return;

	mBuffer[mWritePos] = sample;
	mWritePos++;

	if (mWritePos == BVSM_BUFFER_SIZE)
		mWritePos = 0;

	available++;
}

int BVSMic::read(byte *buffer, int bufferSize)
{
	int bytesToRead = bufferSize > available ? available : bufferSize;

	for (int i = 0; i < bytesToRead; i++)
	{
		buffer[i] = mBuffer[mReadPos];
		mReadPos++;

		if (mReadPos == BVSM_BUFFER_SIZE)
			mReadPos = 0;
	}

	available -= bytesToRead;
	return bytesToRead;
}

int BVSMic::read(byte *buffer, int bufferSize, int offset, int count)
{
	if (count > bufferSize || offset + count > bufferSize)
		return 0;

	int bytesToRead = count > available ? available : count;

	for (int i = 0; i < bytesToRead; i++)
	{
		buffer[i + offset] = mBuffer[mReadPos];
		mReadPos++;

		if (mReadPos == BVSM_BUFFER_SIZE)
			mReadPos = 0;
	}

	available -= bytesToRead;
	return bytesToRead;
}

unsigned long BVSMic::getEOCInterrupt()
{
	return mEOCInterrupt;
}

byte BVSMic::getAnalogPin()
{
	return mPin;
}

#if defined __SAM3X8E__ || __SAM3X8H__ || __SAM3U4E__
void TC5_Handler()
{
	TC_GetStatus(TC1, 2);

	if (globalBVSMic != NULL && globalBVSMic->isRecording)
	{
		// Starts ADC conversion
		adc_start(ADC);
	}
}

void ADC_Handler()
{
	if ((adc_get_status(ADC) & globalBVSMic->getEOCInterrupt()) == globalBVSMic->getEOCInterrupt())
	{
		if (globalBVSMic != NULL && globalBVSMic->isRecording)
		{
			byte analogPin = globalBVSMic->getAnalogPin();
			unsigned long sample = adc_get_channel_value(ADC, 
				(adc_channel_num_t)g_APinDescription[analogPin].ulADCChannelNumber);
			globalBVSMic->write((byte)(sample >> 4));
		}
	}
}
#else

#if defined(__AVR_ATmega32U4__)
	ISR(TIMER3_COMPA_vect)
	{
		if (globalBVSMic != NULL && globalBVSMic->isRecording)
		{
			//Starts ADC conversion  
			sbi(ADCSRA, ADSC);
		}
	}
#else
	ISR(TIMER2_COMPA_vect)
	{
		if (globalBVSMic != NULL && globalBVSMic->isRecording)
		{
			//Starts ADC conversion  
			sbi(ADCSRA, ADSC);
		}
	}
#endif



ISR(ADC_vect) 
{
	if (globalBVSMic != NULL && globalBVSMic->isRecording)
		globalBVSMic->write(ADCH);
}
#endif