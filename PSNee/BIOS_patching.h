#pragma once

/*
 * ======================================================================================
 * FUNCTION    : Bios_Patching()
 * TARGET      : Data Bus (DX) synchronized via Address Bus (AX / AY)
 *
 * OPERATIONAL LOGIC:
 *    Intercepts specific memory transactions by monitoring address lines (AX/AY).
 *    Uses hardware interrupts (ISR) for high-speed pulse counting and cycle-accurate
 *    delays to inject modified data onto the Data line (DX) in real-time.
 *
 * KEY PHASES:
 *    1. STABILIZATION & ALIGNMENT (AX): Synchronizes execution with a clean rising 
 *       edge of the AX signal to establish a deterministic timing reference.
 *
 *    2. ADDRESS CALL DETECTION (PHASE 2): Scans the bus for specific address calls 
 *       by validating consecutive polling blocks (SILENCE_THRESHOLD).
 *
 *    3. SILENCE CONFIRMATION (GATING): Counts the exact number of validated silence 
 *       windows (CONFIRM_COUNTER_TARGET) to reach the correct pre-patching stage.
 *
 *    4. HARDWARE PULSE COUNTING (ISR): Uses INT0/INT1 to decrement the 'impulse' 
 *       counter on each rising edge with minimal hardware latency.
 *
 *    5. DATA OVERDRIVE (DX): Upon reaching the target pulse, triggers a calibrated 
 *       delay (BIT_OFFSET) and momentarily forces the DX pin to OUTPUT mode to 
 *       overwrite the BIOS bit for a precise duration (OVERRIDE).
 * ======================================================================================
 */

#ifdef BIOS_PATCH

/**
 * Shared state variables between ISRs and the main patching loop.
 * Declared 'volatile' to prevent compiler optimization during busy-wait loops.
 */
volatile uint8_t impulse = 0; // Down-counter for physical address pulses
volatile uint8_t patch = 0;   // Synchronization flag (0: Idle, 1: AX Done, 2: AY Done)

/**
 * PHASE 3: Primary Interrupt Service Routine (AX)
 * Triggered on rising edges to perform the real-time bus override.
 */
ISR(PIN_AX_INTERRUPT_VECTOR) {
    if (--impulse == 0) {
        // Precise bit-alignment delay within the memory cycle
        __builtin_avr_delay_cycles(BIT_OFFSET_CYCLES);

        #ifdef PHASE_TWO_PATCH
            PIN_DX_SET; // Pre-drive high if required by specific logic
        #endif

        // DATA OVERDRIVE: Pull the DX bus to the custom state
        PIN_DX_OUTPUT;
        __builtin_avr_delay_cycles(OVERRIDE_CYCLES);

        #ifdef PHASE_TWO_PATCH
            PIN_DX_CLEAR;
        #endif

        // BUS RELEASE: Return DX to High-Z (Input) mode
        PIN_DX_INPUT;

        PIN_AX_INTERRUPT_DISABLE; // Stop tracking AX pulses
        PIN_LED_OFF;
        patch = 1; // Signal Phase 3 completion
    }
}

#ifdef PHASE_TWO_PATCH
/**
 * PHASE 5: Secondary Interrupt Service Routine (AY)
 * Handles the second injection stage if multi-patching is active.
 */
ISR(PIN_AY_INTERRUPT_VECTOR) {
    if (--impulse == 0) {
        __builtin_avr_delay_cycles(BIT_OFFSET_2_CYCLES);

        PIN_DX_OUTPUT;
        __builtin_avr_delay_cycles(OVERRIDE_2_CYCLES);
        PIN_DX_INPUT;

        PIN_AY_INTERRUPT_DISABLE;
        PIN_LED_OFF;
        patch = 2; // Signal Phase 5 completion
    }
}
#endif

