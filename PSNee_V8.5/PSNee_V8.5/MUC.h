//Configuring the clock speed and associated registers.  F_CPU / (TCCR0B |= (1<<CS00) *(OCR0A = 159 +1) = 16000000 /(0 * (160)) = 100KHz

#pragma once

#ifdef ATmega328_168

#define F_CPU 16000000L
#define TIMER_TCNT_CLEAR            TCNT0   =   0x00             //TCNT0 - Timer/Counter Register
#define SET_OCROA_DIV               OCR0A   =   159;             //OCR0A – Output Compare Register A, 0x10011111, 100KHz
#define SET_TIMER_TCCROA            TCCR0A |=  (1 << WGM01);     //TCCR0A – Timer/Counter Control Register A. turn on CTC mode, WGM01
#define SET_TIMER_TCCROB            TCCR0B |=  (1 << CS00);      //TCCR0B – Timer/Counter Control Register B,  CS00: Clock Select,  clk I/O
                                                                 //Waveform Generation Mode, Mode 2 CTC
#define CTC_TIMER_VECTOR            TIMER0_COMPA_vect            //interrupt vector for match event, OCR0A comparison and Timer/Counter 0


#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

// Globale interrupt seting
#define GLOBAL_INTERRUPT_ENABLE     SREG   |=  (1<<7)           
#define GLOBAL_INTERRUPT_DISABLE    SREG   &= ~(1<<7)    

// Handling the main pins

// Main pins input
#define PIN_DATA_INPUT              DDRB   &= ~(1<<DDB0)        
#define PIN_WFCK_INPUT              DDRB   &= ~(1<<DDB1)       // Create a mask (1<<0) with the first bit at 1 b00000001 uses the ~ operator to perform a bit inversion b11111110,
#define PIN_SQCK_INPUT              DDRD   &= ~(1<<DDD6)       // &= updates the DDRB register with the AND operator and the mask, DDRB bxxxxxxxx OR mask b11111110 = bxxxxxxx0                     
#define PIN_SUBQ_INPUT              DDRD   &= ~(1<<DDD7) 
                            
// Main pin output
#define PIN_DATA_OUTPUT             DDRB   |=  (1<<DDB0)       // Create a mask (1<<0) with the first bit at 1 b00000001,     
#define PIN_WFCK_OUTPUT             DDRB   |=  (1<<DDB1)       // |= updates the DDRB register with the OR operator and the mask, DDRB bxxxxxxxx OR mask b00000001 = bxxxxxxx1
           
// Define pull-ups and set high at the main pin
#define PIN_DATA_SET                PORTB  |=  (1<<PB0)        // Create a mask (1<<0) with the first bit at 1 b00000001,
                                                               // |= updates the PORTB register with the OR operator and the mask, PORTB bxxxxxxxx OR mask b00000001 = bxxxxxxx1
             
// Define pull-ups set down at the main pin
#define PIN_DATA_CLEAR              PORTB  &= ~(1<<PB0)        // Create a mask (1<<0) with the first bit at 1 b00000001 uses the ~ operator to perform a bit inversion b11111110,                    
#define PIN_WFCK_CLEAR              PORTB  &= ~(1<<PB1)        // &= updates the DDRB register with the AND operator and the mask, DDRB bxxxxxxxx OR mask b11111110 = bxxxxxxx0
                         
// Read the main pins
#define PIN_SQCK_READ              (PIND   &   (1<<PIND6))     // Create a mask (1<<6) with the six bit at 1 b00100000,                       
#define PIN_SUBQ_READ              (PIND   &   (1<<PIND7))     // compare the PINB register and the mask with the AND operator, and returns the result, PINB bxx1xxxxx AND mask b00100000 = 1                      
#define PIN_WFCK_READ              (PINB   &   (1<<PINB1))                                                       

// Handling and use of the LED pin
#define LED_RUN
#define PIN_LED_OUTPUT              DDRB   |=  (1<<DDB5)                                
#define PIN_LED_ON                  PORTB  |=  (1<<PB5)      
#define PIN_LED_OFF                 PORTB  &= ~(1<<PB5)   

// Handling the BIOS patch

// BIOS interrupt seting
#define TIMER_INTERRUPT_ENABLE      TIMSK0 |=  (1<<OCIE0A)
#define TIMER_INTERRUPT_DISABLE     TIMSK0 &= ~(1<<OCIE0A)

// BIOS timer clear
#define TIMER_TIFR_CLEAR            TIFR0  |=  (1<<OCF0A)

