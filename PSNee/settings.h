#pragma once

/*
 The _delay_us function uses loops to generate an approximate delay for the specified number of microseconds.
 It calculates the number of clock cycles required to achieve the requested delay and loops the corresponding number of times.
 
 The temporal precision of _delay_us depends on the microcontroller's clock frequency (F_CPU).
 For the ATmega328 operating at a typical frequency of 16 MHz, here are some details on the precision.


*/

/*------------------------------------------------------------------------------------------------
           Specific parameter section for BIOS patches
------------------------------------------------------------------------------------------------*/
                                    // Results of the  maximum values

                                    // tested with  an Atmega328P



// ------ SCPH 100 / 102 ------
#if defined(SCPH_100) || \
    defined(SCPH_102)

  #define BIOS_PATCH
  #define SILENCE_THRESHOLD 1400
  #define CONFIRM_COUNTER_TARGET 8
  #define PULSE_COUNT 47         
  #define BIT_OFFSET_CYCLES 67   //60+7
  #define OVERRIDE_CYCLES 3       
#endif
// #endif

// // -------- SCPH 7500 / 9000 --------
#ifdef SCPH_7500_9000
  #define BIOS_PATCH
  #define SILENCE_THRESHOLD 1400
  #define CONFIRM_COUNTER_TARGET 1
  #define PULSE_COUNT 15
  #define BIT_OFFSET_CYCLES 66    //60+
  #define OVERRIDE_CYCLES 4       
#endif


// -------- SCPH 7000 --------
#ifdef SCPH_7000
  #define BIOS_PATCH
  #define SILENCE_THRESHOLD 1400
  #define CONFIRM_COUNTER_TARGET 1
  #define PULSE_COUNT 15
  #define BIT_OFFSET_CYCLES 66
  #define OVERRIDE_CYCLES 4
#endif


// // ----- SCPH 5000 / 5500 -----
#ifdef SCPH_5000_5500
  #define BIOS_PATCH
  #define SILENCE_THRESHOLD 35000
  #define CONFIRM_COUNTER_TARGET 1
  #define PULSE_COUNT 84
  #define BIT_OFFSET_CYCLES 60
  #define OVERRIDE_CYCLES 3
#endif


// // // -------- SCPH 5000 --------
// #ifdef SCPH_5000
// #define BIOS_PATCH_3
// #define INTERRUPT_RISING
// #define SILENCE_THRESHOLD 35000
// #define CONFIRM_COUNTER_TARGET 1
// #define PULSE_COUNT 84
// #define BIT_OFFSET_CYCLES 60
// #define OVERRIDE_CYCLES 3
// #endif
// // #endif

// // -------- SCPH 3500 --------
#ifdef SCPH_3500
  #define BIOS_PATCH
  #define SILENCE_THRESHOLD 34000
  #define CONFIRM_COUNTER_TARGET 1
  #define PULSE_COUNT 85
  #define BIT_OFFSET_CYCLES 44
  #define OVERRIDE_CYCLES 3
#endif


// // -------- SCPH 3000 --------
#ifdef SCPH_3000
  #define BIOS_PATCH
  #define INTERRUPT_RISING_HIGH_PATCH
  #define SILENCE_THRESHOLD 1200
  #define CONFIRM_COUNTER_TARGET 9
  #define PULSE_COUNT 60
  #define BIT_OFFSET_CYCLES 46
  #define OVERRIDE_CYCLES 3
  #define SILENCE_THRESHOLD_2 1200
  #define CONFIRM_COUNTER_TARGET_2 206 
  #define PULSE_COUNT_2 43
  #define BIT_OFFSET_2_CYCLES 54
  #define OVERRIDE_2_CYCLES 2
#endif


// // -------- SCPH 1000 --------
#ifdef SCPH_1000
  #define BIOS_PATCH
  #define INTERRUPT_RISING_HIGH_PATCH
  #define SILENCE_THRESHOLD 1300
  #define CONFIRM_COUNTER_TARGET 9
  #define PULSE_COUNT 91             
  #define BIT_OFFSET_CYCLES 58      
  #define OVERRIDE_CYCLES 3
  #define CONFIRM_COUNTER_TARGET_2 222   
  #define PULSE_COUNT_2 71
  #define BIT_OFFSET_2_CYCLES 54
  #define OVERRIDE_2_CYCLES 3
