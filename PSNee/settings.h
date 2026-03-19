#pragma once


/*------------------------------------------------------------------------------------------------
 * BIOS PATCH PARAMETERS EXPLANATION
 * 
 * 1. SILENCE_THRESHOLD: 
 *    Defines the size of a polling block where the AX pin must remain LOW. 
 *    It serves as a "bus address call detector" for specific idle windows.
 * 
 * 2. CONFIRM_COUNTER_TARGET: 
 *    Defines the exact number of SILENCE_THRESHOLD blocks that must be validated 
 *    consecutively before arming the hardware interrupt (ISR).
 * 
 * 3. PULSE_COUNT: 
 *    The starting value for the 'impulse' down-counter. It counts physical 
 *    rising edges on the AX pin until it reaches zero to trigger the injection.
 * 
 * 4. BIT_OFFSET_CYCLES (Inside ISR): 
 *    The sub-microsecond delay (CPU cycles) applied after the final pulse to 
 *    precisely align the DX injection with the target bit in the memory cycle.
 * 
 * 5. OVERRIDE_CYCLES (Inside ISR): 
 *    The duration for which the DX pin is forced to OUTPUT mode to overwrite 
 *    the original BIOS data with the custom state.
 ------------------------------------------------------------------------------------------------*/


                                    // tested with  an Atmega328P
#if defined(ATmega32U4_16U4)

  // ------ SCPH 100 / 102 ------
  #if defined(SCPH_100) || \
      defined(SCPH_102)
    #define BIOS_PATCH
    #define SILENCE_THRESHOLD 1100
    #define CONFIRM_COUNTER_TARGET 8
    #define PULSE_COUNT 47         //47
    #define BIT_OFFSET_CYCLES 47   //60
    #define OVERRIDE_CYCLES 3       
  #endif


  // // -------- SCPH 7500 / 9000 --------
  #ifdef SCPH_7500_9000
    #define BIOS_PATCH
    #define SILENCE_THRESHOLD 1100
    #define CONFIRM_COUNTER_TARGET 1
    #define PULSE_COUNT 15          //15
    #define BIT_OFFSET_CYCLES 47    //60
    #define OVERRIDE_CYCLES 3       
  #endif


  // -------- SCPH 7000 --------
  #ifdef SCPH_7000
    #define BIOS_PATCH
    #define SILENCE_THRESHOLD 1100
    #define CONFIRM_COUNTER_TARGET 1
    #define PULSE_COUNT 15
    #define BIT_OFFSET_CYCLES 47
    #define OVERRIDE_CYCLES 3
  #endif


  // // ----- SCPH 3500 / 5000 / 5500 -----
  #ifdef SCPH_3500_5500
    #define BIOS_PATCH
    #define SILENCE_THRESHOLD 25000
    #define CONFIRM_COUNTER_TARGET 1
    #define PULSE_COUNT 84         //84
    #define BIT_OFFSET_CYCLES 47    //60
    #define OVERRIDE_CYCLES 3
  #endif

  // // -------- SCPH 3000 --------
  #ifdef SCPH_3000
    #define BIOS_PATCH
    #define PHASE_TWO_PATCH
    #define SILENCE_THRESHOLD 1100
    #define CONFIRM_COUNTER_TARGET 9
    #define PULSE_COUNT 59
    #define BIT_OFFSET_CYCLES 45
    #define OVERRIDE_CYCLES 3
    #define CONFIRM_COUNTER_TARGET_2 206 
    #define PULSE_COUNT_2 42
    #define BIT_OFFSET_2_CYCLES 48
    #define OVERRIDE_2_CYCLES 3
  #endif


  // // -------- SCPH 1000 --------
  #ifdef SCPH_1000
    #define BIOS_PATCH
    #define PHASE_TWO_PATCH
    #define SILENCE_THRESHOLD 1100
    #define CONFIRM_COUNTER_TARGET 9
    #define PULSE_COUNT 91             
    #define BIT_OFFSET_CYCLES 45      
    #define OVERRIDE_CYCLES 3
    #define CONFIRM_COUNTER_TARGET_2 222   
    #define PULSE_COUNT_2 70
    #define BIT_OFFSET_2_CYCLES 48
    #define OVERRIDE_2_CYCLES 3
  #endif