// Pins input
#define PIN_AX_INPUT                DDRD   &= ~(1<<DDD2)                            
#define PIN_AY_INPUT                DDRD   &= ~(1<<DDD3)                             
#define PIN_DX_INPUT                DDRD   &= ~(1<<DDD4)                             
// Pin output
#define PIN_DX_OUTPUT               DDRD   |=  (1<<DDD4)                             
// Define pull-ups set high 
#define PIN_DX_SET                  PORTD  |=  (1<<PD4)                              
// Define pull-ups set down 
#define PIN_DX_CLEAR                PORTD  &= ~(1<<PD4)                            
// Read pins for BIOS patch
#define PIN_AX_READ                (PIND   &   (1<<PIND2))                             
#define PIN_AY_READ                (PIND   &   (1<<PIND3))                             

// Handling the external interrupt
#define PIN_AX_INTERRUPT_ENABLE     EIMSK  |=  (1<<INT0)
#define PIN_AY_INTERRUPT_ENABLE     EIMSK  |=  (1<<INT1)

#define PIN_AX_INTERRUPT_DISABLE    EIMSK  &= ~(1<<INT0)
#define PIN_AY_INTERRUPT_DISABLE    EIMSK  &= ~(1<<INT1)

#define PIN_AX_INTERRUPT_RISING     EICRA  |=  (1<<ISC01)|(1<<ISC00)
#define PIN_AY_INTERRUPT_RISING     EICRA  |=  (1<<ISC11)|(1<<ISC10)

#define PIN_AX_INTERRUPT_FALLING   (EICRA   =  (EICRA & ~(1<<ISC00)) | (1<<ISC01))
#define PIN_AY_INTERRUPT_FALLING   (EICRA   =  (EICRA & ~(1<<ISC10)) | (1<<ISC11))

#define PIN_AX_INTERRUPT_VECTOR        INT0_vect              
#define PIN_AY_INTERRUPT_VECTOR        INT1_vect    

// Handling and reading the switch pin for patch BIOS
#define PIN_SWITCH_INPUT            DDRD   &= ~(1<<DDD5)                              
#define PIN_SWITCH_SET              PORTD  |=  (1<<PD5)                                
#define PIN_SWICHE_READ            (PIND   &   (1<<PIND5))

#endif

#ifdef LGT8F328P

#define F_CPU 32000000L
#define TIMER_TCNT_CLEAR            TCNT0   =   0x00             
#define SET_OCROA_DIV               OCR0A   =   319;             
#define SET_TIMER_TCCROA            TCCR0A |=  (1 << WGM01);     
#define SET_TIMER_TCCROB            TCCR0B |=  (1 << CS00); 
#define CTC_TIMER_VECTOR            TIMER0_COMPA_vect            //??


#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

// Globale interrupt seting
#define GLOBAL_INTERRUPT_ENABLE     SREG   |=  (1<<7)           
#define GLOBAL_INTERRUPT_DISABLE    SREG   &= ~(1<<7)    

// Handling the main pins

// Main pins input
#define PIN_DATA_INPUT              DDRB   &= ~(1<<DDB0)        
#define PIN_WFCK_INPUT              DDRB   &= ~(1<<DDB1)       
#define PIN_SQCK_INPUT              DDRD   &= ~(1<<DDD6)      
#define PIN_SUBQ_INPUT              DDRD   &= ~(1<<DDD7) 
                            
// Main pin output
#define PIN_DATA_OUTPUT             DDRB   |=  (1<<DDB0)       
#define PIN_WFCK_OUTPUT             DDRB   |=  (1<<DDB1)       
           
// Define pull-ups and set high at the main pin
#define PIN_DATA_SET                PORTB  |=  (1<<PB0)        
                                                              
             
// Define pull-ups set down at the main pin
#define PIN_DATA_CLEAR              PORTB  &= ~(1<<PB0)        
#define PIN_WFCK_CLEAR              PORTB  &= ~(1<<PB1)        
                         
// Read the main pins
#define PIN_SQCK_READ              (PIND   &   (1<<PIND6))         
#define PIN_SUBQ_READ              (PIND   &   (1<<PIND7))     
#define PIN_WFCK_READ              (PINB   &   (1<<PINB1))                                                       

// Handling and use of the LED pin
#define LED_RUN
#define PIN_LED_OUTPUT              DDRB   |=  (1<<DDB5)                                
#define PIN_LED_ON                  PORTB  |=  (1<<PB5)      
#define PIN_LED_OFF                 PORTB  &= ~(1<<PB5)   

// Handling the BIOS patch

// BIOS interrupt seting
#define TIMER_INTERRUPT_ENABLE      TIMSK0 |=  (1<<OCIE0A)
#define TIMER_INTERRUPT_DISABLE     TIMSK0 &= ~(1<<OCIE0A)

// BIOS timer clear
#define TIMER_TIFR_CLEAR            TIFR0  |=  (1<<OCF0A)

