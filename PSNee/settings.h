#pragma once

/*
 The _delay_us function uses loops to generate an approximate delay for the specified number of microseconds.
 It calculates the number of clock cycles required to achieve the requested delay and loops the corresponding number of times.
 
 The temporal precision of _delay_us depends on the microcontroller's clock frequency (F_CPU).
 For the ATmega328 operating at a typical frequency of 16 MHz, here are some details on the precision.

 Clock Frequency: F_CPU must be defined correctly before using the function. For an ATmega328 operating at 16 MHz:
    1 clock cycle = 1 / 16,000,000 s ≈ 62.5 ns
    1 µs ≈ 16 clock cycles

   BIT_OFFSET _delay_us(2.75)    = 44  clock cycles
   OVERRIDE _delay_us(0.2) = 3,2 clock cycles

*/

/*------------------------------------------------------------------------------------------------
           Specific parameter section for BIOS patches
------------------------------------------------------------------------------------------------*/

                                    // Results of the  maximum values
                                    // tested with  an Atmega328P

#if defined(IS_32U4_FAMILY)

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
#endif

#if defined(IS_328_168_FAMILY)

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

#if defined(PSNEE_DEBUG_SERIAL_MONITOR)

void Debug_Log (uint16_t debounce, int Wfck_mode){          //Information about the MCU, and old or late console mode.

#if  defined(IS_ATTINY_FAMILY)
  mySerial.print("m "); mySerial.println(Wfck_mode);
#else
  Serial.print(" MCU frequency: "); Serial.print(F_CPU); Serial.println(" Hz");
  Serial.print(" debounce: "); Serial.println(debounce);
  Serial.print(" wfck_mode: "); Serial.println(Wfck_mode);
  Serial.print(" region: "); Serial.print(region[0]); Serial.print(region[1]); Serial.println(region[2]);
#endif
}

  // log SUBQ packets. We only have 12ms to get the logs written out. Slower MCUs get less formatting.
void Debug_Scbuf (uint8_t *Scbuf){         // Data from the DATA bus
#if defined(IS_ATTINY_FAMILY)
  if (!(Scbuf[0] == 0 && Scbuf[1] == 0 && Scbuf[2] == 0 && Scbuf[3] == 0)) { // a bad sector read is all 0 except for the CRC fields. Don't log it.
    for (int i = 0; i < 12; i++) {
      if (Scbuf[i] < 0x10) {
        mySerial.print("0"); // padding
    #if  defined(IS_ATTINY_FAMILY)
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
#endif
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

  #if defined(IS_ATTINY_FAMILY)
      // --- COMPACT FORMATTING (ATtiny) ---
      // Minimal formatting to meet the strict 12ms timing constraint on slower MCUs.
      for (uint8_t i = 0; i < 12; i++) {
        if (dataBuffer[i] < 0x10) {
          mySerial.print("0"); // Leading zero padding for hex alignment

        }
        mySerial.print(Scbuf[i, HEX]);
      }
    mySerial.println("");
    }
  #elif !defined(IS_ATTINY_FAMILY)
    if (!(Scbuf[0] == 0 && Scbuf[1] == 0 && Scbuf[2] == 0 && Scbuf[3] == 0)) {
      for (int i = 0; i < 12; i++) {
        if (Scbuf[i] < 0x10) {
          Serial.print("0"); // padding
        }
        Serial.print(Scbuf[i], HEX);
        Serial.print(" ");
      }
      Serial.println("");
    }
  #endif
  }

void Debug_Inject(){       // Confirmation of region code injection


#if defined(IS_ATTINY_FAMILY)
    // --- MINIMALIST NOTIFICATION (ATtiny) ---
    mySerial.print("!"); 
#else
    // --- VERBOSE NOTIFICATION (ATmega) ---
    // Standard visual feedback for debugging and monitoring.
    Serial.println("           INJECT ! ");
#endif
}

#endif

/*------------------------------------------------------------------------------------------------
               Compilation message
-----------------------------------------------------------------------------------------------*/

#ifdef LED_RUN
  #pragma message "Feature: Status LED ENABLED"
#endif

// SECURITY CHECK: Ensure only one console is selected
// If you get "not portable" warnings here, it's only because multiple models are active.
#if (defined(SCPH_1000) + defined(SCPH_3000) + defined(SCPH_3500_5500) + \
     defined(SCPH_7000) + defined(SCPH_7500_9000) + defined(SCPH_100) + \
     defined(SCPH_102) + defined(SCPH_xxx1) + defined(SCPH_xxx2) + \
     defined(SCPH_xxx3) + defined(SCPH_5903) + defined(SCPH_xxxx)) > 1
  #error "Too many consoles selected! Please uncomment ONLY ONE model."
#endif

// Show target console.
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
#elif defined(SCPH_102)
  #pragma message "Target Console: SCPH-102 (PAL)"
#elif defined(SCPH_xxx1)
  #pragma message "Target Console: SCPH_xxx1 Generic NTSC-U/C"
#elif defined(SCPH_xxx2)
  #pragma message "Target Console: SCPH_xxx2 Generic PAL"
#elif defined(SCPH_xxx3)
  #pragma message "Target Console: SCPH_xxx3 Generic NTSC-J"
#elif defined(SCPH_5903)
  #pragma message "Target Console: SCPH-5903 (Video CD Dual-Interface)"
#elif defined(SCPH_xxxx)
  #pragma message "Target Console: SCPH_xxxx Universal Region Mode"
#else
  // Error if no console is uncommented
  #error "Console not selected! Please uncomment one SCPH model."
#endif

// Show detected microcontroller (e.g. __AVR_atmega328p__)
#define STRINGIZE(x) #x
#define TO_STR(x) STRINGIZE(x)

#ifdef __AVR_DEVICE_NAME__
  #pragma message "Microcontroller: __AVR_" TO_STR(__AVR_DEVICE_NAME__) "__"
#endif

// --- RESOURCE CONFLICT CHECK (ATtiny) ---
#if defined(IS_ATTINY_FAMILY) && \
    defined(LED_RUN)        && \
    defined(PSNEE_DEBUG_SERIAL_MONITOR)
  #error "Resource conflict: LED_RUN and DEBUG_SERIAL are incompatible on ATtiny."
#endif

// --- Feature Status ---

#ifdef PSNEE_DEBUG_SERIAL_MONITOR
  #pragma message "Feature: Serial Debug Monitor ENABLED"
#endif


