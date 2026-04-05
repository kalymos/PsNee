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
    #define OVERRIDE_CYCLES 4
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
    #define BIT_OFFSET_CYCLES 43      
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

#if defined(DEBUG_SERIAL_MONITOR)
extern uint8_t request_counter;

  #if defined(IS_32U4_FAMILY)
    #define DEBUG_OUT Serial1
  #else
    #define DEBUG_OUT Serial
  #endif

void BoardDetectionLog (uint16_t window_result, uint8_t Wfck_mode, uint8_t region){          //Information about the MCU, and old or late console mode.

#if  defined(IS_ATTINY_FAMILY)
  mySerial.print("m "); mySerial.println(Wfck_mode);
#else
  static const char* regionNames[] = {
    "NTSC-J",    // 0
    "NTSC-U/C",  // 1
    "PAL",       // 2
    "Universal"  // 3
  };

  DEBUG_OUT.println("");
  DEBUG_OUT.print(F(" CPU Speed: ")); DEBUG_OUT.print(F_CPU / 1000000L); DEBUG_OUT.println(F(" MHz"));
  DEBUG_OUT.print(F(" Sync Window: ")); DEBUG_OUT.println(window_result); // Real-time diagnostic
  DEBUG_OUT.print(F(" WFCK Mode: ")); DEBUG_OUT.println(Wfck_mode);
  DEBUG_OUT.print(F(" Region ID: ")); DEBUG_OUT.println(regionNames[region]);
  DEBUG_OUT.println("");
#endif

}


void CaptureSUBQLog(uint8_t *dataBuffer) {
  static uint16_t errorCount = 0; // Tracks missed sectors between valid reads
  
  // Cache the first 4 bytes for a quick null check (failed read)
  uint8_t b0 = dataBuffer[0], b1 = dataBuffer[1], b2 = dataBuffer[2], b3 = dataBuffer[3];

  // --- ERROR FILTERING ---
  // If the sector is empty (all zeros), increment error counter and exit
  if (b0 == 0 && b1 == 0 && b2 == 0 && b3 == 0) {
    errorCount++; 
    return;
  }


  // --- DATA & request_counter DISPLAY ---
  #if defined(IS_ATTINY_FAMILY)
    // Compact hex output for ATtiny to maintain 12ms timing
    for (uint8_t i = 0; i < 12; i++) {
      uint8_t val = dataBuffer[i];
      if (val < 0x10) mySerial.print("0"); 
      mySerial.print(val, HEX);
    }
    // Append global request_counter on the same line
    mySerial.print(F(" h:")); 
    mySerial.println(request_counter); 
  #else
    // Formatted hex output for ATmega
    for (uint8_t i = 0; i < 12; i++) {
      uint8_t val = dataBuffer[i];
      if (val < 0x10) DEBUG_OUT.print("0");
      DEBUG_OUT.print(val, HEX); 
      DEBUG_OUT.print(" ");
    }
    // Append global request_counter on the same line
    DEBUG_OUT.print(F("| Hyst: ")); 
    DEBUG_OUT.println(request_counter); 
  #endif
}



void InjectLog(){     

  #if defined(IS_ATTINY_FAMILY)
    mySerial.print("!"); // --- MINIMALIST NOTIFICATION (ATtiny) ---
  #else
    DEBUG_OUT.println("           INJECT ! ");
  #endif
}

#endif

/*------------------------------------------------------------------------------------------------
               Compilation message
-----------------------------------------------------------------------------------------------*/
#if (REQUEST_INJECT_GAP >= REQUEST_INJECT_TRIGGER)
  #error "Critical Error: REQUEST_INJECT_GAP must be smaller than REQUEST_INJECT_TRIGGER!"
#endif

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
    defined(DEBUG_SERIAL_MONITOR)
  #error "Resource conflict: LED_RUN and DEBUG_SERIAL are incompatible on ATtiny."
#endif

// --- Feature Status ---

#ifdef DEBUG_SERIAL_MONITOR
  #pragma message "Feature: Serial Debug Monitor ENABLED"
#endif


