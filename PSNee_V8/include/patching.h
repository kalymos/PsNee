#pragma once

#ifdef BIOS_PATCH

void Timer_Start(void);
void Timer_Stop(void);

extern volatile uint8_t count_isr;
extern volatile uint32_t microsec;
extern volatile uint16_t millisec;

volatile uint8_t impulse = 0;
volatile uint8_t patch = 0;

ISR(INT0_vect)
{
	impulse++;
	if (impulse == TRIGGER)
	{
		HOLD;
		#ifdef HIGH_PATCH
		 DX_SET;
		#endif
		DX_OUTPUT;
		PATCHING;
		#ifdef HIGH_PATCH
		 DX_CLEAR;
		#endif
		DX_INPUT;
		AX_INTERRUPT_DISABLE;

		impulse = 0;
		patch = 1;
	}
}

#ifdef DOUBLE_PATCH

ISR(INT1_vect)
{
	impulse++;
	if (impulse == TRIGGER2)
	{
		HOLD2;
		DX_OUTPUT;
		PATCHING2;
		DX_INPUT;
		AY_INTERRUPT_DISABLE;

		patch = 2;
	}
}

#endif

void Bios_Patching()
{
	#ifdef LOW_TRIGGER
	 AX_INTERRUPT_FALLING;
	#else
	 AX_INTERRUPT_RISING;
	#endif

	if (AX_READ != 0)
	{
		while (AX_READ != 0);
		while (AX_READ == 0);
	}

	else
	{
		while (AX_READ == 0);
	}
	
	Timer_Start();

	while (microsec < CHECKPOINT);
	Timer_Stop();
	AX_INTERRUPT_ENABLE;
	
	while (patch != 1);

	#ifdef DOUBLE_PATCH

	 #ifdef LOW_TRIGGER2
	  AY_INTERRUPT_FALLING;
	 #else
	  AY_INTERRUPT_RISING;
	 #endif

	while (AY_READ != 0);
	Timer_Start();

	while (microsec < CHECKPOINT2);
	Timer_Stop();
	AY_INTERRUPT_ENABLE;
	
	while (patch != 2);

	#endif

}

#endif