// Pins input
#define PIN_AX_INPUT                DDRD   &= ~(1<<DDD2)                            
#define PIN_AY_INPUT                DDRD   &= ~(1<<DDD3)                             
#define PIN_DX_INPUT                DDRD   &= ~(1<<DDD4)                             
// Pin output
#define PIN_DX_OUTPUT               DDRD   |=  (1<<DDD4)                             
// Define pull-ups set high 
#define PIN_DX_SET                  PORTD  |=  (1<<PD4)                              
// Define pull-ups set down 
#define PIN_DX_CLEAR                PORTD  &= ~(1<<PD4)                            
// Read pins for BIOS patch
#define PIN_AX_READ                (PIND   &   (1<<PIND2))                             
#define PIN_AY_READ                (PIND   &   (1<<PIND3))                             

// Handling the external interrupt
#define PIN_AX_INTERRUPT_ENABLE     EIMSK  |=  (1<<INT0)
#define PIN_AY_INTERRUPT_ENABLE     EIMSK  |=  (1<<INT1)

#define PIN_AX_INTERRUPT_DISABLE    EIMSK  &= ~(1<<INT0)
#define PIN_AY_INTERRUPT_DISABLE    EIMSK  &= ~(1<<INT1)

#define PIN_AX_INTERRUPT_RISING     EICRA  |=  (1<<ISC01)|(1<<ISC00)
#define PIN_AY_INTERRUPT_RISING     EICRA  |=  (1<<ISC11)|(1<<ISC10)

#define PIN_AX_INTERRUPT_FALLING   (EICRA   =  (EICRA & ~(1<<ISC00)) | (1<<ISC01))
#define PIN_AY_INTERRUPT_FALLING   (EICRA   =  (EICRA & ~(1<<ISC10)) | (1<<ISC11))

#define PIN_AX_INTERRUPT_VECTOR        INT0_vect              
#define PIN_AY_INTERRUPT_VECTOR        INT1_vect    

// Handling and reading the switch pin for patch BIOS
#define PIN_SWITCH_INPUT            DDRD   &= ~(1<<DDD5)                              
#define PIN_SWITCH_SET              PORTD  |=  (1<<PD5)                                
#define PIN_SWICHE_READ            (PIND   &   (1<<PIND5))




#endif

#ifdef ATmega32U4_16U4

#define F_CPU 16000000L
#define TIMER_TCNT_CLEAR            TCNT0   =   0x00;
#define SET_OCROA_DIV               OCR0A   =   159;
#define SET_TIMER_TCCROA            TCCR0A |=  (1 << WGM01);
#define SET_TIMER_TCCROB            TCCR0B |=  (1 << CS00);
#define CTC_TIMER_VECTOR            TIMER0_COMPA_vect

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

// Globale interrupt seting
#define GLOBAL_INTERRUPT_ENABLE     SREG   |=  (1<<7)           
#define GLOBAL_INTERRUPT_DISABLE    SREG   &= ~(1<<7)    

#define TIMER_INTERRUPT_ENABLE      TIMSK0 |=  (1<<OCIE0A)
#define TIMER_INTERRUPT_DISABLE     TIMSK0 &= ~(1<<OCIE0A)

// Handling the main pins

// Main pins
#define PIN_DATA_INPUT              DDRB   &= ~(1<<DDB4)        
#define PIN_WFCK_INPUT              DDRB   &= ~(1<<DDB5)       
#define PIN_SQCK_INPUT              DDRD   &= ~(1<<DDD7)                           
#define PIN_SUBQ_INPUT              DDRE   &= ~(1<<DDE6) 
                            
#define PIN_DATA_OUTPUT             DDRB   |=  (1<<DDB4)       
#define PIN_WFCK_OUTPUT             DDRB   |=  (1<<DDB5)     
           
// Define pull-ups and set high at the main pin
#define PIN_DATA_SET                PORTB  |=  (1<<PB4)  
// Define pull-ups set down at the main pin
#define PIN_DATA_CLEAR              PORTB  &= ~(1<<PB4)                        
#define PIN_WFCK_CLEAR              PORTB  &= ~(1<<PB5)                    
// Read the main pins
#define PIN_SQCK_READ              (PIND   &   (1<<PIND7))                       
#define PIN_SUBQ_READ              (PINE   &   (1<<PINE6))                      
#define PIN_WFCK_READ              (PINB   &   (1<<PINB5))                                                       

// Handling and use of the LED pin
#define LED_RUN
#define PIN_LED_OUTPUT              DDRB   |=  (1<<DDB6)                                
#define PIN_LED_ON                  PORTB  |=  (1<<PB6)      
#define PIN_LED_OFF                 PORTB  &= ~(1<<PB6)   

// Handling the BIOS patch

