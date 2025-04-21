
#pragma once

#ifdef BIOS_PATCH

void Timer_Start(void);
void Timer_Stop(void);

extern volatile uint8_t count_isr;
extern volatile uint32_t microsec;


volatile uint8_t impulse = 0;
volatile uint8_t patch = 0;

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
