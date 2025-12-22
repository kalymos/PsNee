#pragma once

/*
 The _delay_us function uses loops to generate an approximate delay for the specified number of microseconds.
 It calculates the number of clock cycles required to achieve the requested delay and loops the corresponding number of times.
 
 The temporal precision of _delay_us depends on the microcontroller's clock frequency (F_CPU).
 For the ATmega328 operating at a typical frequency of 16 MHz, here are some details on the precision.

 Clock Frequency: F_CPU must be defined correctly before using the function. For an ATmega328 operating at 16 MHz:
    1 clock cycle = 1 / 16,000,000 s ≈ 62.5 ns
    1 µs ≈ 16 clock cycles

   HOLD _delay_us(2.75)    = 44  clock cycles
   PATCHING _delay_us(0.2) = 3,2 clock cycles

*/

/*------------------------------------------------------------------------------------------------
           Specific parameter section for BIOS patches
------------------------------------------------------------------------------------------------*/

#ifdef  SCPH_102_legacy
#define BIOS_PATCH
//#define INTERRUPT_RISING
#define SATBILIZATIONPOINT 100
#define DELAYPOINT 1350
#define HOLD _delay_us(17)
#define PATCHING _delay_us(4)
#endif

#ifdef  SCPH_102
#define BIOS_PATCH
#define INTERRUPT_RISING
#define CHECKPOINT 83.9
#define TRIGGER 48
#define HOLD 2.75
#define PATCHING 0.2
#endif

#ifdef  SCPH_100
#define BIOS_PATCH
#define INTERRUPT_RISING
#define CHECKPOINT 83.9
#define TRIGGER 48
#define HOLD 2.8
#define PATCHING 0.2
#endif

#ifdef  SCPH_7500_9000
#define BIOS_PATCH
#define INTERRUPT_RISING
#define CHECKPOINT 75.2 // ms SCPH_9000 74.95-75.55.
#define TRIGGER 16
#define HOLD 2.8
#define PATCHING 0.2
#endif

#ifdef  SCPH_7000
#define BIOS_PATCH
//#define PATCH_SWITCH
#define INTERRUPT_RISING
#define CHECKPOINT 74.7
#define TRIGGER 16
#define HOLD 2.75
#define PATCHING 0.6
#endif

#ifdef  SCPH_5500
#define BIOS_PATCH
#define INTERRUPT_FALLING
#define CHECKPOINT 76.13
#define TRIGGER 21
#define HOLD 2.8
#define PATCHING 0.15
#endif

#ifdef  SCPH_3500_5000
#define BIOS_PATCH
#define INTERRUPT_FALLING
#define CHECKPOINT 75.26
#define TRIGGER 21
#define HOLD 2.85
#define PATCHING 0.1
#endif

#ifdef  SCPH_3000
#define BIOS_PATCH
#define INTERRUPT_RISING_HIGH_PATCH     
#define CHECKPOINT 83   
#define TRIGGER 60                                               
#define HOLD 2.75
#define PATCHING 0.1                                               
#define HIGH_PATCH
#define CHECKPOINT2 253300
#define TRIGGER2 43  
#define HOLD2 2.88
#define PATCHING2 0.15
#endif

#ifdef  SCPH_1000
#define BIOS_PATCH
#define INTERRUPT_RISING_HIGH_PATCH
#define CHECKPOINT 83
#define TRIGGER 92
#define HOLD 2.7
#define PATCHING 0.1
#define HIGH_PATCH
#define CHECKPOINT2 27.28
#define TRIGGER2 71
#define HOLD2 2.88
#define PATCHING2 0.15
#endif

/*------------------------------------------------------------------------------------------------
                  Region Settings Section
------------------------------------------------------------------------------------------------*/

#if defined(SCPH_xxx1)                            //  NTSC U/C    | America.
const char region[3] = {'a', 'a', 'a'};
#endif

#if defined(SCPH_102)  || defined(SCPH_xxx2)    //  PAL         | Europ.
const char region[3] = {'e', 'e', 'e'};
#endif

#if defined(SCPH_100) || defined(SCPH_7500_9000) || defined(SCPH_7000) || defined(SCPH_5500) || defined(SCPH_3500_5000) || defined(SCPH_3000) || defined(SCPH_1000) || defined(SCPH_xxx3)     //  NTSC J      | Asia.
const char region[3] = {'i', 'i', 'i'};
#endif



/*------------------------------------------------------------------------------------------------
               serial debug section
------------------------------------------------------------------------------------------------*/

#if defined(PSNEE_DEBUG_SERIAL_MONITOR)

void Debug_Log (uint16_t Lows, int Wfck_mode){          //Information about the MCU, and old or late console mode.

#if  defined(ATtiny85_45_25)
  mySerial.print("m "); mySerial.println(Wfck_mode);
#elif !defined(ATtiny85_45_25)
  Serial.print(" MCU frequency: "); Serial.print(F_CPU); Serial.println(" Hz");
  Serial.print(" lows: "); Serial.println(Lows);
  Serial.print(" wfck_mode: "); Serial.println(Wfck_mode);
  Serial.print(" region: "); Serial.print(region[0]); Serial.print(region[1]); Serial.println(region[2]);
#endif
}

  // log SUBQ packets. We only have 12ms to get the logs written out. Slower MCUs get less formatting.
void Debug_Scbuf (uint8_t *Scbuf){         // Data from the DATA bus
#if defined(ATtiny85_45_25)
  if (!(Scbuf[0] == 0 && Scbuf[1] == 0 && Scbuf[2] == 0 && Scbuf[3] == 0)) { // a bad sector read is all 0 except for the CRC fields. Don't log it.
    for (int i = 0; i < 12; i++) {
      if (Scbuf[i] < 0x10) {
        mySerial.print("0"); // padding
      }
      mySerial.print(Scbuf[i, HEX]);
    }
   mySerial.println("");
  }
#elif !defined(ATtiny85_45_25)
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
    !defined(SCPH_7000) && !defined(SCPH_5500) && !defined(SCPH_3500_5000) && !defined(SCPH_3000) && \
    !defined(SCPH_1000) &&  \
    !defined(SCPH_xxx1) && !defined(SCPH_xxx2)
 #error "Console not selected! Please uncoment #define with SCPH model number."
#elif !defined(SCPH_xxx3) ^ \
      defined(SCPH_102) ^ defined(SCPH_101) ^ defined(SCPH_100) ^ defined(SCPH_7500_9000) ^ \
      defined(SCPH_7000) ^ defined(SCPH_5500) ^ defined(SCPH_3500_5000) ^ defined(SCPH_3000) ^ \
      defined(SCPH_1000) ^ defined(SCPH_xxxx)  ^ \
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