void Bios_Patching(void) {

    // --- HARDWARE BYPASS OPTION (SCPH-7000 specific) ---
    #if defined(SCPH_7000)
        PIN_SWITCH_INPUT;              // Configure Pin D5 as Input
        PIN_SWITCH_SET;                // Enable internal Pull-up (D5 defaults to HIGH)
        __builtin_avr_delay_cycles(10); // Short delay for voltage stabilization
        
        /** 
         * Exit immediately if the switch pulls the pin to GND (Logic LOW).
         * This allows the user to disable the BIOS patch on-the-fly.
         */
        if (PIN_SWITCH_READ == 0) { 
            return; 
        }
    #endif

    uint8_t current_confirms = 0;
    //uint16_t count;

    patch = 0;     // Reset sync flag
    sei();         // Enable Global Interrupts
    PIN_AX_INPUT;  // Set AX to monitor mode

    // --- PHASE 1: STABILIZATION & ALIGNMENT ---
    // Align execution pointer to a known rising edge state.
    if (PIN_AX_READ != 0) {
        while (WAIT_AX_FALLING); // Wait if bus is busy
        while (WAIT_AX_RISING);  // Sync with next pulse start
    } else {
        while (WAIT_AX_RISING);  // Sync with upcoming pulse
    }

    // --- PHASE 2: SILENCE DETECTION ---
    // Validate the exact number of silence windows to identify the boot stage.
    while (current_confirms < CONFIRM_COUNTER_TARGET) {
        uint16_t count = SILENCE_THRESHOLD; 
        while (count > 0) {
            if (PIN_AX_READ != 0) {
                while (WAIT_AX_FALLING);
                break; // Impulse detected: retry current silence block
            }
            #ifdef IS_32U4_FAMILY
            __asm__ __volatile__ ("nop"); 
            #endif

            count--;
        }
        if (count == 0) {
            current_confirms++; // Validated one silence window
        }
    }

    // --- PHASE 3: LAUNCH HARDWARE COUNTING (AX) ---
    impulse = PULSE_COUNT;
    PIN_LED_ON;
    PIN_AX_INTERRUPT_CLEAR;
    PIN_AX_INTERRUPT_RISING; // Setup rising-edge trigger
    PIN_AX_INTERRUPT_ENABLE; // Engage ISR

    while (patch != 1);


    // --- PHASE 4 & 5: SECONDARY PATCHING SEQUENCE ---
    #ifdef PHASE_TWO_PATCH
        PIN_AY_INPUT;
        current_confirms = 0;
        impulse = PULSE_COUNT_2;
        // Monitor for the specific silent gap before the second patch window
        while (current_confirms < CONFIRM_COUNTER_TARGET_2) {
            uint16_t count = SILENCE_THRESHOLD;
            while (count > 0) {
                if (PIN_AX_READ != 0) {
                    while (WAIT_AX_FALLING);
                    break;
                }

                #ifdef IS_32U4_FAMILY
                __asm__ __volatile__ ("nop"); 
                #endif

                count--;
            }
            if (count == 0) {
                current_confirms++;
            }
        }

        PIN_LED_ON;
        PIN_AY_INTERRUPT_CLEAR;
        PIN_AY_INTERRUPT_FALLING;
        PIN_AY_INTERRUPT_ENABLE;
    
        while (patch != 2); // Busy-wait for secondary ISR completion

        
        return;

    #endif

    
}
#endif


/*
 *  Portability Note: The non-ISR (polling-based) version of the code is maintained
 *  to facilitate porting to other platforms and architectures that may not support
 *  AVR-specific hardware interrupts.
 *
 *  Stability Limitation: While the polling version is more portable, it was found
 *  that at 16MHz, achieving consistent DATA OVERDRIVE stability is nearly impossible
 *  without using a Hardware ISR. The latency and jitter of software polling at this
 *  frequency are too high to guarantee a sub-microsecond cycle-accurate injection.
 *  Therefore, for ATmega328P @ 16MHz, the ISR-driven implementation remains the
 *  tandard.
 */

// #ifdef BIOS_PATCH

// volatile uint8_t impulse = 0;
// volatile uint8_t patch = 0;


