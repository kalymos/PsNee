
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
    if (PIN_AX_READ != 0) {
        while (PIN_AX_READ != 0);           // Wait for falling edge
        while (PIN_AX_READ == 0);           // Sync on first clean rising edge
    } else {
        while (PIN_AX_READ == 0);           // Wait for rising edge
    }

    /* 
     * PHASE 2: Address Bus Window Alignment
     * Bypassing initial boot routines to reach the target memory-access cycle.
     */
    _delay_ms(BOOT_OFFSET);    
    PIN_LED_ON;     

        /* 
     * PHASE 3: Zero-Jitter Pulse Counting (Falling Edge Trigger)
     * Optimized to capture the exact moment AX returns to LOW on the 48th pulse.
     */

    while (current_pulses < PULSE_COUNT) {
        // 1. Ultra-fast Rising Edge detection
        while (!(PIND & (1 << 2))); 
        
        current_pulses++;

        // 2. Falling Edge detection
        // This line is critical: the CPU remains "locked" here as long as the pulse is HIGH.
        while (PIND & (1 << 2)); 

        // On the PULSE_COUNT iteration, the loop exits IMMEDIATELY after the signal falls.
    }

    /* 
     * PHASE 4: Precision Bit Alignment
     * Strategic delay to shift from AX address edge to the DX data bit.
     */
    _delay_us(BIT_OFFSET);                            

    /* 
     * PHASE 5: Data Bus Overdrive (The Patch)
     * Overwriting the 0.2us pulse on the DX line.
     * Direct register access (Psnee v8.7 macros) ensures instantaneous execution.
     */
    PIN_DX_OUTPUT;                          // Force line (Low/High-Z override)
    _delay_us(OVERRIDE);                       
    PIN_DX_INPUT;                           // Release bus immediately
    PIN_LED_OFF;
    sei();                                  // Restore global interrupts

          
    patch_done = 1; 
  }

  #endif


  #ifdef INTERRUPT_RISING

    ISR(PIN_AX_INTERRUPT_VECTOR) {
     /* 
      * PHASE 3: Pulse Counting (Inside ISR)
      * The hardware Interrupt Service Routine (ISR) now takes over.
      * It counts the exact number of incoming pulses on PIN_AX until it 
      * matches the PULSE_COUNT value.
      */
      pulse_counter++;                         
      if (pulse_counter == PULSE_COUNT){           // If pulse_counter reaches the value defined by PULSE_COUNT
       /* 
        * PHASE 4: Precision Bit Alignment
        * Once the PULSE_COUNT is reached, a micro-delay (BIT_OFFSET) is applied.
        * This shifts the timing from the clock edge to the exact bit position 
        * within the data stream that needs modification.
        */
        _delay_us(BIT_OFFSET);                            
       /* 
        * PHASE 5: Data Bus Overdrive (The Patch)
        * Briefly forcing PIN_DX to OUTPUT to pull the line and "nullify" the target bit.
        * This effectively overwrites the BIOS data on-the-fly 
        * before reverting the pin to INPUT to release the bus.
        */
        PIN_DX_OUTPUT;
        _delay_us (OVERRIDE);                       
        PIN_DX_INPUT;                      
        PIN_AX_INTERRUPT_DISABLE;          

        pulse_counter = 0;                    
        patch_done = 1;                       // patch_done is set to 1, indicating that the first patch is completed.
      }
      PIN_LED_ON;
      PIN_LED_OFF;
    }

    void Bios_Patching(){
     /* 
      * PHASE 1: Signal Stabilization & Alignment
      * Detects the startup state (Cold Boot vs. Reset). 
      * If the line is already HIGH (Cold Boot), we wait for a full LOW-to-HIGH transition 
      * to ensure we are aligned with the start of a clean clock cycle.
      */

      if (PIN_AX_READ != 0)                 // Case: Power-on / Line high (---__-_-_)
      {
        while (PIN_AX_READ != 0);           // Wait for falling edge
        while (PIN_AX_READ == 0);           // Wait for next rising edge to sync
      }
      else                                  // Case: Reset / Line low (_____-_-_)
      {
        while (PIN_AX_READ == 0);           // Wait for the very first rising edge
      }
      
     /* 
      * PHASE 2: Address Bus Window Alignment
      * Introduces a BOOT_OFFSET delay to skip initial noise.
      * This aligns the execution window with a
      * known "idle gap" in the address bus activity, positioned 
      * immediately before the target memory-access cycle.
      *
      * BOOT_OFFSET:   |---------//---------|
      * AX LINE:        -_-_-_-//-_-_-_-__________-_-_-_
      * BUS IDLE:                       |--------|
      */
      _delay_ms(BOOT_OFFSET);         
      
       // Armed for hardware detectio
       EIFR |=(1 << INTF0);
      PIN_AX_INTERRUPT_RISING;
      PIN_AX_INTERRUPT_ENABLE;              
      
      while (patch_done != 1);                   // Wait for the first stage of the patch to complete:

    }

  #endif

  #ifdef INTERRUPT_FALLING

    ISR(PIN_AX_INTERRUPT_VECTOR) {
      pulse_counter++;                         
      if (pulse_counter == PULSE_COUNT){          
        _delay_us (BIT_OFFSET);                          
        PIN_DX_OUTPUT;                   
        _delay_us (OVERRIDE);                       
        PIN_DX_INPUT;                      
        PIN_AX_INTERRUPT_DISABLE;          

        pulse_counter = 0;                    
        patch_done = 1;                       
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
      
      _delay_ms(BOOT_OFFSET);        /
                  
      PIN_AX_INTERRUPT_FALLING;     
      PIN_AX_INTERRUPT_ENABLE;              
      
      while (patch_done != 1);                   
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

      PIN_AX_INTERRUPT_RISING;                     
      PIN_AX_INTERRUPT_ENABLE;              
      
      while (patch_done != 1);                  

      while (PIN_AY_READ != 0);             

      _delay_ms(FOLLOWUP_OFFSET);  

      PIN_AY_INTERRUPT_RISING;                    
      PIN_AY_INTERRUPT_ENABLE;            
      while (patch_done != 2);                 

    }

  #endif




#endif


