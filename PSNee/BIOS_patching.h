
#pragma once

#ifdef BIOS_PATCH

  volatile uint8_t pulse_counter = 0;
  volatile uint8_t patch_done = 0;

  #ifdef TEST_BIOS

    void Bios_Patching() {
      PIN_DX_INPUT;
      PIN_DX_LOW;
    
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

    /* 
     * PHASE 3: Zero-Latency Software Pulse Counting
     * Using manual polling to eliminate the jitter (0.5us) caused by ISR overhead.
     * cli() locks the CPU for cycle-accurate timing.
     */
    uint8_t current_pulses = 0;
    cli();                                  // Disable interrupts for timing integrity

    while (current_pulses < PULSE_COUNT) {
        // Wait for AX line to go HIGH (Target Rising Edge)
        while (PIN_AX_READ == 0); 
        current_pulses++;
        
        // If not the final pulse, wait for the line to go LOW before next loop
        if (current_pulses < PULSE_COUNT) {
            while (PIN_AX_READ != 0); 
        }
        // At the 48th pulse, we exit immediately to Phase 4
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
      
       // Armed for hardware detection
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