//     ISR(PIN_AX_INTERRUPT_VECTOR) {
//         //impulse--;
//         if (--impulse == 0){           // If impulse reaches the value defined by TRIGGER, the following actions are performed:
//         // Precise cycle-accurate delay before triggering
//             __builtin_avr_delay_cycles(BIT_OFFSET_CYCLES);

//             #ifdef PHASE_TWO_PATCH
//                 PIN_DX_SET;
//             #endif

//             PIN_DX_OUTPUT; // Pull the line (Override start)
//             __builtin_avr_delay_cycles(OVERRIDE_CYCLES);

//             #ifdef PHASE_TWO_PATCH
//                 PIN_DX_CLEAR; // Release the bus (Override end)
//             #endif

//             PIN_DX_INPUT;

//             PIN_LED_OFF;
//             PIN_AX_INTERRUPT_DISABLE;
//             patch = 1;                       // patch is set to 1, indicating that the first patch is completed.
//         }
//     }

//     #ifdef PHASE_TWO_PATCH


//         ISR(PIN_AY_INTERRUPT_VECTOR){

//             //impulse--;
//             if (--impulse == 0)           // If impulse reaches the value defined by TRIGGER2, the following actions are performed:
//             {
//                 __builtin_avr_delay_cycles(BIT_OFFSET_2_CYCLES);

//                 PIN_DX_OUTPUT;
//                 __builtin_avr_delay_cycles(OVERRIDE_2_CYCLES);
//                 PIN_DX_INPUT;

//                 PIN_AY_INTERRUPT_DISABLE;
//                 PIN_LED_OFF;
//                 patch = 2;                       // patch is set to 2, indicating that the second patch is completed.
//             }
//         }
//     #endif

// // --- BIOS Patching Main Function ---
// void Bios_Patching(void) {

//     uint8_t current_confirms = 0;
//     uint8_t count = 0;
//     patch = 0;
//     sei();
//     PIN_AX_INPUT;
//   // --- PHASE 1: Signal Stabilization & Alignment (AX) ---
//   if (PIN_AX_READ != 0) {
//       while  (WAIT_AX_FALLING);  // Wait for falling edge
//       while  (WAIT_AX_RISING);   // Wait for next rising edge to sync
//   } else {
//       while (WAIT_AX_RISING);   // Wait for first rising edge
//   }

//   // --- PHASE 2: Silence Detection (AX) ---

//   while (current_confirms < CONFIRM_COUNTER_TARGET) {
//       uint16_t count = SILENCE_THRESHOLD;

//       // --- Scan for ONE continuous block of silence ---
//       while (count > 0) {
//           if (PIN_AX_READ != 0) {
//               while  (WAIT_AX_FALLING); // Pulse detected: wait for bus to clear
//               break;                  // Reset and try a new silence block
//           }
//           count--;
//       }

//       // If count reaches 0, a silent block is validated
//       if (count == 0) {
//           current_confirms++;
//       }
//   }
//     impulse = PULSE_COUNT;
//   PIN_LED_ON;
//     PIN_AX_INTERRUPT_RISING;
//     PIN_AX_INTERRUPT_ENABLE;
//     while (patch != 1);                   // Wait for the first stage of the patch to complete:

//         //PIN_LED_OFF;
//         // -------- Secondary Patch ----------
//         #ifdef PHASE_TWO_PATCH

//             current_confirms = 0;
//             while (current_confirms < CONFIRM_COUNTER_TARGET_2) {
//                 uint16_t count = SILENCE_THRESHOLD;

//                 while (count > 0) {
//                     if (PIN_AX_READ != 0) {

//                         while  (WAIT_AX_FALLING);
//                         break;
//                     }
//                     count--;
//                 }

//                 if (count == 0) {
//                     current_confirms++;
//                 }
//             }

//             PIN_LED_ON;
//             impulse = PULSE_COUNT_2;
//             PIN_AY_INTERRUPT_RISING;
//             PIN_AY_INTERRUPT_ENABLE;

//             while (patch != 2);                 // Wait for the second stage of the patch to complete:

//         #endif

//         cli();
//     }
// #endif


