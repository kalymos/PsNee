//                           PSNee-8.7.0

/*------------------------------------------------------------------------------------------------
                            MCU selection
------------------------------------------------------------------------------------------------*/

//       MCU               //     Arduino
//------------------------------------------------------------------------------------------------
//#define ATmega328_168    //  Nano, Pro Mini, Uno
//#define ATmega32U4_16U4  //  Micro, Pro Micro
//#define ATtiny85_45_25   //  ATtiny

/*------------------------------------------------------------------------------------------------
                          Console selection
--------------------------------------------------------------------------------------------------

   No BIOS patching. 
   You can use injection via USB.

   SCPH model number //  region code | region
-------------------------------------------------------------------------------------------------*/
//#define SCPH_xxx1  //  NTSC U/C    | America.
//#define SCPH_xxx2  //  PAL         | Europ.
//#define SCPH_xxx3  //  NTSC J      | Asia.
//#define SCPH_xxxx  //              | All mode works the same as V7, but that's not what I recommend the most.


// Models that require a BIOS patch.

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
                              Caution!

 It is only possible to inject the code via ISP! 

 For the patch to work, the BIOS version is more important than the SCPH number.
 
 The delay in starting up caused by the bootloader of the Arduino cards prevents the injection of the BIOS patch within the delay,
 that's why you have to use the ISP which eliminates the bootloader.
 
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX


                                      |                Adres pin            |
     SCPH model number    // Data pin |    32-pin BIOS   |   40-pin BIOS    | BIOS version
-------------------------------------------------------------------------------------------------*/
//#define SCPH_102        // DX - D0  | AX - A7          |                  | 4.4e - CRC 0BAD7EA9, 4.5e -CRC 76B880E5
//#define SCPH_102_legacy // ! works in progress DX - D2, AX - A18.         | 4.4e - CRC 0BAD7EA9, 4.5e -CRC 76B880E5
//#define SCPH_100        // DX - D0  | AX - A7          |                  | 4.3j - CRC F2AF798B
//#define SCPH_7000_9000  // DX - D0  | AX - A7          |                  | 4.0j - CRC EC541CD0
//#define SCPH_5500       // DX - D0  | AX - A5          |                  | 3.0j - CRC FF3EEB8C
//#define SCPH_3500_5000  // DX - D0  | AX - A5          | AX - A4          | 2.2j - CRC 24FC7E17, 2.1j - CRC BC190209
//#define SCPH_3000       // DX - D5  | AX - A7, AY - A8 | AX - A6, AY - A7 | 1.1j - CRC 3539DEF6
//#define SCPH_1000       // DX - D5  | AX - A7, AY - A8 | AX - A6, AY - A7 | 1.0j - CRC 3B601FC8

/*------------------------------------------------------------------------------------------------
                           Options
------------------------------------------------------------------------------------------------*/

#define LED_RUN         // Turns on the LED when injections occur.
//                         D13 for Arduino, ATtiny add a led between PB3 (pin 2) and gnd with a 1k resistor in series, ATmega32U4 (Pro Micro) add a led between PB6 (pin 10) and gnd with a 1k resistor in series.

//#define PATCH_SWITCH  // Enables hardware support for disabling BIOS patching.
//                         With SCPH_7000 - 9000 models, Bios 4.0j, the bios patch prevents reading memory cards in the console interface, and in some cases can cause a crash (No problem in game).
//                         In rare cases where the BIOS patch prevents the playback of original games.

//#define PSNEE_DEBUG_SERIAL_MONITOR  // Enables serial monitor output. 
/*                                       Requires compilation with Arduino libs!
                                         For Arduino connect TXD and GND, for ATtiny PB3 (pin 2) and GND, to your serial card RXD and GND.
                                           Arduino   |   ATtiny
                                           ---------------------
                                           TXD--RXD  |  PB3--RXD
                                           GND--GND  |  GND--GND */

/*------------------------------------------------------------------------------------------------
                          Hysteresis 
------------------------------------------------------------------------------------------------*/
#define HYSTERESIS_MAX 15         // All model.  
//#define HYSTERESIS_MAX 25         // Only FAT! For models with problematic CD players.
//           Determines the number of times the data in the DATA line must match the filter of the region code injection function to trigger the injection.

/*------------------------------------------------------------------------------------------------
            Summary of practical information. Fuses. Pinout
------------------------------------------------------------------------------------------------*/