#endif
// #endif
/*------------------------------------------------------------------------------------------------
                  Region Settings Section
------------------------------------------------------------------------------------------------*/

#if defined(SCPH_100)       || \
    defined(SCPH_7500_9000) || \
    defined(SCPH_7000)      || \
    defined(SCPH_5000_5500) || \
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

void Debug_Log (int Wfck_mode){          //Information about the MCU, and old or late console mode.

#if  defined(ATtiny85_45_25)
  mySerial.print("m "); mySerial.println(Wfck_mode);
#else
  Serial.print(" MCU frequency: "); Serial.print(F_CPU); Serial.println(" Hz");
  //Serial.print(" lows: "); Serial.println(Lows);
  Serial.print(" wfck_mode: "); Serial.println(Wfck_mode);
  Serial.print(" region: "); Serial.print(region[0]); Serial.print(region[1]); Serial.println(region[2]);
#endif
}

// Logs SUBQ packets to serial. We only have 12ms to write logs before the next packet.
// Slower MCUs (like ATtiny) receive minimal formatting to save cycles.
void logSubQ(uint8_t *dataBuffer) {
  
  // A bad sector read results in zeros (except for CRC). Skip logging if first 4 bytes are 0.
  if (!(dataBuffer[0] == 0 && dataBuffer[1] == 0 && dataBuffer[2] == 0 && dataBuffer[3] == 0)) {

#if defined(ATtiny85_45_25)
    // Compact formatting for ATtiny to meet the 12ms timing constraint
    for (uint8_t i = 0; i < 12; i++) {
      if (dataBuffer[i] < 0x10) {
        mySerial.print("0"); // Leading zero padding
      }
      mySerial.print(dataBuffer[i], HEX);
    }
    mySerial.println("");

#else
    // Standard formatting with spaces for more powerful MCUs
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


void Debug_Inject(){       // Confirmation of region code injection

#if defined(ATtiny85_45_25)
    mySerial.print("!");
#else
    Serial.println("           INJECT ! ");
#endif
}

#endif

/*------------------------------------------------------------------------------------------------
               Compilation message
-----------------------------------------------------------------------------------------------*/

#if !defined(SCPH_xxx3)      && \
    !defined(SCPH_102)       && \
    !defined(SCPH_101)       && \
    !defined(SCPH_100)       && \
    !defined(SCPH_7500_9000) && \
    !defined(SCPH_7000)      && \
    !defined(SCPH_5000_5500) && \
    !defined(SCPH_3500)      && \
    !defined(SCPH_3000)      && \
    !defined(SCPH_1000)      && \
    !defined(SCPH_5903)      && \
    !defined(SCPH_xxx1)      && \
    !defined(SCPH_xxx2)      && \
    !defined(SCPH_xxxx)
 #error "Console not selected! Please uncoment #define with SCPH model number."
#elif !defined(SCPH_xxx3)      ^ \
       defined(SCPH_102)       ^ \
       defined(SCPH_101)       ^ \
       defined(SCPH_100)       ^ \
       defined(SCPH_7500_9000) ^ \
       defined(SCPH_7000)      ^ \
       defined(SCPH_5000_5500) ^ \
       defined(SCPH_3500)      ^ \
       defined(SCPH_3000)      ^ \
       defined(SCPH_1000)      ^ \
       defined(SCPH_xxxx)      ^ \
       defined(SCPH_5903)      ^ \
       defined(SCPH_xxx1)      ^ \
       defined(SCPH_xxx2)
 #error "May be selected only one console! Please check #define with SCPH model number."
#endif

#if !defined(ATmega328_168)   && \
    !defined(ATmega32U4_16U4) && \
    !defined(ATtiny85_45_25) && \
    !defined(RP2040)
 #error "MCU not selected! Please choose one"
#elif !defined(ATmega328_168)    ^ \
       defined(ATmega32U4_16U4 ) ^ \
       defined(ATtiny85_45_25 ) ^ \
       defined(RP2040)
 #error "May be selected only one MCU"
#endif

#if defined(LED_RUN)                    && \
    defined(PSNEE_DEBUG_SERIAL_MONITOR) && \
    defined(ATtiny85_45_25)
 #error"Compilation options LED_RUN and PSNEE_DEBUG_SERIAL_MONITOR are not simultaneously compatible with ATtiny85_45_25"
#endif
