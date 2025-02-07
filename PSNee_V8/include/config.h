#pragma once

#ifdef ATmega328_168

#define DATA_OUTPUT     DDRB |= (1<<0)
#define DATA_INPUT      DDRB &= ~(1<<0)
#define DATA_CLEAR      PORTB &= ~(1<<0)
#define DATA_SET        PORTB |= (1<<0)
#define WFCK_OUTPUT     DDRB |= (1<<1)
#define WFCK_INPUT      DDRB &= ~(1<<1)
#define WFCK_CLEAR      PORTB &= ~(1<<1)
#define DX_OUTPUT       DDRD |= (1<<4)
#define DX_INPUT        DDRD &= ~(1<<4)
#define DX_CLEAR        PORTD &= ~(1<<4)
#define DX_SET          PORTD |= (1<<4)

#define SQCK_READ       (PIND & (1<<6))
#define SUBQ_READ       (PIND & (1<<7))
#define WFCK_READ       (PINB & (1<<1))
#define AX_READ         (PIND & (1<<2))
#define AY_READ         (PIND & (1<<3))

#define TIMER_INTERRUPT_ENABLE      TIMSK0 |= (1<<OCIE0A)
#define TIMER_INTERRUPT_DISABLE     TIMSK0 &= ~(1<<OCIE0A)
#define TIMER_TCNT_CLEAR            TCNT0 = 0x00
#define TIMER_TIFR_CLEAR            TIFR0 |= (1<<OCF0A)

#define AX_INTERRUPT_ENABLE         EIMSK = (1<<INT0)
#define AX_INTERRUPT_DISABLE        EIMSK &= ~(1<<INT0)
#define AX_INTERRUPT_RISING         EICRA = (1<<ISC01)|(1<<ISC00)
#define AX_INTERRUPT_FALLING        EICRA = (1<<ISC01)

#define AY_INTERRUPT_ENABLE         EIMSK = (1<<INT1)
#define AY_INTERRUPT_DISABLE        EIMSK &= ~(1<<INT1)
#define AY_INTERRUPT_RISING         EICRA = (1<<ISC11)|(1<<ISC10)
#define AY_INTERRUPT_FALLING        EICRA = (1<<ISC11)

#define SW_USE			PORTD |= (1<<5)
#define SW_CHECK		(PIND & (1<<5))

#define LED_USE			DDRB |= (1<<5)
#define LED_ON			PORTB |= (1<<5)
#define LED_OFF			PORTB &= ~(1<<5)

void Init()
{
	TCNT0 = 0x00;
	OCR0A = 159; 
	TCCR0A |= (1<<WGM01);
	TCCR0B |= (1<<CS00);

	#ifdef PATCH_SW
	 SW_USE;
	#endif

	LED_USE;
	sei();
}

#endif