/*  
  Fuses  
  MCU    | High | Low | Extended
  --------------------------------------------------
  ATmega | DF   | EE  | FF 
  ATtiny | DF   | E2  | FF

  Pinout
  Arduino   | PSNee     |
  ---------------------------------------------------
  VCC       | VCC       |
  GND       | GND       |
  RST       | RESET     | Only for JAP_FAT
  D2        | BIOS AX   | Only for Bios patch
  D3        | BIOS AY   | Only for BIOS patch ver, 1.0j, 1.1j
  D4        | BIOS DX   | Only for Bios patch
  D5        | SWITCH    | Optional for disabling Bios patch
  D6        | SQCK      |
  D7        | SUBQ      |
  D8        | DATA      |
  D9        | WFCK |
  D13 ^ D10 | LED       | D10 only for ATmega32U4_16U4

  ATtiny | PSNee        | ISP   |
  ---------------------------------------------------
  Pin1   |              | RESET |
  Pin2   | LED ^ serial |       | serial only for PSNEE_DEBUG_SERIAL_MONITOR
  Pin3   | WFCK         |       |
  Pin4   | GND          | GND   |
  Pin5   | SQCK         | MOSI  |
  Pin6   | SUBQ         | MISO  |
  Pin7   | DATA         | SCK   |
  Pin8   | VCC          | VCC   |
*/

/*------------------------------------------------------------------------------------------------
                         pointer and variable section
------------------------------------------------------------------------------------------------*/

#include "MCU.h"
#include "settings.h"
#include "BIOS_patching.h"

//Initializing values ​​for region code injection timing
#define DELAY_BETWEEN_BITS 4000      // 250 bits/s (microseconds) (ATtiny 8Mhz works from 3950 to 4100) PU-23 PU-22 MAX 4250 MIN 3850
#define DELAY_BETWEEN_INJECTIONS 90  // The sweet spot is around 80~100. For all observed models, the worst minimum time seen is 72, and it works well up to 250.

//Creation of the different variables for the counter
volatile uint8_t count_isr = 0;
volatile uint32_t microsec = 0;
volatile uint32_t millisec = 0;

//Flag initializing for automatic console generation selection 0 = old, 1 = pu-22 end  ++
volatile bool wfck_mode = 0;

volatile bool Flag_Switch = 0;

/*------------------------------------------------------------------------------------------------
                         Code section
------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------
 Interrupt Service Routine: CTC_TIMER_VECTOR
 Description: 
 This ISR is triggered by the Timer/Counter Compare Match event. It increments time-related 
 counters used for tracking microseconds and milliseconds.

 Functionality:
 - Increments `microsec` by 10 on each interrupt call.
 - Increments `count_isr` to keep track of the number of interrupts.
 - When `count_isr` reaches 100, it means 1 millisecond has elapsed:
     - `millisec` is incremented.
     - `count_isr` is reset to 0.

 Notes:
 - This method provides a simple way to maintain a software-based timekeeping system.
------------------------------------------------------------------------------------------------*/
ISR(CTC_TIMER_VECTOR) {
  microsec += 10;                    
  count_isr++;                  
  if (count_isr == 100)              
  {
    millisec++;
    count_isr = 0;
  }
}

// *****************************************************************************************
// Function: Timer_Start
// Description: 
// This function initializes and starts the timer by resetting the timer counter register 
// and enabling timer interrupts. It ensures compatibility across multiple microcontrollers.
//
// Supported Microcontrollers:
// - ATmega328/168
// - ATmega32U4/16U4
// - ATtiny85/45/25
//
// Functionality:
// - Clears the timer counter to ensure a fresh start.
// - Enables the timer interrupt to allow periodic execution of ISR routines.
// - If BIOS_PATCH is defined, it also clears the timer interrupt flag to prevent 
//   unwanted immediate interrupts.
//
// Notes:
// - The actual timer configuration is handled in MCU.h.
// - This function ensures that all supported MCUs behave consistently.
//
// *****************************************************************************************
void Timer_Start() {
#if defined(ATmega328_168) || defined(ATmega32U4_16U4) || defined(ATtiny85_45_25)
  TIMER_TCNT_CLEAR;
  TIMER_INTERRUPT_ENABLE;
  #if defined(BIOS_PATCH)
    TIMER_TIFR_CLEAR;
  #endif
#endif
}

