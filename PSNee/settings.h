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

#ifdef  SCPH_102_legacy
//#define SCEE
#define BIOS_PATCH
#define SATBILIZATIONPOINT 100
#define DELAYPOINT 1350
#define HOLD _delay_us(17)
#define PATCHING _delay_us(4)
#endif

#ifdef  SCPH_102
//#define SCEE
#define BIOS_PATCH
#define HOLD _delay_us(2.75)
#define PATCHING _delay_us(0.2)
#define CHECKPOINT 83900
#define TRIGGER 48
#endif

#ifdef  SCPH_100
//#define SCEI
#define BIOS_PATCH
#define HOLD _delay_us(2.7)
#define PATCHING _delay_us(0.2)
#define CHECKPOINT 83900
#define TRIGGER 48
#endif

#ifdef  SCPH_7000_9000
//#define SCEI
#define BIOS_PATCH
#define HOLD _delay_us(2.85) 
#define PATCHING _delay_us(0.1)
#define CHECKPOINT 75270
#define TRIGGER 16
#endif

#ifdef  SCPH_5500
//#define SCEI
#define BIOS_PATCH
#define HOLD _delay_us(2.85)
#define PATCHING _delay_us(0.1)
#define CHECKPOINT 76130
#define TRIGGER 21
#define LOW_TRIGGER
#endif

#ifdef  SCPH_3500_5000
//#define SCEI
#define BIOS_PATCH
#define HOLD _delay_us(2.85)
#define PATCHING _delay_us(0.1)
#define CHECKPOINT 75260
#define TRIGGER 21
#define LOW_TRIGGER
#endif

#ifdef  SCPH_3000
//#define SCEI
#define BIOS_PATCH                                                    
#define HOLD _delay_us(2.75)
#define PATCHING _delay_us(0.1)
#define CHECKPOINT 83000      
#define TRIGGER 60                                               
//#define DOUBLE_PATCH 
//#define LOW_TRIGGER2
#define HIGH_PATCH
#define HOLD2 _delay_us(2.88)
#define PATCHING2 _delay_us(0.15)
#define CHECKPOINT2 253300
#define TRIGGER2 43  
#endif

#ifdef  SCPH_1000
//#define SCEI
#define BIOS_PATCH
#define HOLD _delay_us(2.7)
#define PATCHING _delay_us(0.1)
#define CHECKPOINT 83000
#define TRIGGER 92
//#define DOUBLE_PATCH
//#define LOW_TRIGGER2
#define HIGH_PATCH
#define HOLD2 _delay_us(2.88)
#define PATCHING2 _delay_us(0.15)
#define CHECKPOINT2 272800
#define TRIGGER2 71
#endif

#if defined(SCPH_xxx15)
const char region[3] = {'a', 'a', 'a'};
#endif

#if defined(SCPH_102)  || defined(SCPH_xxx2)
const char region[3] = {'e', 'e', 'e'};
#endif

#if defined(SCPH_100) || defined(SCPH_7000_9000) || defined(SCPH_5500) || defined(SCPH_3500_5000) || defined(SCPH_3000) || defined(SCPH_1000) || defined(SCPH_xxx3)
const char region[3] = {'i', 'i', 'i'};
#endif

#if defined(SCPH_xxxx)
const char region[3] = {'a', 'e', 'i'};
#endif

//All models have bad behavior below 11, PU-41 can start to have bad behavior beyond 20, for fat models we can go up to 60
// #if  defined(SCPH_102) || defined(SCPH_102_legacy) || defined(SCPH_xxxx) || defined(SCPH_xxx1_15) || defined(SCPH_xxx2_15) || defined(SCPH_xxx3_15)    
// #define HYSTERESIS_MAX 15
// #endif

// #if  defined(SCPH_7000_9000) || defined(SCPH_5500) || defined(SCPH_3500_5000) || defined(SCPH_3000) || defined(SCPH_1000) || defined(SCPH_xxxx_25) || defined(SCPH_xxx1_25) || defined(SCPH_xxx2_25)   || defined(SCPH_xxx3_25)  
// #define HYSTERESIS_MAX 25 
// #endif


#if defined(PSNEE_DEBUG_SERIAL_MONITOR)

void Debug_Log (int Lows, int Wfck_mode){

#if  defined(ATtiny85_45_25)
   mySerial.print("m "); mySerial.println(Wfck_mode);
#elif !defined(ATtiny85_45_25)
  //DEBUG_PRINT("highs: "); DEBUG_PRINT(highs); 
   Serial.print(" lows: "); Serial.println(Lows);
   Serial.print(" wfck_mode: "); Serial.println(Wfck_mode);
   Serial.print(" region: "); Serial.print(region[0]); Serial.print(region[1]); Serial.println(region[2]);
  // Power saving
  // Disable the ADC by setting the ADEN bit (bit 7)  of the ADCSRA register to zero.
  ADCSRA = ADCSRA & B01111111;
  // Disable the analog comparator by setting the ACD bit (bit 7) of the ACSR register to one.
  ACSR = B10000000;
  // Disable digital input buffers on all analog input pins by setting bits 0-5 of the DIDR0 register to one.
  DIDR0 = DIDR0 | B00111111;
#endif
}

  // log SUBQ packets. We only have 12ms to get the logs written out. Slower MCUs get less formatting.
void Debug_Scbuf (uint8_t *Scbuf){
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

void Debug_Inject(){

#if defined(ATtiny85_45_25)
    mySerial.print("!");
#elif  !defined(ATtiny85_45_25)
    Serial.println("           INJECT ! ");
#endif
}

#endif

#if !defined(SCPH_103) && \
    !defined(SCPH_102) && !defined(SCPH_101) && !defined(SCPH_100) && !defined(SCPH_7000_9000) && \
    !defined(SCPH_5500) && !defined(SCPH_3500_5000) && !defined(SCPH_3000) && \
    !defined(SCPH_1000) && !defined(SCPH_xxxx) && !defined(SCPH_102_legacy) && \
    !defined(SCPH_xxx1) && !defined(SCPH_xxx2) && !defined(SCPH_xxxx3)
 #error "Console not selected! Please uncoment #define with SCPH model number."
#elif !defined(SCPH_103) ^ \
      defined(SCPH_102) ^ defined(SCPH_101) ^ defined(SCPH_100) ^ defined(SCPH_7000_9000) ^ \
      defined(SCPH_5500) ^ defined(SCPH_3500_5000) ^ defined(SCPH_3000) ^ \
      defined(SCPH_1000) ^ defined(SCPH_xxxx) ^ defined(SCPH_102_legacy) ^ \
      defined(SCPH_xxx1) ^ defined(SCPH_xxx2) ^ defined(SCPH_xxx3)
 #error "May be selected only one console! Please check #define with SCPH model number."
#endif
