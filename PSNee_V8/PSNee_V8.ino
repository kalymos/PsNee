// PSNee V8 - Ultimate PSX unlocker.   /ver. 8.0.1
// Developed by brill & postal2201, based on PSNee V7 open source project.   /Emu-land.net

#define F_CPU 16000000L
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

//------------------------------------------------------------------------------------------------
//                         Select your chip
//------------------------------------------------------------------------------------------------

#define ATmega328_168      // Fuses: JAP_FAT - H: DF, L: EE, E: FF; Other - H: DF, L: FF, E: FF.
/*  PinVCC-3.5v, PinGND-GND, Pin2-BIOS AX, Pin3-BIOS AY, Pin4-BIOS DX, Pin5-Switch*(optional),
    Pin6-SQCK, Pin7-SUBQ, Pin8-DATA, Pin9-GATE_WFCK, PinRST-RESET*(Only for JAP_FAT)  */

//------------------------------------------------------------------------------------------------
//                         Select your console
//------------------------------------------------------------------------------------------------
//                              Attention!
//   If a BIOS checksum is specified, it is more important than the SCPH model number!
//------------------------------------------------------------------------------------------------	

//#define UC_ALL           // Use for all NTSC-U/C models. No BIOS patching needed.
//#define PAL_FAT          // Use for all PAL FAT models. No BIOS patching needed.
//#define SCPH_103         // No BIOS patching needed.
//#define SCPH_102         // DX - D0, AX - A7. BIOS ver. 4.4e, CRC 0BAD7EA9 | 4.5e, CRC 76B880E5
//#define SCPH_100         // DX - D0, AX - A7. BIOS ver. 4.3j, CRC F2AF798B
//#define SCPH_7000_9000   // DX - D0, AX - A7. BIOS ver. 4.0j, CRC EC541CD0
//#define SCPH_5500        // DX - D0, AX - A5. BIOS ver. 3.0j, CRC FF3EEB8C
//#define SCPH_3500_5000   // DX - D0, for 40-pin BIOS: AX - A4, for 32-pin BIOS: AX - A5. BIOS ver. 2.2j, CRC 24FC7E17 | 2.1j, CRC BC190209
//#define SCPH_3000        // DX - D5, for 40-pin BIOS: AX - A6, AY - A7, for 32-pin BIOS: AX - A7, AY - A8. BIOS ver. 1.1j, CRC 3539DEF6
//#define SCPH_1000        // DX - D5, for 40-pin BIOS: AX - A6, AY - A7, for 32-pin BIOS: AX - A7, AY - A8. BIOS ver. 1.0j, CRC 3B601FC8

//------------------------------------------------------------------------------------------------
//                         Options
//------------------------------------------------------------------------------------------------

#define AUTOREGION         // If disable, send all SCEX codes, instead of the code selected model.
#define PATCH_SW           // Enables hardware support for disabling BIOS patching.

//------------------------------------------------------------------------------------------------
//                         Code section
//------------------------------------------------------------------------------------------------

#include "config.h"
#include "settings.h"
#include "patching.h"

volatile uint8_t count_isr = 0;
volatile uint32_t microsec = 0;
volatile uint16_t millisec = 0;

bool wfck_mode;

const int16_t delay_between_bits = 4000; 
const int16_t delay_between_injections = 90;

uint8_t readBit(uint8_t index, const uint8_t * ByteSet)
{
	return !!(ByteSet[index / 8] & (1 << (index % 8)));
}

ISR(TIMER0_COMPA_vect)
{
	microsec += 10;
	count_isr++;
	if (count_isr == 100)
	{
		millisec++;
		count_isr = 0;
	}
}

void Timer_Start()
{
	TIMER_TCNT_CLEAR;
	TIMER_TIFR_CLEAR;
	TIMER_INTERRUPT_ENABLE;
}

void Timer_Stop()
{
	TIMER_INTERRUPT_DISABLE;
	TIMER_TCNT_CLEAR;
	count_isr = 0;
	microsec = 0;
	millisec = 0;
}

void inject_SCEX(const char region)
{
	static const uint8_t SCEEData[] = {
		0b01011001,
		0b11001001,
		0b01001011,
		0b01011101,
		0b11101010,
		0b00000010
	};
	
	static const uint8_t SCEAData[] = {
		0b01011001,
		0b11001001,
		0b01001011,
		0b01011101,
		0b11111010,
		0b00000010
	};
	
	static const uint8_t SCEIData[] = {
		0b01011001,
		0b11001001,
		0b01001011,
		0b01011101,
		0b11011010,
		0b00000010
	};

	uint8_t bit_counter;
	for (bit_counter = 0; bit_counter < 44; bit_counter++)
	{
		if (readBit(bit_counter, region == 'e' ? SCEEData : region == 'a' ? SCEAData : SCEIData) == 0)
		{
			DATA_OUTPUT;
			DATA_CLEAR;
			_delay_us(delay_between_bits);
		}

		else
		{
			if (wfck_mode)
			{
				DATA_OUTPUT;
				Timer_Start();
			 do
			 {
				if(WFCK_READ)
				{
					DATA_SET;
				}

				else
				{
					DATA_CLEAR;
				}
			 }

			while (microsec < delay_between_bits);
			Timer_Stop();
			}

			else
			{
				DATA_INPUT;
				_delay_us(delay_between_bits);
			}
		}
	}

	DATA_OUTPUT;
	DATA_CLEAR;
	_delay_ms(delay_between_injections);
}