// Pins input
#define PIN_AX_INPUT                DDRD   &= ~(1<<DDD1)                            
#define PIN_AY_INPUT                DDRD   &= ~(1<<DDD0)                             
#define PIN_DX_INPUT                DDRD   &= ~(1<<DDD4)                             
// Pin output
#define PIN_DX_OUTPUT               DDRD   |=  (1<<DDD4)                             
// Define pull-ups set high 
#define PIN_DX_SET                  PORTD  |=  (1<<PD4)                              
// Define pull-ups set down 
#define PIN_DX_CLEAR                PORTD  &= ~(1<<PD4)                            
// Read pins for BIOS patch
#define PIN_AX_READ                (PIND   &   (1<<PIND1))                             
#define PIN_AY_READ                (PIND   &   (1<<PIND0)) 
// Handling and reading the switch pin for patch BIOS
#define PIN_SWITCH_INPUT            DDRC   &= ~(1<<DDC6)                              
#define PIN_SWITCH_SET              PORTC  |=  (1<<PC6)                                
#define PIN_SWICHE_READ            (PINC   &   (1<<PINC6))

// BIOS timer clear
#define TIMER_TCNT_CLEAR            TCNT0  =    0x00
#define TIMER_TIFR_CLEAR            TIFR0  |=  (1<<OCF0A)

// Handling the external interrupt
#define PIN_AX_INTERRUPT_ENABLE     EIMSK  |=  (1<<INT1)
#define PIN_AY_INTERRUPT_ENABLE     EIMSK  |=  (1<<INT0)

#define PIN_AX_INTERRUPT_DISABLE    EIMSK  &= ~(1<<INT1)
#define PIN_AY_INTERRUPT_DISABLE    EIMSK  &= ~(1<<INT0)

#define PIN_AX_INTERRUPT_RISING     EICRA  |=  (1<<ISC11)|(1<<ISC10)
#define PIN_AY_INTERRUPT_RISING     EICRA  |=  (1<<ISC01)|(1<<ISC00)

#define PIN_AX_INTERRUPT_FALLING   (EICRA   =  (EICRA & ~(1<<ISC10)) | (1<<ISC11))
#define PIN_AY_INTERRUPT_FALLING   (EICRA   =  (EICRA & ~(1<<ISC00)) | (1<<ISC01))

#define PIN_AX_INTERRUPT_VECTOR        INT1_vect
#define PIN_AY_INTERRUPT_VECTOR        INT0_vect

#endif

#ifdef ATtiny85_45_25

#define DF_CPU 8000000L
#define TIMER_TCNT_CLEAR            TCNT0  =   0x00;            
#define SET_OCROA_DIV               OCR0A  =   79;              //OCR0A – Output Compare Register A,100KHz
#define SET_TIMER_TCCROA            TCCR0A |= (1 << WGM01);     
#define SET_TIMER_TCCROB            TCCR0B |= (1 << CS00);
#define CTC_TIMER_VECTOR            TIMER0_COMPA_vect



#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

// Globale interrupt seting
#define GLOBAL_INTERRUPT_ENABLE     SREG   |=  (1<<7)           
#define GLOBAL_INTERRUPT_DISABLE    SREG   &= ~(1<<7)    

// Handling the main pins

// Main pins input
#define PIN_DATA_INPUT              DDRB   &= ~(1<<DDB2)        
#define PIN_WFCK_INPUT              DDRB   &= ~(1<<DDB4)      
#define PIN_SQCK_INPUT              DDRB   &= ~(1<<DDB0)              
#define PIN_SUBQ_INPUT              DDRB   &= ~(1<<DDB1) 
                            
// Main pin output
#define PIN_DATA_OUTPUT             DDRB   |=  (1<<DDB2)    
#define PIN_WFCK_OUTPUT             DDRB   |=  (1<<DDB4) 
           
// Define pull-ups and set high at the main pin
#define PIN_DATA_SET                PORTB  |=  (1<<PB2)    
             
// Define pull-ups set down at the main pin
#define PIN_DATA_CLEAR              PORTB  &= ~(1<<PB2)                   
#define PIN_WFCK_CLEAR              PORTB  &= ~(1<<PB4)   
                         
// Read the main pins
#define PIN_SQCK_READ              (PINB   &   (1<<PINB0))                  
#define PIN_SUBQ_READ              (PINB   &   (1<<PINB1))                      
#define PIN_WFCK_READ              (PINB   &   (1<<PINB4))      

#defintu conais les attiny 414e TIMER_INTERRUPT_ENABLE      TIMSK |=  (1<<OCIE0A)
#define TIMER_INTERRUPT_DISABLE     TIMSK &= ~(1<<OCIE0A)

#if !defined(SCPH_xxx1) && !defined(SCPH_xxx2) && !defined(SCPH_103)
 #error "ATtiny85_45_25 Not compatible with BIOS patch, please choose a compatible SCPH. For example: SCPH_xxx1: SCPH_xxx2: SCPH_103"
#endif

#ifdef ATtiny88_48

