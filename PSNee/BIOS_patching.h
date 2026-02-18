
#pragma once

#ifdef BIOS_PATCH
  uint8_t current_pulses = 0;
  volatile uint8_t pulse_counter = 0;
  volatile uint8_t patch_done = 0;

  #ifdef TEST_BIOS

    void Bios_Patching() {
      PIN_DX_INPUT;
      //PIN_DX_LOW;

          
    cli();                                  // Disable interrupts for timing integrity
    
    /* 
     * PHASE 1: Signal Stabilization & Alignment
     * Synchronizes the MCU with the PS1 startup state (Cold Boot vs Reset).
     */
    if (PIN_AX_READ != 0) {                 // Case: Power-on / Line high (---__-_-_)
        while (PIN_AX_READ != 0);           // Wait for falling edge
        while (PIN_AX_READ == 0);           // Sync on first clean rising edge
    } else {                                // Case: Reset / Line low (_____-_-_)
        while (PIN_AX_READ == 0);           // Wait for rising edge
    }

    /* 
     * PHASE 2: Address Bus Window Alignment
     * Bypassing initial boot routines to reach one window with a
     * known "idle gap" in the address bus activity, positioned 
     * immediately before the target memory-access cycle.
     * BOOT_OFFSET:  |----//----------|
     * AX:        ___-_-_//-_-_-________________-_-_
     */
    _delay_ms(BOOT_OFFSET);    
    PIN_LED_ON;     

        /* 
     * PHASE 3:  Edge Trigger
     * Capture the moment AX go HIGH.
     * Edge Triger:                  |
     * AX: _-_-_-_-_-________________-_-_-_-_-_-__
     */
    #ifdef LOW_TRIGGER
    while (PIN_AX_READ);
    #else
    while (! PIN_AX_READ);
    #endif

    /* 
     * PHASE 4: Precision Bit Alignment
     * Delay to shift from AX address edge to the DX data bit.
     * BIT_OFFSET:                   |-------//-----|
     * AX: _-_-_-_-_-________________-_-_-_-_//_-_-_-_
     */
    _delay_us(BIT_OFFSET);                            

    /* 
    * PHASE 5: Data Bus Overdrive (The Patch)
    * Briefly forcing PIN_DX to OUTPUT to pull the line and "nullify" the target bit.
    * This effectively overwrites the BIOS data on-the-fly 
    * before reverting the pin to INPUT to release the bus.
    */
    PIN_DX_OUTPUT;                          // Force line (Low/High-Z override)
    _delay_us(OVERRIDE);                       
    PIN_DX_INPUT;                           // Release bus immediately
    PIN_LED_OFF;
    sei();                                  // Restore global interrupts

          
    patch_done = 1; 
  }

  #endif

  #ifdef HIGH_PATCH_A


    ISR(PIN_AY_INTERRUPT_VECTOR){

      pulse_counter++;                         
      if (pulse_counter == PULSE_COUNT_2)           
      {
        _delay_us (BIT_OFFSET_2);                           
        PIN_DX_OUTPUT;                  
        _delay_us (OVERRIDE_2);                      
        PIN_DX_INPUT;                        
        PIN_AY_INTERRUPT_DISABLE;           

        patch_done = 2;                      
      }
    }

    void Bios_Patching(){
          PIN_DX_INPUT;
          cli();                                  // Disable interrupts for timing integrity

      if (PIN_AX_READ != 0)                
      {
        while (PIN_AX_READ != 0);           
        while (PIN_AX_READ == 0);          
      }
      else                                  
      {
        while (PIN_AX_READ == 0);          
      }

                  
      _delay_ms(BOOT_OFFSET);
      PIN_LED_ON; 
       _delay_us(BIT_OFFSET);     

    PIN_DX_OUTPUT;                          // Force line (Low/High-Z override)
    _delay_us(OVERRIDE);                       
    PIN_DX_INPUT;                           // Release bus immediately
    PIN_LED_OFF;
    sei();                                  // Restore global interrupts 

      
      while (patch_done != 1);                  
      PIN_LED_OFF;
      while (PIN_AY_READ != 0);             

      _delay_ms(FOLLOWUP_OFFSET);  

      PIN_AY_INTERRUPT_RISING;                    
      PIN_AY_INTERRUPT_ENABLE;            
      while (patch_done != 2);                 

    }

  #endif
 

  #ifdef INTERRUPT_RISING_HIGH_PATCH

    ISR(PIN_AX_INTERRUPT_VECTOR) {
      pulse_counter++;                         
      if (pulse_counter == PULSE_COUNT){          
        _delay_us (BIT_OFFSET);                                         
        PIN_DX_SET;                     
        PIN_DX_OUTPUT;                   
        _delay_us (OVERRIDE);                       
        PIN_DX_CLEAR;                     
        PIN_DX_INPUT;                      
        PIN_AX_INTERRUPT_DISABLE;          

        pulse_counter = 0;                    
        patch_done = 1;                      
      }
    }
  
    ISR(PIN_AY_INTERRUPT_VECTOR){

      pulse_counter++;                         
      if (pulse_counter == PULSE_COUNT_2)           
      {
        _delay_us (BIT_OFFSET_2);                           
        PIN_DX_OUTPUT;                  
        _delay_us (OVERRIDE_2);                      
        PIN_DX_INPUT;                        
        PIN_AY_INTERRUPT_DISABLE;           

        patch_done = 2;                      
      }
    }

    void Bios_Patching(){

      if (PIN_AX_READ != 0)                
      {
        while (PIN_AX_READ != 0);           
        while (PIN_AX_READ == 0);          
      }
      else                                  
      {
        while (PIN_AX_READ == 0);          
      }

                  
      _delay_ms(BOOT_OFFSET);
      PIN_LED_ON; 
      PIN_AX_INTERRUPT_RISING;                     
      PIN_AX_INTERRUPT_ENABLE;              
      

      
      while (patch_done != 1);                  
      PIN_LED_OFF;
      while (PIN_AY_READ != 0);             

      _delay_ms(FOLLOWUP_OFFSET);  

      PIN_AY_INTERRUPT_RISING;                    
      PIN_AY_INTERRUPT_ENABLE;            
      while (patch_done != 2);                 

    }

  #endif




#endif