int main()
{
	uint8_t hysteresis = 0;
	uint8_t scbuf[12] = {0};
	uint16_t timeout_clock_counter = 0;
	uint8_t bitbuf = 0;
	uint8_t bitpos = 0;
	uint8_t scpos = 0;
	uint16_t highs = 0, lows = 0;

	#if !defined(UC_ALL) && !defined(PAL_FAT) && !defined(SCPH_103) && \
      !defined(SCPH_102) && !defined(SCPH_100) && !defined(SCPH_7000_9000) && \
      !defined(SCPH_5500) && !defined(SCPH_3500_5000) && !defined(SCPH_3000) && \
      !defined(SCPH_1000)
     #error "Console not selected! Please uncoment #define with SCPH model number."
  	#elif !(defined(UC_ALL) ^ defined(PAL_FAT) ^ defined(SCPH_103) ^ \
          defined(SCPH_102) ^ defined(SCPH_100) ^ defined(SCPH_7000_9000) ^ \
          defined(SCPH_5500) ^ defined(SCPH_3500_5000) ^ defined(SCPH_3000) ^ \
          defined(SCPH_1000))
     #error "May be selected only one console! Please check #define with SCPH model number."
  	#endif
 
	#ifndef AUTOREGION
	 const char region[3] = {'e', 'a', 'i'};
	#endif

	Init();

	#ifdef LED_USE
	 LED_ON;
	#endif

	#if defined(BIOS_PATCH) && !defined(PATCH_SW)
	 Bios_Patching();
	#elif defined(BIOS_PATCH) && defined(PATCH_SW)
	 if (SW_CHECK != 0)
	 {
	 	Bios_Patching();
	 }
	 else
	 {
	 while (SQCK_READ == 0);
	 while (WFCK_READ == 0); 
	 }
	#else
	 while (SQCK_READ == 0);
	 while (WFCK_READ == 0);
	#endif

	Timer_Start();
	
	do
	{
		if (WFCK_READ == 1) highs++;
		if (WFCK_READ == 0) lows++;
		_delay_us(200);
	}
	while (millisec < 1000);

	Timer_Stop();

	if (lows > 100)
	{
		wfck_mode = 1;
	}

	else
	{
		wfck_mode = 0;
	}

	#ifdef LED_USE
	 LED_OFF;
	#endif

	while(1)
	{
		_delay_ms(1);
		cli();
		do
		{
			for (bitpos = 0; bitpos < 8; bitpos++)
			{
				while (SQCK_READ != 0)
				{
					timeout_clock_counter++;
					if (timeout_clock_counter > 1000)
					{
						scpos = 0;
						timeout_clock_counter = 0;
						bitbuf = 0;
						bitpos = 0;
						continue;
					}
				}
				
				while (SQCK_READ == 0);
				
				if(SUBQ_READ)
				{
					bitbuf |= 1 << bitpos;
				}
				
				timeout_clock_counter = 0;
			}

			scbuf[scpos] = bitbuf;
			scpos++;
			bitbuf = 0;
		}

		while (scpos < 12);
		sei();

		uint8_t isDataSector = (((scbuf[0] & 0x40) == 0x40) && (((scbuf[0] & 0x10) == 0) && ((scbuf[0] & 0x80) == 0)));

		if (
		(isDataSector && scbuf[1] == 0x00 && scbuf[6] == 0x00) && // [0] = 41 means psx game disk. the other 2 checks are garbage protection
		(scbuf[2] == 0xA0 || scbuf[2] == 0xA1 || scbuf[2] == 0xA2 || // if [2] = A0, A1, A2 ..
		(scbuf[2] == 0x01 && (scbuf[3] >= 0x98 || scbuf[3] <= 0x02))) // .. or = 01 but then [3] is either > 98 or < 02
		)
		{
			hysteresis++;
		}

		else if (hysteresis > 0 && ((scbuf[0] == 0x01 || isDataSector) && (scbuf[1] == 0x00 /*|| scbuf[1] == 0x01*/ ) && scbuf[6] == 0x00))
		{
			hysteresis++;
		}

		else if (hysteresis > 0)
		{
			hysteresis--; 
		}
		
		if (hysteresis >= 14)
		{
			hysteresis = 11;
			
			#ifdef LED_USE
			 LED_ON;
			#endif

			DATA_OUTPUT;
			DATA_CLEAR;

			if (!wfck_mode)
			{
				WFCK_OUTPUT;
				WFCK_CLEAR;
			}
			
			_delay_ms(delay_between_injections);

			uint8_t scex;
			for (scex = 0; scex < 2; scex++)
			{
				inject_SCEX(region[scex]);
			}

			if (!wfck_mode)
			{
				WFCK_INPUT; 
			}

			DATA_INPUT;

			#ifdef LED_USE
			 LED_OFF;
			#endif
		}
	}
}