#define F_CPU 16000000L
#define TIMER_TCNT_CLEAR            TCNT0   =   0x00             //TCNT0 - Timer/Counter Register
#define SET_OCROA_DIV               OCR0A   =   159;             //OCR0A – Output Compare Register A, 0x10011111, 100KHz
#define SET_TIMER_TCCROA            TCCR0A |=  (1 << CTC0 );     //TCCR0A – Timer/Counter Control Register A. turn on CTC mode, CTC0
#define SET_TIMER_TCCROB            TCCR0A |=  (1 << CS00);      //TCCR0B – Timer/Counter Control Register B,  CS00: Clock Select,  clk I/O
                                                                 //Waveform Generation Mode, Mode 2 CTC
#define CTC_TIMER_VECTOR            TIMER0_COMPA_vect            //interrupt vector for match event, OCR0A comparison and Timer/Counter 0


#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

// Globale interrupt seting
#define GLOBAL_INTERRUPT_ENABLE     SREG   |=  (1<<7)           
#define GLOBAL_INTERRUPT_DISABLE    SREG   &= ~(1<<7)    

// Handling the main pins

// Main pins input
#define PIN_DATA_INPUT              DDRB   &= ~(1<<DDB0)        
#define PIN_WFCK_INPUT              DDRB   &= ~(1<<DDB1)       // Create a mask (1<<0) with the first bit at 1 b00000001 uses the ~ operator to perform a bit inversion b11111110,
#define PIN_SQCK_INPUT              DDRD   &= ~(1<<DDD6)       // &= updates the DDRB register with the AND operator and the mask, DDRB bxxxxxxxx OR mask b11111110 = bxxxxxxx0                     
#define PIN_SUBQ_INPUT              DDRD   &= ~(1<<DDD7) 
                            
// Main pin output
#define PIN_DATA_OUTPUT             DDRB   |=  (1<<DDB0)       // Create a mask (1<<0) with the first bit at 1 b00000001,     
#define PIN_WFCK_OUTPUT             DDRB   |=  (1<<DDB1)       // |= updates the DDRB register with the OR operator and the mask, DDRB bxxxxxxxx OR mask b00000001 = bxxxxxxx1
           
// Define pull-ups and set high at the main pin
#define PIN_DATA_SET                PORTB  |=  (1<<PB0)        // Create a mask (1<<0) with the first bit at 1 b00000001,
                                                               // |= updates the PORTB register with the OR operator and the mask, PORTB bxxxxxxxx OR mask b00000001 = bxxxxxxx1
             
// Define pull-ups set down at the main pin
#define PIN_DATA_CLEAR              PORTB  &= ~(1<<PB0)        // Create a mask (1<<0) with the first bit at 1 b00000001 uses the ~ operator to perform a bit inversion b11111110,                    
#define PIN_WFCK_CLEAR              PORTB  &= ~(1<<PB1)        // &= updates the DDRB register with the AND operator and the mask, DDRB bxxxxxxxx OR mask b11111110 = bxxxxxxx0
                         
// Read the main pins
#define PIN_SQCK_READ              (PIND   &   (1<<PIND6))     // Create a mask (1<<6) with the six bit at 1 b00100000,                       
#define PIN_SUBQ_READ              (PIND   &   (1<<PIND7))     // compare the PINB register and the mask with the AND operator, and returns the result, PINB bxx1xxxxx AND mask b00100000 = 1                      
#define PIN_WFCK_READ              (PINB   &   (1<<PINB1))                                                       

// Handling and use of the LED pin
//#define LED_RUN
//#define PIN_LED_OUTPUT              DDRB   |=  (1<<DDB5)                                
//#define PIN_LED_ON                  PORTB  |=  (1<<PB5)      
//#define PIN_LED_OFF                 PORTB  &= ~(1<<PB5)   

// Handling the BIOS patch

// BIOS interrupt seting
#define TIMER_INTERRUPT_ENABLE      TIMSK0 |=  (1<<OCIE0A)
#define TIMER_INTERRUPT_DISABLE     TIMSK0 &= ~(1<<OCIE0A)

// BIOS timer clear
#define TIMER_TIFR_CLEAR            TIFR0  |=  (1<<OCF0A)

// Pins input
#define PIN_AX_INPUT                DDRD   &= ~(1<<DDD2)                            
#define PIN_AY_INPUT                DDRD   &= ~(1<<DDD3)                             
#define PIN_DX_INPUT                DDRD   &= ~(1<<DDD4)                             
// Pin output
#define PIN_DX_OUTPUT               DDRD   |=  (1<<DDD4)                             
// Define pull-ups set high 
#define PIN_DX_SET                  PORTD  |=  (1<<PD4)                              
// Define pull-ups set down 
#define PIN_DX_CLEAR                PORTD  &= ~(1<<PD4)                            
// Read pins for BIOS patch
#define PIN_AX_READ                (PIND   &   (1<<PIND2))                             
#define PIN_AY_READ                (PIND   &   (1<<PIND3))                             

// Handling the external interrupt
#define PIN_AX_INTERRUPT_ENABLE     EIMSK  |=  (1<<INT0)
#define PIN_AY_INTERRUPT_ENABLE     EIMSK  |=  (1<<INT1)

