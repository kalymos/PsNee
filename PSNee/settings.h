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

#ifdef  SCPH_102      
#define BIOS_PATCH_2
#define INTERRUPT_RISING
#define BOOT_OFFSET 83.9
#define PULSE_COUNT 48
//#define BIT_OFFSET 2.75
#define BIT_OFFSET 3.25 //BIOS_PATCH_2
#define OVERRIDE 0.2
#endif

#ifdef  SCPH_100        
#define BIOS_PATCH
#define INTERRUPT_RISING
#define BOOT_OFFSET 83.9              //83.72 - 84.15
#define PULSE_COUNT 48
#define BIT_OFFSET 2.75                     //2.63 - 2.87
#define OVERRIDE 0.2
#endif

#ifdef  SCPH_7500_9000    
#define BIOS_PATCH
#define INTERRUPT_RISING
#define BOOT_OFFSET 75.2              //74.95 - 75.55
#define PULSE_COUNT 16
#define BIT_OFFSET 2.8
#define OVERRIDE 0.2
#endif

#ifdef  SCPH_7000         
#define BIOS_PATCH
#define INTERRUPT_RISING
#define BOOT_OFFSET 75.2              //75.3
#define PULSE_COUNT 16
#define BIT_OFFSET 2.8
#define OVERRIDE 0.2
#endif

#ifdef  SCPH_5500         
#define BIOS_PATCH
#define INTERRUPT_FALLING
#define BOOT_OFFSET 76.07             //75.99 - 76.14
#define PULSE_COUNT 21
#define BIT_OFFSET 2.8
#define OVERRIDE 0.2
#endif

#ifdef  SCPH_5000        
#define BIOS_PATCH
#define INTERRUPT_FALLING
#define BOOT_OFFSET 75.2               //75.12 - 75.27
#define PULSE_COUNT 21
#define BIT_OFFSET 2.8                      //2.7 - 2.85
#define OVERRIDE 0.1
#endif

#ifdef  SCPH_3500        
#define BIOS_PATCH
#define INTERRUPT_FALLING
#define BOOT_OFFSET 75.2               //75.12 - 75.27
#define PULSE_COUNT 21
#define BIT_OFFSET 2.75                     //2.65  - 2.85
#define OVERRIDE 0.2
#endif

#ifdef  SCPH_3000
#define BIOS_PATCH    
#define INTERRUPT_RISING_HIGH_PATCH 
#define BOOT_OFFSET 82.9              //82.65 - 83.26
#define PULSE_COUNT 60                                               
#define BIT_OFFSET 2.7                     //2.58 - 2.8
#define OVERRIDE 0.15                                               
#define HIGH_PATCH
#define FOLLOWUP_OFFSET 253.3
#define PULSE_COUNT_2 43  
#define BIT_OFFSET_2 2.88
#define OVERRIDE_2 0.15
#endif

#ifdef  SCPH_1000              
#define BIOS_PATCH
#define INTERRUPT_RISING_HIGH_PATCH
#define BOOT_OFFSET 82.9              // 82.63 - 83.26
#define PULSE_COUNT 92
#define BIT_OFFSET 2.65                    // 2.58 - 2.75
#define OVERRIDE 0.15 
#define HIGH_PATCH
#define FOLLOWUP_OFFSET 272.8      
#define PULSE_COUNT_2 71
#define BIT_OFFSET_2 2.88
#define OVERRIDE_2 0.15
#endif

/*------------------------------------------------------------------------------------------------
                  Region Settings Section
------------------------------------------------------------------------------------------------*/

#if defined(SCPH_100) || defined(SCPH_7500_9000) || defined(SCPH_7000) || \
    defined(SCPH_5500) || defined(SCPH_5000) ||defined(SCPH_3500) || defined(SCPH_3000) || \
    defined(SCPH_1000) || defined(SCPH_xxx3) || defined(SCPH_5903)
    #define INJECT_SCEx 0   // NTSC-J
#endif

#if defined(SCPH_xxx1)
    #define INJECT_SCEx 1   // NTSC-U/C 
#endif

#if defined(SCPH_xxx2) || defined(SCPH_102)
    #define INJECT_SCEx 2   // PAL 
#endif

#if defined(SCPH_xxxx)
    #define INJECT_SCEx 3   // Universal: NTSC-J -> NTSC-U/C -> PAL
#endif



/*------------------------------------------------------------------------------------------------
               serial debug section
------------------------------------------------------------------------------------------------*/

#if defined(PSNEE_DEBUG_SERIAL_MONITOR)

void Debug_Log (int Wfck_mode){          //Information about the MCU, and old or late console mode.

#if  defined(ATtiny85_45_25)
  mySerial.print("m "); mySerial.println(Wfck_mode);
#elif !defined(ATtiny85_45_25)
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
#elif  !defined(ATtiny85_45_25)|| defined(SCPH_102_legacy) 
    Serial.println("           INJECT ! ");
#endif
}

#endif

/*------------------------------------------------------------------------------------------------
               Compilation message
-----------------------------------------------------------------------------------------------*/

#if !defined(SCPH_xxx3) && \
    !defined(SCPH_102) && !defined(SCPH_101) && !defined(SCPH_100) && !defined(SCPH_7500_9000) && \
    !defined(SCPH_7000) && !defined(SCPH_5500) && !defined(SCPH_5000) && !defined(SCPH_3500) && !defined(SCPH_3000) && \
    !defined(SCPH_1000) && !defined(SCPH_5903) &&\
    !defined(SCPH_xxx1) && !defined(SCPH_xxx2) && !defined(SCPH_xxxx)
 #error "Console not selected! Please uncoment #define with SCPH model number."
#elif !defined(SCPH_xxx3) ^ \
      defined(SCPH_102) ^ defined(SCPH_101) ^ defined(SCPH_100) ^ defined(SCPH_7500_9000) ^ \
      defined(SCPH_7000) ^ defined(SCPH_5500) ^ defined(SCPH_5000) ^ defined(SCPH_3500) ^ defined(SCPH_3000) ^ \
      defined(SCPH_1000) ^ defined(SCPH_xxxx) ^ defined(SCPH_5903) ^ \
      defined(SCPH_xxx1) ^ defined(SCPH_xxx2)
 #error "May be selected only one console! Please check #define with SCPH model number."
#endif

#if !defined(ATmega328_168) && !defined(ATmega32U4_16U4) && !defined(ATtiny85_45_25)
 #error "MCU not selected! Please choose one"
#elif !defined(ATmega328_168) ^ defined(ATmega32U4_16U4 ) ^ defined(ATtiny85_45_25)
 #error "May be selected only one MCU"
#endif

#if defined(LED_RUN) && defined(PSNEE_DEBUG_SERIAL_MONITOR) && defined(ATtiny85_45_25)
 #error"Compilation options LED_RUN and PSNEE_DEBUG_SERIAL_MONITOR are not simultaneously compatible with ATtiny85_45_25"
#endif