#else

  // ------ SCPH 100 / 102 ------
  #if defined(SCPH_100) || \
      defined(SCPH_102)
    #define BIOS_PATCH
    #define SILENCE_THRESHOLD 1500
    #define CONFIRM_COUNTER_TARGET 8
    #define PULSE_COUNT 47         //47
    #define BIT_OFFSET_CYCLES 47   //60
    #define OVERRIDE_CYCLES 3       
  #endif


  // // -------- SCPH 7500 / 9000 --------
  #ifdef SCPH_7500_9000
    #define BIOS_PATCH
    #define SILENCE_THRESHOLD 1500
    #define CONFIRM_COUNTER_TARGET 1
    #define PULSE_COUNT 15          //15
    #define BIT_OFFSET_CYCLES 47    //60
    #define OVERRIDE_CYCLES 3       
  #endif


  // -------- SCPH 7000 --------
  #ifdef SCPH_7000
    #define BIOS_PATCH
    #define SILENCE_THRESHOLD 1500
    #define CONFIRM_COUNTER_TARGET 1
    #define PULSE_COUNT 15
    #define BIT_OFFSET_CYCLES 47
    #define OVERRIDE_CYCLES 3
  #endif


  // // ----- SCPH 3500 / 5000 / 5500 -----
  #ifdef SCPH_3500_5500
    #define BIOS_PATCH
    #define SILENCE_THRESHOLD 32000
    #define CONFIRM_COUNTER_TARGET 1
    #define PULSE_COUNT 84         //84
    #define BIT_OFFSET_CYCLES 47    //60
    #define OVERRIDE_CYCLES 3
  #endif

  // // -------- SCPH 3000 --------
  #ifdef SCPH_3000
    #define BIOS_PATCH
    #define PHASE_TWO_PATCH
    #define SILENCE_THRESHOLD 1500
    #define CONFIRM_COUNTER_TARGET 9
    #define PULSE_COUNT 59
    #define BIT_OFFSET_CYCLES 45
    #define OVERRIDE_CYCLES 3
    #define CONFIRM_COUNTER_TARGET_2 206 
    #define PULSE_COUNT_2 42
    #define BIT_OFFSET_2_CYCLES 48
    #define OVERRIDE_2_CYCLES 3
  #endif


  // // -------- SCPH 1000 --------
  #ifdef SCPH_1000
    #define BIOS_PATCH
    #define PHASE_TWO_PATCH
    #define SILENCE_THRESHOLD 1500
    #define CONFIRM_COUNTER_TARGET 9
    #define PULSE_COUNT 91             
    #define BIT_OFFSET_CYCLES 45      
    #define OVERRIDE_CYCLES 3
    #define CONFIRM_COUNTER_TARGET_2 222   
    #define PULSE_COUNT_2 70
    #define BIT_OFFSET_2_CYCLES 48
    #define OVERRIDE_2_CYCLES 3
  #endif
#endif

  /*------------------------------------------------------------------------------------------------
                    Region Settings Section
  ------------------------------------------------------------------------------------------------*/

  #if defined(SCPH_100)       || \
      defined(SCPH_7500_9000) || \
      defined(SCPH_7000)      || \
      defined(SCPH_3500_5500) || \
      defined(SCPH_3500)      || \
      defined(SCPH_3000)      || \
      defined(SCPH_1000)      || \
      defined(SCPH_xxx3)      || \
      defined(SCPH_5903)

    #define INJECT_SCEx 0   // NTSC-J

  #elif defined(SCPH_xxx1)

    #define INJECT_SCEx 1   // NTSC-U/C 

  #elif defined(SCPH_xxx2) || \
        defined(SCPH_102)

    #define INJECT_SCEx 2   // PAL 
      
  #elif defined(SCPH_xxxx)

    #define INJECT_SCEx 3   // Universal: NTSC-J -> NTSC-U/C -> PAL

  #endif


/*------------------------------------------------------------------------------------------------
               serial debug section
------------------------------------------------------------------------------------------------*/

/******************************************************************************************
 * FUNCTION    : Debug_Log
 * 
 * DESCRIPTION : 
 *    Transmits hardware status and system configuration via the serial interface.
 *    Displays MCU frequency, WFCK detection mode, and active regional settings.
 * 
 *    - ATtiny: Uses a compact "Short-Code" format to minimize serial bus overhead.
 *    - Standard MCUs: Provides verbose, human-readable system diagnostics.
 * 
 * INPUT       : Wfck_mode (int) - The detected board generation (0: Static, 1: Frequency).
 ******************************************************************************************/