// *****************************************************************************************
// Function: Timer_Stop
// Description: 
// Stops the timer by disabling interrupts and resetting the timer counter. 
// It also clears the time tracking variables (count_isr, microsec, millisec) 
// to ensure a fresh start when the timer is restarted.
//
// Supported Microcontrollers:
// - ATmega328/168
// - ATmega32U4/16U4
// - ATtiny85/45/25
//
// *****************************************************************************************
void Timer_Stop() {
  
  #if defined(ATmega328_168) || defined(ATmega32U4_16U4) || defined(ATtiny85_45_25)
    TIMER_INTERRUPT_DISABLE;  // Disable timer interrupts to stop counting
    TIMER_TCNT_CLEAR;         // Reset the timer counter to ensure proper timing when restarted
  #endif
  // Reset time tracking variables
  count_isr = 0;
  microsec = 0;
  millisec = 0;
}

// *****************************************************************************************
// Function: readBit
// Description: 
// Reads a specific bit from an array of bytes.
// This function helps retrieve SCEX data efficiently while working within 
// the constraints of Harvard architecture.
//
// Parameters:
// - index: The bit position to read within the byte array.
// - ByteSet: A pointer to the byte array containing the data.
//
// Return:
// - Returns 1 if the specified bit at the given index is set (1).
// - Returns 0 if the specified bit is cleared (0).
//
// Explanation:
// - The function determines which byte contains the requested bit using (index / 8).
// - It then calculates the bit position within that byte using (index % 8).
// - A bitwise AND operation extracts the bit's value, and the double NOT (!!) operator 
//   ensures a clean boolean return value (1 or 0).
//
// *****************************************************************************************
uint8_t readBit(uint8_t index, const uint8_t* ByteSet) {
  return !!(ByteSet[index / 8] & (1 << (index % 8)));  // Return true if the specified bit is set in ByteSet[index]
}


// *****************************************************************************************
// Function: inject_SCEX
// Description: 
// Injects SCEX data corresponding to a given region ('e' for Europe, 'a' for America, 
// 'i' for Japan). This function is used for modulating the SCEX signal to bypass 
// region-locking mechanisms.
//
// Parameters:
// - region: A character ('e', 'a', or 'i') representing the target region.
//
// *****************************************************************************************
void inject_SCEX(const char region) {
  // SCEX data patterns for different regions (SCEE, SCEA, SCEI)
  static const uint8_t SCEEData[] = {
    0b01011001,
    0b11001001,
    0b01001011,
    0b01011101,
    0b11101010,
    0b00000010
  };

  static const uint8_t SCEAData[] = {
    0b01011001,
    0b11001001,
    0b01001011,
    0b01011101,
    0b11111010,
    0b00000010
  };

  static const uint8_t SCEIData[] = {
    0b01011001,
    0b11001001,
    0b01001011,
    0b01011101,
    0b11011010,
    0b00000010
  };

  // Iterate through 44 bits of SCEX data
  for (uint8_t bit_counter = 0; bit_counter < 44; bit_counter++) {
    // Check if the current bit is 0
    if (readBit(bit_counter, region == 'e' ? SCEEData : region == 'a' ? SCEAData : SCEIData) == 0) {
      PIN_DATA_OUTPUT;         
      PIN_DATA_CLEAR;   
      _delay_us(DELAY_BETWEEN_BITS);  // Wait for specified delay between bits
    }
    else {
      // modulate DATA pin based on WFCK_READ
      if (wfck_mode)  // WFCK mode (pu22mode enabled): synchronize PIN_DATA with WFCK clock signal
      {
        PIN_DATA_OUTPUT;
        Timer_Start();
        do {
          // Read the WFCK pin and set or clear DATA accordingly
          if (PIN_WFCK_READ) {
            PIN_DATA_SET; 
          }

          else {
            PIN_DATA_CLEAR;  
          }
        }
        while (microsec < DELAY_BETWEEN_BITS);
        Timer_Stop();  // Stop the timer after the delay
      }
      // PU-18 or lower mode: simply set PIN_DATA as input with a delay
      else {
        PIN_DATA_INPUT;
        _delay_us(DELAY_BETWEEN_BITS);
      }
    }
  }
  // After injecting SCEX data, set DATA pin as output and clear (low)
  PIN_DATA_OUTPUT;
  PIN_DATA_CLEAR;
  _delay_ms(DELAY_BETWEEN_INJECTIONS);
}

