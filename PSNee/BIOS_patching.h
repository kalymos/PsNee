#pragma once



#ifdef BIOS_PATCH

  volatile uint8_t pulse_counter = 0;
  volatile uint8_t patch_done = 0;


  #ifdef INTERRUPT_RISING

    ISR(PIN_AX_INTERRUPT_VECTOR) {

     /* 
      * PHASE 3: Pulse Counting (Inside ISR)
      * The hardware Interrupt Service Routine (ISR) now takes over.
      * It counts the exact number of incoming pulses on PIN_AX until it 
      * matches the PULSE_COUNT value.
      */


      //if (--pulse_counter == 0) {           
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

      if (PIN_AX_READ != 0) {                // Case: Power-on / Line high (---__-_-_)

        while (PIN_AX_READ != 0);           // Wait for falling edge
        while (PIN_AX_READ == 0);           // Wait for next rising edge to sync
      }
      else {                                // Case: Reset / Line low (_____-_-_)
        while (PIN_AX_READ == 0);           // Wait for the very first rising edge
      }
     /* 
      * PHASE 2: Reaching the Target Memory Window
      * We introduce a strategic delay (BOOT_OFFSET) to skip initial noise.
      * This points the execution to a known idle gap in the 
      * address range calls before the critical data appears.
      * DELAY: |---//-----|
      * AX:    -_-_//-_-_________-_-_-_ 
      */
      _delay_ms(BOOT_OFFSET);         
       // Armed for hardware detection
       //pulse_counter = PULSE_COUNT; 
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

      if (PIN_AX_READ != 0) {
        while (PIN_AX_READ != 0);           
        while (PIN_AX_READ == 0);           
      }
      else {
        while (PIN_AX_READ == 0);          
      }

      _delay_ms(BOOT_OFFSET);        

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
      
      if (pulse_counter == PULSE_COUNT_2) {
        _delay_us (BIT_OFFSET_2);                           
        PIN_DX_OUTPUT;                  
        _delay_us (OVERRIDE_2);                      
        PIN_DX_INPUT;                        
        PIN_AY_INTERRUPT_DISABLE;           
        patch_done = 2;                      
      }
    }

    void Bios_Patching(){
      if (PIN_AX_READ != 0) {
        while (PIN_AX_READ != 0);           
        while (PIN_AX_READ == 0);          
      }
      else {
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

/*
 * ======================================================================================
 * FUNCTION    : Bios_Patching()
 * TARGET      : Data Bus (DX) synchronized via Address Bus (AX / AY)
 * 
 * OPERATIONAL LOGIC:
 *    This function intercepts a specific memory transaction by counting clock cycles 
 *    on Address lines (AX/AY) to inject modified data onto the Data line (DX) 
 *    in real-time (On-the-fly patching).
 *
 * KEY PHASES:
 *    1. SYNC (AX): Aligns the CPU with the first valid address cycle after boot/reset 
 *       to establish a deterministic T0 reference point.
 *    2. GATING (BOOT_OFFSET): Skips initial BIOS noise/calls to reach the target 
 *       memory window.
 *    3. ADDRESS COUNTING (ISR AX): Hardware-based pulse counting using ultra-fast 
 *       decrement-to-zero logic to identify the exact target bit location.
 *    4. DATA OVERDRIVE (DX): Momentarily forces DX pin to OUTPUT mode to overwrite 
 *       the original BIOS bit with a custom logic state.
 *    5. SEQUENCING (Optional AY): Transitions to a secondary address line (AY) for 
 *       multi-stage patching or follow-up verification.
 * ======================================================================================
 */

#ifdef BIOS_PATCH_2

  volatile uint8_t pulse_counter = 0;
  volatile uint8_t patch_done = 0;

  // --- MAIN INTERRUPT SERVICE ROUTINE (ADDRESS AX) ---
  ISR(PIN_AX_INTERRUPT_VECTOR) {
    /* 
     * PHASE 3: Pulse Counting (Inside ISR)
     * Decrementing towards zero is the fastest operation on AVR architecture.
     */
    if (--pulse_counter == 0) {           
      
      /* PHASE 4: Precision Bit Alignment */
      _delay_us(BIT_OFFSET);                            
      
      /* PHASE 5: Data Bus Overdrive (The Patch on DX) */
      #ifdef INTERRUPT_RISING_HIGH_PATCH
        PIN_DX_SET;                         // Pre-set HIGH state for Variant 3
      #endif

      PIN_DX_OUTPUT;                        // Take control of the Data Bus
      _delay_us(OVERRIDE);                       
      
      #ifdef INTERRUPT_RISING_HIGH_PATCH
        PIN_DX_CLEAR;                       // Release HIGH state
      #endif

      PIN_DX_INPUT;                         // Immediately release the Data Bus
      PIN_AX_INTERRUPT_DISABLE;          
      patch_done = 1;                       // Notify Stage 1 completion
    }
  }

  // --- SECONDARY INTERRUPT SERVICE ROUTINE (ADDRESS AY - Variant 3) ---
  #ifdef INTERRUPT_RISING_HIGH_PATCH
    ISR(PIN_AY_INTERRUPT_VECTOR) {
      if (--pulse_counter == 0) {           
        _delay_us(BIT_OFFSET_2);                           
        PIN_DX_OUTPUT;                  
        _delay_us(OVERRIDE_2);                      
        PIN_DX_INPUT;                        
        PIN_AY_INTERRUPT_DISABLE;           
        patch_done = 2;                      // Notify Stage 2 completion
      }
    }
  #endif

  void Bios_Patching() {
    /* 
     * PHASE 1: Signal Stabilization & Alignment (AX) 
     * Handles Cold Boot (Line High) vs Reset (Line Low) states.
     */
    if (PIN_AX_READ != 0) {                // Case: Power-on / Line high
      while (PIN_AX_READ != 0);           // Wait for falling edge
      while (PIN_AX_READ == 0);           // Wait for next rising edge to sync
    }
    else {                                // Case: Reset / Line low
      while (PIN_AX_READ == 0);           // Wait for the very first rising edge
    }

    /* PHASE 2: Reaching the Target Memory Window */
    _delay_ms(BOOT_OFFSET);         

    // Countdown Preparation (Optimized for speed)
    pulse_counter = PULSE_COUNT; 
    patch_done = 0;

    // Dynamic Interrupt Configuration
    #if defined(INTERRUPT_RISING) || defined(INTERRUPT_RISING_HIGH_PATCH)
      PIN_AX_INTERRUPT_RISING;
    #elif defined(INTERRUPT_FALLING)
      PIN_AX_INTERRUPT_FALLING;
    #endif

    // Arm Hardware Interrupt
    PIN_AX_INTERRUPT_ENABLE;              
    while (patch_done != 1);              // Block until first patch is applied

    /* 
     * OPTIONAL PHASE: Secondary Patch (Variant 3 only) 
     * Switches detection to the second Address line (AY).
     */
    #ifdef INTERRUPT_RISING_HIGH_PATCH
      while (PIN_AY_READ != 0);           // Ensure AY is low before arming
      _delay_ms(FOLLOWUP_OFFSET);  

      pulse_counter = PULSE_COUNT_2;      // Re-load counter for AY pulses
      PIN_AY_INTERRUPT_RISING;                    
      PIN_AY_INTERRUPT_ENABLE;            
      while (patch_done != 2);            // Block until second patch is applied
    #endif
  }

#endif

