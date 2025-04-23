
#pragma once

#if defined(SCPH_102A) || defined(SCPH_102) || defined(SCPH_100) || defined(SCPH_7000_9000) || defined(SCPH_5500) || defined(SCPH_3500_5000) || defined(SCPH_3000) || defined(SCPH_1000)


void Timer_Start(void);
void Timer_Stop(void);

extern volatile uint8_t count_isr;
extern volatile uint32_t microsec;
extern volatile uint32_t millisec;


volatile uint8_t impulse = 0;
volatile uint8_t patch = 0;

#endif

#if defined(SCPH_102) || defined(SCPH_100) || defined(SCPH_7000_9000) || defined(SCPH_5500) || defined(SCPH_3500_5000) || defined(SCPH_3000) || defined(SCPH_1000)

ISR(PIN_AX_INTERRUPT_VECTOR) {
	impulse++;                         
	if (impulse == TRIGGER){           // If impulse reaches the value defined by TRIGGER, the following actions are performed:
		HOLD;                            
		#ifdef HIGH_PATCH                
		 PIN_DX_SET;                     
		#endif
		PIN_DX_OUTPUT;                   
		PATCHING;                       
		#ifdef HIGH_PATCH
		 PIN_DX_CLEAR;                     
		#endif
		PIN_DX_INPUT;                      
		PIN_AX_INTERRUPT_DISABLE;          

		impulse = 0;                    
		patch = 1;                       // patch is set to 1, indicating that the first patch is completed.
	}
}


#ifdef HIGH_PATCH 

ISR(PIN_AY_INTERRUPT_VECTOR){

	impulse++;                         
	if (impulse == TRIGGER2)           // If impulse reaches the value defined by TRIGGER2, the following actions are performed:
	{
		HOLD2;                           
		PIN_DX_OUTPUT;                  
		PATCHING2;                      
		PIN_DX_INPUT;                        
		PIN_AY_INTERRUPT_DISABLE;           

		patch = 2;                       // patch is set to 2, indicating that the second patch is completed.
	}
}
#endif

void Bios_Patching(){

  // If LOW_TRIGGER is defined
	#ifdef LOW_TRIGGER                
	 PIN_AX_INTERRUPT_FALLING;           
	 #else
	 PIN_AX_INTERRUPT_RISING;             
	#endif

	if (PIN_AX_READ != 0)                 // If the AX pin is high
	{
		while (PIN_AX_READ != 0);           // Wait for it to go low
		while (PIN_AX_READ == 0);           // Then wait for it to go high again.
	}
	else                                  // If the AX pin is low
	{
		while (PIN_AX_READ == 0);           // Wait for it to go high.
	}
	
	Timer_Start();                    
	while (microsec < CHECKPOINT);        // Wait until the number of microseconds elapsed reaches a value defined by CHECKPOINT.
	Timer_Stop();                     
	PIN_AX_INTERRUPT_ENABLE;              
	
	while (patch != 1);                   // Wait for the first stage of the patch to complete:

	#ifdef HIGH_PATCH 

	 #ifdef HIGH_PATCH              
	  PIN_AY_INTERRUPT_FALLING;          
	  #else
	  PIN_AY_INTERRUPT_RISING;           
	 #endif
   
	  while (PIN_AY_READ != 0);             // Wait for it to go low
	  Timer_Start();                   
	  while (microsec < CHECKPOINT2);     // Wait until the number of microseconds elapsed reaches a value defined by CHECKPOINT2.
	  Timer_Stop();                       
	  PIN_AY_INTERRUPT_ENABLE;            

	  while (patch != 2);                 // Wait for the second stage of the patch to complete:

	#endif
}
#endif

#ifdef SCPH_102A
void Bios_Patching_SCPH_102A() {

//   PIN_AX_INPUT;                          //A18
//   PIN_DX_INPUT;                          //D2

//   Timer_Start();
//   while (millisec < SATBILIZATIONPOINT);               // this is right after SQCK appeared. wait a little to avoid noise
//   while (PIN_AX_READ != 0);
//   Timer_Stop();

//   //                                  //wait for stage 1 A18 pulse


//   Timer_Start();
//   while (millisec < DELAYPOINT);       //wait through stage 1 of A18 activity delay(1350)
//   Timer_Stop();

//   //noInterrupts();                       // start critical section
//   GLOBAL_INTERRUPT_DISABLE;
//   Timer_Start();
//   while (PIN_AX_READ != 0);
//   {
//     ;                                   //wait for priming A18 pulse
//   }
//   //while (microsec < HOLD );      // delayMicroseconds(17) min 13us max 17us for 16Mhz ATmega (maximize this when tuning!)
//   HOLD;
//   PIN_DX_CLEAR;                            // store a low
//   PIN_DX_OUTPUT;                           // D2 = output. drags line low now
//   PATCHING;
//   //while (microsec < PATCHING );  // delayMicroseconds(4) min 2us for 16Mhz ATmega, 8Mhz requires 3us (minimize this when tuning, after maximizing first us delay!)
//   PIN_DX_INPUT;                            // D2 = input / high-z
//   GLOBAL_INTERRUPT_ENABLE;
//   //interrupts();                         // end critical section
//   Timer_Stop();
//                                           // not necessary but I want to make sure these pins are now high-z again
//   PIN_AX_INPUT;
//   PIN_DX_INPUT;
// }

// Original function equivalent to NTSC_fix(), using macros
void Bios_Patching_SCPH_102A(void) {

    // configure A18 and D2 as inputs
    PIN_AX_INPUT;
    PIN_DX_INPUT;

    // arm A18 interrupt for noise immunity (optional)
    PIN_AX_INTERRUPT_RISING;
    PIN_AX_INTERRUPT_ENABLE;

    // initial stabilization delay
    Timer_Start();
    SATBILIZATIONPOINT;    // _delay_ms(100)
    Timer_Stop();

    // wait for stage 1 A18 pulse
    while (!PIN_AX_READ) ;

    // wait through stage 1 of A18 activity
    Timer_Start();
    DELAYPOINT;            // _delay_ms(1350)
    Timer_Stop();

    // critical section
    noInterrupts();
    // wait for priming A18 pulse
    while (!PIN_AX_READ) ;
    Timer_Start();
    HOLD;                  // _delay_us(17)
    Timer_Stop();

    // drive D2 low for patch
    PIN_DX_CLEAR;          // clear D2
    PIN_DX_OUTPUT;         // set D2 as output
    Timer_Start();
    PATCH;                 // _delay_us(4)
    Timer_Stop();
    PIN_DX_INPUT;          // release D2 (input/high-Z)
    interrupts();

    // restore pins to input
    PIN_AX_INPUT;
    PIN_DX_INPUT;

    // disable A18 interrupt now that patch is done
    PIN_AX_INTERRUPT_DISABLE;
}


#endif