#define PIN_AX_INTERRUPT_DISABLE    EIMSK  &= ~(1<<INT0)
#define PIN_AY_INTERRUPT_DISABLE    EIMSK  &= ~(1<<INT1)

#define PIN_AX_INTERRUPT_RISING     EICRA  |=  (1<<ISC01)|(1<<ISC00)
#define PIN_AY_INTERRUPT_RISING     EICRA  |=  (1<<ISC11)|(1<<ISC10)

#define PIN_AX_INTERRUPT_FALLING   (EICRA   =  (EICRA & ~(1<<ISC00)) | (1<<ISC01))
#define PIN_AY_INTERRUPT_FALLING   (EICRA   =  (EICRA & ~(1<<ISC10)) | (1<<ISC11))

#define PIN_AX_INTERRUPT_VECTOR        INT0_vect              
#define PIN_AY_INTERRUPT_VECTOR        INT1_vect    

// Handling and reading the switch pin for patch BIOS
#define PIN_SWITCH_INPUT            DDRD   &= ~(1<<DDD5)                              
#define PIN_SWITCH_SET              PORTD  |=  (1<<PD5)                                
#define PIN_SWICHE_READ            (PIND   &   (1<<PIND5))

#endif

#ifdef ATtiny214_414



//#define SET_CTRLA
//#define F_CPU 16000000L
#define TIMER_TCNT_CLEAR            TCA0.SINGLE.CNT   =   0x00             //TCNT0 - Timer/Counter Register
#define SET_OCROA_DIV               TCA0.SINGLE.CMP0L   =   159;             //OCR0A – Output Compare Register A, 0x10011111, 100KHz
#define SET_TIMER_TCCROA            TCA0.SINGLE.CTRLB |=  TCA_SINGLE_WGMODE_FRQ_gc    //TCCR0A – Timer/Counter Control Register A. turn on CTC mode, CTC0
#define SET_TIMER_TCCROB            TCA0.SINGLE.CTRLA |=  TCA_SINGLE_CLKSEL_DIV1_gc//TCCR0B – Timer/Counter Control Register B,  CS00: Clock Select,  clk I/O
                                                                 //Waveform Generation Mode, Mode 2 CTC
#define CTC_TIMER_VECTOR            TCA0_CMP0_vect            //interrupt vector for match event, OCR0A comparison and Timer/Counter 0


#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

// Globale interrupt seting
#define GLOBAL_INTERRUPT_ENABLE     CPU.SREG   |=  (1<<7)           
#define GLOBAL_INTERRUPT_DISABLE    CPU.SREG   &= ~(1<<7)    

// Handling the main pins

// Main pins input
#define PIN_DATA_INPUT              PORTA.DIR = PIN2_bm      
#define PIN_WFCK_INPUT              PORTA.DIR = PIN1_bm         // Create a mask (1<<0) with the first bit at 1 b00000001 uses the ~ operator to perform a bit inversion b11111110,
#define PIN_SQCK_INPUT              PORTA.DIR = PIN4_bm         // &= updates the DDRB register with the AND operator and the mask, DDRB bxxxxxxxx OR mask b11111110 = bxxxxxxx0                     
#define PIN_SUBQ_INPUT              PORTA.DIR = PIN3_bm   
                            
// Main pin output
#define PIN_DATA_OUTPUT             PORTA.DIR |= PIN2_bm         // Create a mask (1<<0) with the first bit at 1 b00000001,     
#define PIN_WFCK_OUTPUT             PORTA.DIR |= PIN1_bm         // |= updates the DDRB register with the OR operator and the mask, DDRB bxxxxxxxx OR mask b00000001 = bxxxxxxx1
           
// Define pull-ups and set high at the main pin
#define PIN_DATA_SET                PORTA.OUT |= PIN2_bm         // Create a mask (1<<0) with the first bit at 1 b00000001,
                                                               // |= updates the PORTB register with the OR operator and the mask, PORTB bxxxxxxxx OR mask b00000001 = bxxxxxxx1
             
// Define pull-ups set down at the main pin
#define PIN_DATA_CLEAR              PORTA.OUT &= ~PIN2_bm         // Create a mask (1<<0) with the first bit at 1 b00000001 uses the ~ operator to perform a bit inversion b11111110,                    
#define PIN_WFCK_CLEAR              PORTA.OUT &= ~PIN1_bm         // &= updates the DDRB register with the AND operator and the mask, DDRB bxxxxxxxx OR mask b11111110 = bxxxxxxx0
                         
// Read the main pins
#define PIN_SQCK_READ              PORTA.IN & PIN4_bm     // Create a mask (1<<6) with the six bit at 1 b00100000,                       
#define PIN_SUBQ_READ             PORTA.IN & PIN3_bm     // compare the PINB register and the mask with the AND operator, and returns the result, PINB bxx1xxxxx AND mask b00100000 = 1                      
#define PIN_WFCK_READ              PORTA.IN & PIN1_bm                                                       