void Init() {
#if defined(ATmega328_168) || defined(ATmega32U4_16U4) || defined(ATtiny85_45_25)
  TIMER_TCNT_CLEAR;
  SET_OCROA_DIV;
  SET_TIMER_TCCROA;
  SET_TIMER_TCCROB;
#endif

#if defined(ATmega328_168)
  // Power saving
  // Disable the ADC by setting the ADEN bit (bit 7)  of the ADCSRA register to zero.
  ADCSRA = ADCSRA & B01111111;
  // Disable the analog comparator by setting the ACD bit (bit 7) of the ACSR register to one.
  ACSR = B10000000;
  // Disable digital input buffers on all analog input pins by setting bits 0-5 of the DIDR0 register to one.
  DIDR0 = DIDR0 | B00111111;
#endif

#if defined(PATCH_SWITCH) && defined(BIOS_PATCH)
  PIN_SWITCH_INPUT;
  PIN_SWITCH_SET;
  if (PIN_SWITCH_READ == 0){
   Flag_Switch =1;
  }
#endif

#ifdef LED_RUN
  PIN_LED_OUTPUT;
#endif

  GLOBAL_INTERRUPT_ENABLE;
  
  PIN_SQCK_INPUT;
  PIN_SUBQ_INPUT;

#if defined(PSNEE_DEBUG_SERIAL_MONITOR) && defined(ATtiny85_45_25)
  //pinMode(debugtx, OUTPUT); // software serial tx pin
  mySerial.begin(115200); // 13,82 bytes in 12ms, max for softwareserial. (expected data: ~13 bytes / 12ms) // update: this is actually quicker
#elif defined(PSNEE_DEBUG_SERIAL_MONITOR) && !defined(ATtiny85_45_25)
  Serial.begin(500000); // 60 bytes in 12ms (expected data: ~26 bytes / 12ms) // update: this is actually quicker
#endif
}