#if defined(PSNEE_DEBUG_SERIAL_MONITOR)

  void Debug_Log (int Wfck_mode){   

    #if  defined(ATtiny85_45_25)
      // --- COMPACT SYSTEM LOG (ATtiny) ---
      // Minimalistic output to save CPU cycles and maintain timing precision.
      mySerial.print("m "); 
      mySerial.println(Wfck_mode); // Mode indicator
    #else
      // --- VERBOSE DIAGNOSTICS (ATmega) ---
      // Detailed system information for standard development and debugging.
      Serial.print(" MCU frequency: "); 
      Serial.print(F_CPU); 
      Serial.println(" Hz");

      Serial.print(" wfck_mode: "); 
      Serial.println(Wfck_mode); // Board generation (0: Legacy, 1: Modern)

      Serial.print(" region: "); 
      Serial.print(region[0]); 
      Serial.print(region[1]); 
      Serial.println(region[2]); // Active injection string (e.g., SCEE)
    #endif
  }

/******************************************************************************************
 * FUNCTION    : LogSUBQ
 * 
 * DESCRIPTION : 
 *    Logs captured SUBQ frames to the serial interface.
 *    Timing is critical: the entire 12-byte frame must be processed and transmitted 
 *    within the ~12ms window before the next SUBQ packet arrives.
 * 
 *    - ATtiny: Uses minimal formatting (no spaces) to stay within the timing budget.
 *    - Standard MCUs: Includes spaces for better readability.
 * 
 * INPUT       : dataBuffer (uint8_t*) - Pointer to the 12-byte SUBQ frame.
 ******************************************************************************************/

void LogSUBQ(uint8_t *dataBuffer) {
  
  /** 
   * ERROR FILTERING:
   * A failed sector read usually results in zeroed data (excluding CRC).
   * Skip logging if the first 4 bytes are null to reduce bus traffic.
   */
  if (!(dataBuffer[0] == 0 && dataBuffer[1] == 0 && dataBuffer[2] == 0 && dataBuffer[3] == 0)) {

#if defined(ATtiny85_45_25)
    // --- COMPACT FORMATTING (ATtiny) ---
    // Minimal formatting to meet the strict 12ms timing constraint on slower MCUs.
    for (uint8_t i = 0; i < 12; i++) {
      if (dataBuffer[i] < 0x10) {
        mySerial.print("0"); // Leading zero padding for hex alignment
      }
      mySerial.print(dataBuffer[i], HEX);
    }
    mySerial.println("");

#else
    // --- STANDARD FORMATTING (ATmega) ---
    // More descriptive output with space separation for easier debugging.
    for (uint8_t i = 0; i < 12; i++) {
      if (dataBuffer[i] < 0x10) {
        Serial.print("0"); // Leading zero padding
      }
      Serial.print(dataBuffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println("");
#endif

  }
}

/******************************************************************************************
 * FUNCTION    : Debug_Inject
 * 
 * DESCRIPTION : 
 *    Provides real-time visual confirmation during SCEx region code injection.
 *    Used to verify that the hysteresis threshold has been met and the 
 *    injection sequence is active.
 * 
 *    - ATtiny: Prints a single '!' to minimize CPU blocking during the critical 
 *      injection timing window.
 *    - Standard MCUs: Prints a clear, verbose "INJECT !" message.
 ******************************************************************************************/
void DebugInject(){       

#if defined(ATtiny85_45_25)
    // --- MINIMALIST NOTIFICATION (ATtiny) ---
    mySerial.print("!"); 

#else
    // --- VERBOSE NOTIFICATION (ATmega) ---
    // Standard visual feedback for debugging and monitoring.
    Serial.println("           INJECT ! ");
#endif
}

#endif

/* 
 * =============================================================
 *           COMPILATION CHECKS
 * =============================================================
 */

// --- CONSOLE SELECTION CHECK ---
#if !defined(SCPH_1000)      && \
    !defined(SCPH_3000)      && \
    !defined(SCPH_3500_5500) && \
    !defined(SCPH_7000)      && \
    !defined(SCPH_7500_9000) && \
    !defined(SCPH_100)       && \
    !defined(SCPH_101)       && \
    !defined(SCPH_102)       && \
    !defined(SCPH_xxx1)      && \
    !defined(SCPH_xxx2)      && \
    !defined(SCPH_xxx3)      && \
    !defined(SCPH_5903)      && \
    !defined(SCPH_xxxx)
  #error "No console selected! Please uncomment your SCPH model."

#elif (defined(SCPH_1000)      + \
       defined(SCPH_3000)      + \
       defined(SCPH_3500_5500) + \
       defined(SCPH_7000)      + \
       defined(SCPH_7500_9000) + \
       defined(SCPH_100)       + \
       defined(SCPH_101)       + \
       defined(SCPH_102)       + \
       defined(SCPH_xxx1)      + \
       defined(SCPH_xxx2)      + \
       defined(SCPH_xxx3)      + \
       defined(SCPH_5903)      + \
       defined(SCPH_xxxx) > 1)
  #error "Multiple consoles selected! Please enable only one SCPH model."
#endif

// --- MCU SELECTION CHECK ---
#if !defined(ATmega328_168)   && \
    !defined(ATmega328_168PB) && \
    !defined(ATmega32U4_16U4) && \
    !defined(ATtiny85_45_25)
  #error "No MCU selected! Please choose one supported architecture."

#elif (defined(ATmega328_168)    + \
       defined(ATmega328_168PB)  + \
       defined(ATmega32U4_16U4)  + \
       defined(ATtiny85_45_25) > 1)
  #error "Multiple MCUs selected! Please enable only one architecture."
