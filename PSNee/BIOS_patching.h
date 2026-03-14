#pragma once
 
/*
 * ======================================================================================
 * FUNCTION    : Bios_Patching()
 * TARGET      : Data Bus (DX) synchronized via Address Bus (AX / AY)
 * 
 * OPERATIONAL LOGIC:
 *    Intercepts specific memory transactions by monitoring address lines (AX/AY).
 *    Uses precise pulse counting and timed delays to inject modified data onto 
 *    the Data line (DX) in real-time (On-the-fly patching).
 *
 * KEY PHASES:
 *    1. STABILIZATION & ALIGNMENT (AX): Ensures the CPU is synchronized with a 
 *       clean rising edge of the AX signal to establish a stable reference point.
 *
 *    2. SILENCE DETECTION (GATING): Scans for a specific window of bus inactivity 
 *       (SILENCE_THRESHOLD) to identify the correct pre-patching state and 
 *       avoid false triggers from boot noise.
 *
 *    3. PULSE COUNTING (AX): Implements a high-speed countdown of address pulses.
 *       The code blocks on each edge to maintain cycle-accurate synchronization 
 *       with the target memory access.
 *
 *    4. DATA OVERDRIVE (DX): At the exact target pulse, triggers a calibrated 
 *       delay (BIT_OFFSET) before forcing the DX pin to OUTPUT mode to 
 *       overwrite the original BIOS bit for a specific duration (OVERRIDE).
 *
 *    5. SECONDARY PATCH (AY): If enabled, repeats the silence detection and 
 *       pulse counting logic on a secondary address line (AY) for multi-stage 
 *       memory patching.
 * ======================================================================================
 */


#ifdef BIOS_PATCH

// --- BIOS Patching Main Function ---
void Bios_Patching(void) {

  uint8_t pulse_counter = 0;
  uint8_t current_confirms = 0;

  PIN_AX_INPUT;
  // --- PHASE 1: Signal Stabilization & Alignment (AX) ---
  if (PIN_AX_READ != 0) {        
      while  WAIT_AX_FALLING;  // Wait for falling edge
      while  WAIT_AX_RISING;   // Wait for next rising edge to sync
  } else {                       
      while WAIT_AX_RISING;   // Wait for first rising edge
  }

  // --- PHASE 2: Silence Detection (AX) ---

  while (current_confirms < CONFIRM_COUNTER_TARGET) {
      uint16_t count = SILENCE_THRESHOLD; 
      
      // --- Scan for ONE continuous block of silence ---
      while (count > 0) {
          if (PIN_AX_READ != 0) { 
              while  WAIT_AX_FALLING; // Pulse detected: wait for bus to clear
              break;                  // Reset and try a new silence block
          }
          count--;
      }

      // If count reaches 0, a silent block is validated
      if (count == 0) {
          current_confirms++;
      }
  }

  PIN_LED_ON;

  // --- PHASE 3: Pulse Counting & Patch 1 (AX) ---
  pulse_counter = PULSE_COUNT;


  while (pulse_counter > 0) {
    while WAIT_AX_RISING;   // Block here until rising edge
    
    pulse_counter--;

    if (pulse_counter == 0) {
      // Precise cycle-accurate delay before triggering
        __builtin_avr_delay_cycles(BIT_OFFSET_CYCLES);

        #ifdef INTERRUPT_RISING_HIGH_PATCH
            PIN_DX_SET;
        #endif

        PIN_DX_OUTPUT; // Pull the line (Override start)
        __builtin_avr_delay_cycles(OVERRIDE_CYCLES);

        #ifdef INTERRUPT_RISING_HIGH_PATCH
            PIN_DX_CLEAR; // Release the bus (Override end)
        #endif

        PIN_DX_INPUT;
        
        PIN_LED_OFF; 
        break; 
    }

    while  WAIT_AX_FALLING;  // Wait for pulse to clear
  }

  //PIN_LED_OFF;
  // -------- Secondary Patch ----------
  #ifdef INTERRUPT_RISING_HIGH_PATCH

    current_confirms = 0;
    while (current_confirms < CONFIRM_COUNTER_TARGET_2) {
        uint16_t count = SILENCE_THRESHOLD; 
        
        while (count > 0) {
            if (PIN_AX_READ != 0) { 

                while  WAIT_AX_FALLING; 
                break; 
            }
            count--;
        }

        if (count == 0) {
            current_confirms++;
        }
    }

    PIN_LED_ON;
    pulse_counter = PULSE_COUNT_2;

    while (pulse_counter > 0) {
      while WAIT_AY_RISING;
      
      pulse_counter--;

      if (pulse_counter == 0) {
        __builtin_avr_delay_cycles(BIT_OFFSET_2_CYCLES);

        PIN_DX_OUTPUT;
        __builtin_avr_delay_cycles(OVERRIDE_2_CYCLES);
        PIN_DX_INPUT;
        
        PIN_LED_OFF;
        break;
      }
      while WAIT_AY_FALLING; 

    #endif
  }
#endif

#ifdef BIOS_PATCH_4

volatile uint8_t patch_done = 0;

void Bios_Patching(void) {
    // PHASE 1: Sync (unchanged)
    if (PIN_AX_READ) {
        while (PIN_AX_READ);
        while (!PIN_AX_READ);
    } else {
        while (!PIN_AX_READ);
    }

    // PHASE 2: Silence Detection
    uint8_t confirms = 0;
    while (confirms < CONFIRM_COUNTER_TARGET) {
        uint16_t silence = SILENCE_THRESHOLD;
        uint8_t valid = 1;
        
        while (silence) {
            if (PIN_AX_READ) {
                while (PIN_AX_READ);
                valid = 0;
                break;
            }
            silence--;
        }
        
        if (valid) confirms++;
    }
    
    PIN_LED_ON;
    
    // PHASE 3: Pulse counting AX
    uint8_t pulses = PULSE_COUNT;
    uint8_t prev = (PIN_AX_READ != 0);
    
    while (pulses) {
        uint8_t curr = (PIN_AX_READ != 0);
        if (!prev && curr) {
            pulses--;
            if (!pulses) {
                __builtin_avr_delay_cycles(BIT_OFFSET_CYCLES);
                PIN_DX_OUTPUT;
                __builtin_avr_delay_cycles(OVERRIDE_CYCLES);
                PIN_DX_INPUT;
                break;
            }
        }
        prev = curr;
    }
    
    // PHASE 4: Optional AY patch
    #ifdef INTERRUPT_RISING_HIGH_PATCH
    
        while (PIN_AY_READ);
        _delay_ms(FOLLOWUP_OFFSET_MS);
        
        pulses = PULSE_COUNT_2;
        prev = (PIN_AY_READ != 0);
        
        while (pulses) {
            uint8_t curr = (PIN_AY_READ != 0);
            if (!prev && curr) {
                pulses--;
                if (!pulses) {
                    __builtin_avr_delay_cycles(BIT_OFFSET_2_CYCLES);
                    PIN_DX_OUTPUT;
                    __builtin_avr_delay_cycles(OVERRIDE_2_CYCLES);
                    PIN_DX_INPUT;
                    break;
                }
            }
            prev = curr;
        }
    
    #endif
    
    patch_done = 1;
}

#endif