// Handling and use of the LED pin
#define LED_RUN
#define PIN_LED_OUTPUT              PORTB.DIR |= PIN2_bm                                
#define PIN_LED_ON                  PORTB.OUT |= PIN2_bm       
#define PIN_LED_OFF                 PORTB.OUT  &= ~PIN2_bm  

// Handling the BIOS patch

// BIOS interrupt seting
#define TIMER_INTERRUPT_ENABLE      TCA0.SPLIT.INTCTRL |= TCA_SINGLE_CMP0_bm
#define TIMER_INTERRUPT_DISABLE     TCA0.SPLIT.INTCTRL &= ~TCA_SPLIT_HCMP0_bm

// BIOS timer clear
#define TIMER_TIFR_CLEAR            TCA0.SPLIT.INTFLAGS = TCA_SPLIT_HCMP0_bm

// Pins input
#define PIN_AX_INPUT                PORTB.DIR &= ~PIN3_bm                             
#define PIN_AY_INPUT                PORTA.DIR &= ~PIN7_bm                              
#define PIN_DX_INPUT                PORTA.DIR &= ~PIN6_bm                              
// Pin output
#define PIN_DX_OUTPUT               PORTA.DIR |= PIN6_bm                              
// Define pull-ups set high 
#define PIN_DX_SET                  PORTA.OUT |= PIN6_bm                               
// Define pull-ups set down 
#define PIN_DX_CLEAR                PORTA.OUT &= ~PIN6_bm                             
// Read pins for BIOS patch
#define PIN_AX_READ                PORTB.IN & PIN3_bm                            
#define PIN_AY_READ                PORTA.IN & PIN6_bm                              

// Handling the external interrupt
//#define PIN_AX_INTERRUPT_ENABLE     PORTB.PIN3CTRL  |=  (1<<INTn)
//#define PIN_AY_INTERRUPT_ENABLE     PORTA.PIN7CTRL  |=  (1<<INTn)

#define PIN_AX_INTERRUPT_DISABLE    PORTB.PIN3CTRL = PORT_ISC_INTDISABLE_gc
#define PIN_AY_INTERRUPT_DISABLE    PORTA.PIN7CTRL = PORT_ISC_INTDISABLE_gc

#define PIN_AX_INTERRUPT_RISING     PORTB.PIN3CTRL = PORT_ISC_RISING_gc
#define PIN_AY_INTERRUPT_RISING     PORTA.PIN7CTRL = PORT_ISC_RISING_gc

#define PIN_AX_INTERRUPT_FALLING    PORTB.PIN3CTRL = PORT_ISC_FALLING_gc
#define PIN_AY_INTERRUPT_FALLING    PORTA.PIN7CTRL = PORT_ISC_FALLING_gc

//#define PB3_INTERRUPT PORTB.INTFLAGS & PIN3_bm
//#define PA7_INTERRUPT PORTB.INTFLAGS & PIN7_bm

#define PIN_AX_INTERRUPT_VECTOR        PORTB_PORT_vect               
#define PIN_AY_INTERRUPT_VECTOR        PORTA_PORT_vect    

// Handling and reading the switch pin for patch BIOS
#define PIN_SWITCH_INPUT            PORTA.DIR &= ~PIN5_bm                               
#define PIN_SWITCH_SET              PORTA.OUT |= PIN5_bm                                 
#define PIN_SWICHE_READ            PORTA.IN & PIN5_bm 

#endif


#ifdef CH32V003

#include "ch32v003.h" // Inclure le fichier d'en-tête spécifique au microcontrôleur
#include <stdint.h>
#include <stdbool.h>

// Fréquence d'horloge
#define F_CPU 8000000L

// Configuration du timer pour une fréquence de 100 kHz
#define TIMER_TCNT_CLEAR            TIM2_CNT = 0x00            // ok Effacer le compteur du Timer 2
#define SET_OCROA_DIV               TIM2_ARR = 79              // not Définir la valeur de comparaison pour générer une interruption à 100 kHz
#define SET_TIMER_TCCROA            TIM2_CR1 |= TIM_CR1_OPM    // notMettre le Timer en mode One Pulse (à adapter selon le mode souhaité)
#define SET_TIMER_TCCROB            TIM2_PSC = 0               // ok Définir le prescaler à 0 pour une fréquence maximale

// Vecteur d'interruption pour le Timer 2
#define CTC_TIMER_VECTOR            TIM2_UP_IRQHandler         // Remplacer par le vecteur d'interruption approprié

// Interruption globale
#define GLOBAL_INTERRUPT_ENABLE     __enable_irq()             
#define GLOBAL_INTERRUPT_DISABLE    __disable_irq()    