#endif

// --- RESOURCE CONFLICT CHECK (ATtiny) ---
#if defined(ATtiny85_45_25) && \
    defined(LED_RUN)        && \
    defined(PSNEE_DEBUG_SERIAL_MONITOR)
  #error "Resource conflict: LED_RUN and DEBUG_SERIAL are incompatible on ATtiny."
#endif

// --- Console Model Info ---
#if defined(SCPH_1000)
  #pragma message "Target Console: SCPH-1000 (NTSC-J)"
#elif defined(SCPH_3000)
  #pragma message "Target Console: SCPH-3000 (NTSC-J)"
#elif defined(SCPH_3500_5500)
  #pragma message "Target Console: SCPH-3500/5000/5500 (NTSC-J)"
#elif defined(SCPH_7000)
  #pragma message "Target Console: SCPH-7000 (Internal Switch enabled)"
#elif defined(SCPH_7500_9000)
  #pragma message "Target Console: SCPH-7500/9000 (NTSC-J)"
#elif defined(SCPH_100)
  #pragma message "Target Console: SCPH-100 (NTSC-J)"
#elif defined(SCPH_101)
  #pragma message "Target Console: SCPH-101 (NTSC-U/C)"
#elif defined(SCPH_102)
  #pragma message "Target Console: SCPH-102 (PAL)"
#elif defined(SCPH_xxx1)
  #pragma message "Target Console: Generic NTSC-U/C"
#elif defined(SCPH_xxx2)
  #pragma message "Target Console: Generic PAL"
#elif defined(SCPH_xxx3)
  #pragma message "Target Console: Generic NTSC-J"
#elif defined(SCPH_5903)
  #pragma message "Target Console: SCPH-5903 (Video CD Dual-Interface)"
#elif defined(SCPH_xxxx)
  #pragma message "Target Console: Universal Region Mode"
#endif

// --- MCU Architecture Info ---
#if defined(ATmega328_168)
  #pragma message "Microcontroller: ATmega328/168 (Arduino Nano/Uno)"
#elif defined(ATmega32U4_16U4)
  #pragma message "Microcontroller: ATmega32U4/16U4 (Leonardo/Pro Micro)"
#elif defined(ATtiny85_45_25)
  #pragma message "Microcontroller: ATtiny85/45/25"
#endif

// --- Feature Status ---

#ifdef PSNEE_DEBUG_SERIAL_MONITOR
  #pragma message "Feature: Serial Debug Monitor ENABLED"
#endif

#ifdef LED_RUN
  #pragma message "Feature: Status LED (PB5) ENABLED"
#endif