int main() {
  uint8_t  hysteresis = 0;
  uint8_t  scbuf[12] = { 0 };             // SUBQ bit storage
  uint16_t timeout_clock_counter = 0;
  uint8_t  bitbuf = 0;
  uint8_t  bitpos = 0;
  uint8_t  scpos = 0;                     // scbuf position
  uint16_t lows = 0;  

  Init();

#ifdef BIOS_PATCH

#ifdef LED_RUN
  PIN_LED_ON;
#endif

  if (Flag_Switch == 0) {
    #ifdef SCPH_102_legacy
      Bios_Patching_SCPH_102_legacy();
    #else
      Bios_Patching();
    #endif
  }

#ifdef LED_RUN
  PIN_LED_OFF;
#endif

#endif

  Timer_Start();
  /*----------------------------------------------------------------------
   Board detection
  
   WFCK: __-----------------------  // this is a PU-7 .. PU-20 board!
  
        __-_-_-_-_-_-_-_-_-_-_-_-  // this is a PU-22 or newer board!
  
   typical readouts PU-22: highs: 2449 lows: 2377
  
  -----------------------------------------------------------------------*/
  do {
    if (PIN_WFCK_READ == 0) lows++;             // good for ~5000 reads in 1s
    _delay_us(200);
  } 
  while (millisec < 1000);                     // sample 1s

  Timer_Stop();

  if (lows > 100) {
    wfck_mode = 1;                             //flag pu22mode
  }

  else {
    wfck_mode = 0;                             //flag oldmod
  }

#if defined(PSNEE_DEBUG_SERIAL_MONITOR)
 Debug_Log(lows, wfck_mode);
#endif

  while (1) {

    _delay_ms(1); /* Start with a small delay, which can be necessary 
                    in cases where the MCU loops too quickly and picks up the laster SUBQ trailing end*/

    GLOBAL_INTERRUPT_DISABLE;      // start critical section

    // Capture 8 bits for 12 runs > complete SUBQ transmission
    do {
      for (bitpos = 0; bitpos < 8; bitpos++) {
        while (PIN_SQCK_READ != 0)  // wait for clock to go low
        {
          timeout_clock_counter++;  
          // a timeout resets the 12 byte stream in case the PSX sends malformatted clock pulses, as happens on bootup
          if (timeout_clock_counter > 1000) {
            scpos = 0;                  
            timeout_clock_counter = 0;  
            bitbuf = 0;                 
            bitpos = 0;                 
            continue;
          }
        }

        // Wait for clock to go high
        while (PIN_SQCK_READ == 0);  

        if (PIN_SUBQ_READ)              // If clock pin high
        {
          bitbuf |= 1 << bitpos;  // Set the bit at position bitpos in the bitbuf to 1. Using OR combined with a bit shift
        }

        timeout_clock_counter = 0;  // no problem with this bit
      }

      scbuf[scpos] = bitbuf;  // One byte done
      scpos++;
      bitbuf = 0;
    }

    while (scpos < 12);             // Repeat for all 12 bytes

    GLOBAL_INTERRUPT_ENABLE;  // End critical section




#if defined(PSNEE_DEBUG_SERIAL_MONITOR)
  Debug_Scbuf(scbuf);
#endif

    /*-------------------------------------------------------------------------------
     Check if read head is in wobble area
     We only want to unlock game discs (0x41) and only if the read head is in the outer TOC area.
     We want to see a TOC sector repeatedly before injecting (helps with timing and marginal lasers).
     All this logic is because we don't know if the HC-05 is actually processing a getSCEX() command.
     Hysteresis is used because older drives exhibit more variation in read head positioning.
     While the laser lens moves to correct for the error, they can pick up a few TOC sectors.
    -------------------------------------------------------------------------------*/

    //This variable initialization macro is to replace (0x41) with a filter that will check that only the three most significant bits are correct. 0x001xxxxx
    uint8_t isDataSector = (((scbuf[0] & 0x40) == 0x40) && (((scbuf[0] & 0x10) == 0) && ((scbuf[0] & 0x80) == 0)));

    if (
      (isDataSector && scbuf[1] == 0x00 && scbuf[6] == 0x00) &&       // [0] = 41 means psx game disk. the other 2 checks are garbage protection
      (scbuf[2] == 0xA0 || scbuf[2] == 0xA1 || scbuf[2] == 0xA2 ||    // if [2] = A0, A1, A2 ..
       (scbuf[2] == 0x01 && (scbuf[3] >= 0x98 || scbuf[3] <= 0x02)))  // .. or = 01 but then [3] is either > 98 or < 02
    ) {
      hysteresis++;
    }

    // This CD has the wobble into CD-DA space. (started at 0x41, then went into 0x01)
    else if (hysteresis > 0 && ((scbuf[0] == 0x01 || isDataSector) && (scbuf[1] == 0x00 /*|| scbuf[1] == 0x01*/) && scbuf[6] == 0x00)) {
      hysteresis++;  
    }

    // None of the above. Initial detection was noise. Decrease the counter.
    else if (hysteresis > 0) {
      hysteresis--;  
    }

    // hysteresis value "optimized" using very worn but working drive on ATmega328 @ 16Mhz
    // should be fine on other MCUs and speeds, as the PSX dictates SUBQ rate
    if (hysteresis >= HYSTERESIS_MAX) {
      // If the read head is still here after injection, resending should be quick.
      // Hysteresis naturally goes to 0 otherwise (the read head moved).
      hysteresis = 11;

#ifdef LED_RUN
  PIN_LED_ON;
#endif

/*-------------------------------------------------------------------------------
        Executes the region code injection sequence.
-------------------------------------------------------------------------------*/

      PIN_DATA_OUTPUT;  
      PIN_DATA_CLEAR;   

      if (!wfck_mode)  // If wfck_mode is fals (oldmode)
      {
        PIN_WFCK_OUTPUT;  
        PIN_WFCK_CLEAR;  
      }

      _delay_ms(DELAY_BETWEEN_INJECTIONS);  // HC-05 waits for a bit of silence (pin low) before it begins decoding.

      // inject symbols now. 2 x 3 runs seems optimal to cover all boards
      for (uint8_t scex = 0; scex < 2; scex++) {
        inject_SCEX(region[scex]);
      }

      if (!wfck_mode)  // Set WFCK pin input
      {
        PIN_WFCK_INPUT;  
      }

      PIN_DATA_INPUT;

#ifdef LED_RUN
  PIN_LED_OFF;
#endif

#if defined(PSNEE_DEBUG_SERIAL_MONITOR)
 Debug_Inject();
#endif

    }
  }
}