// Configuration des broches GPIO
#define PIN_DATA_INPUT              GPIOA->INDR &= ~(GPIO_MODER_MODER0)        
#define PIN_WFCK_INPUT              GPIOA->INDR &= ~(GPIO_MODER_MODER1)       
#define PIN_SQCK_INPUT              GPIOA->INDR &= ~(GPIO_MODER_MODER6)                     
#define PIN_SUBQ_INPUT              GPIOA->INDR &= ~(GPIO_MODER_MODER7) 

#define PIN_DATA_OUTPUT             GPIOA->OUTDR |=  (GPIO_MODER_MODER0_0)       
#define PIN_WFCK_OUTPUT             GPIOA->OUTDR |=  (GPIO_MODER_MODER1_0)       

#define PIN_DATA_SET                GPIOA->BSHR  |=  (GPIO_ODR_ODR_0)     

#define PIN_DATA_CLEAR              GPIOA->BRC  &= ~(GPIO_ODR_ODR_0)                     
#define PIN_WFCK_CLEAR              GPIOA->BRC  &= ~(GPIO_ODR_ODR_1)       

#define PIN_SQCK_READ              (GPIOA->IDR   &   (GPIO_IDR_IDR_6))    
#define PIN_SUBQ_READ              (GPIOA->IDR   &   (GPIO_IDR_IDR_7))                     
#define PIN_WFCK_READ              (GPIOA->IDR   &   (GPIO_IDR_IDR_1))                                                       

// Gestion de la broche LED
#define PIN_LED_OUTPUT              GPIOA->MODER |=  (GPIO_MODER_MODER5_0)                                
#define PIN_LED_ON                  GPIOA->ODR  |=  (GPIO_ODR_ODR_5)      
#define PIN_LED_OFF                 GPIOA->ODR  &= ~(GPIO_ODR_ODR_5)   

// Gestion des interruptions du timer
#define TIMER_INTERRUPT_ENABLE      TIM2_DIER |=  (TIM_DIER_UIE)
#define TIMER_INTERRUPT_DISABLE     TIM2_DIER &= ~(TIM_DIER_UIE)
#define TIMER_TIFR_CLEAR            TIM2_SR   &= ~(TIM_SR_UIF)

// Configuration des broches pour le BIOS
#define PIN_AX_INPUT                GPIOA->MODER &= ~(GPIO_MODER_MODER2)               AFIO_EXTICR             
#define PIN_AY_INPUT                GPIOA->MODER &= ~(GPIO_MODER_MODER3)                             
#define PIN_DX_INPUT                GPIOA->MODER &= ~(GPIO_MODER_MODER4)

#define PIN_DX_OUTPUT               GPIOA->MODER |=  (GPIO_MODER_MODER4_0)  

#define PIN_DX_SET                  GPIOA->ODR  |=  (GPIO_ODR_ODR_4)    

#define PIN_DX_CLEAR                GPIOA->ODR  &= ~(GPIO_ODR_ODR_4)   

#define PIN_AX_READ                (GPIOA->IDR   &   (GPIO_IDR_IDR_2))                             
#define PIN_AY_READ                (GPIOA->IDR   &   (GPIO_IDR_IDR_3))                             

// Gestion des interruptions externes
#define PIN_AX_INTERRUPT_ENABLE     EXTI->IMR  |=  (EXTI_IMR_MR0)               //1<<EXTI_INTENR_MR0
#define PIN_AY_INTERRUPT_ENABLE     EXTI->IMR  |=  (EXTI_IMR_MR1)

#define PIN_AX_INTERRUPT_DISABLE    EXTI->IMR  &= ~(EXTI_IMR_MR0)                 //EXTI_INTENR
#define PIN_AY_INTERRUPT_DISABLE    EXTI->IMR  &= ~(EXTI_IMR_MR1)

#define PIN_AX_INTERRUPT_RISING     EXTI->RTSR |=  (EXTI_RTSR_TR0)               //EXTI_RTENR
#define PIN_AY_INTERRUPT_RISING     EXTI->RTSR |=  (EXTI_RTSR_TR1)

#define PIN_AX_INTERRUPT_FALLING    EXTI->FTSR |=  (EXTI_FTENR_MR0)                  //EXTI_FTENR
#define PIN_AY_INTERRUPT_FALLING    EXTI->FTSR |=  (EXTI_FTENR_MR1)

#define PIN_AX_INTERRUPT_VECTOR     EXTI0_IRQHandler              
#define PIN_AY_INTERRUPT_VECTOR     EXTI1_IRQHandler   

// Gestion de la broche de commutation pour le BIOS
#define PIN_SWITCH_INPUT            GPIOA->MODER &= ~(GPIO_MODER_MODER5)                              
#define PIN_SWITCH_SET              GPIOA->ODR  |=  (GPIO_ODR_ODR_5)                                
#define PIN_SWICHE_READ            (GPIOA->IDR   &   (GPIO_IDR_IDR_5))

#endif

#endif