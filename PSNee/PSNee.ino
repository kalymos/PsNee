//                           PSNee-8.7.0

/*------------------------------------------------------------------------------------------------
                            MCU selection
------------------------------------------------------------------------------------------------*/

//       MCU               //     Arduino
//------------------------------------------------------------------------------------------------
//#define ATmega328_168    //  Nano, Pro Mini, Uno
#define ATmega32U4_16U4  //  Micro, Pro Micro
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
#define SCPH_5903  //  NTSC J      | Asia VCD:


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
//#define SCPH_100        // DX - D0  | AX - A7          |                  | 4.3j - CRC F2AF798B
//#define SCPH_7500_9000  // DX - D0  | AX - A7          |                  | 4.0j - CRC EC541CD0
//#define SCPH_7000       // DX - D0  | AX - A7          |                  | 4.0j - CRC EC541CD0  Enables hardware support for disabling BIOS patching.
//#define SCPH_5500       // DX - D0  | AX - A5          |                  | 3.0j - CRC FF3EEB8C
//#define SCPH_3500_5000  // DX - D0  | AX - A5          | AX - A4          | 2.2j - CRC 24FC7E17, 2.1j - CRC BC190209
//#define SCPH_3000       // DX - D5  | AX - A7, AY - A8 | AX - A6, AY - A7 | 1.1j - CRC 3539DEF6
//#define SCPH_1000       // DX - D5  | AX - A7, AY - A8 | AX - A6, AY - A7 | 1.0j - CRC 3B601FC8

/*------------------------------------------------------------------------------------------------
                           Options
------------------------------------------------------------------------------------------------*/

#define LED_RUN         // Turns on the LED when injections occur.
//                         D13 for Arduino, ATtiny add a led between PB3 (pin 2) and gnd with a 1k resistor in series, ATmega32U4 (Pro Micro) add a led between PB6 (pin 10) and gnd with a 1k resistor in series.


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


//Flag initializing for automatic console generation selection 0 = old, 1 = pu-22 end  ++
volatile bool wfck_mode = 0;

volatile bool Flag_Switch = 0;


// --- Prototypes (Forward declarations) ---
// These tell the compiler that the functions exist later in the code.
void logic_Standard(uint8_t isDataSector);
void logic_SCPH_5903(uint8_t isDataSector);

// Function pointer type definition for the console detection logic.
// This allows switching between 'Standard' and 'SCPH-5903' heuristics dynamically.
typedef void (*ConsoleLogicPtr)(uint8_t isDataSector);

// Global pointer holding the currently active logic function.
// Using a function pointer eliminates the need for repetitive 'if/else' checks in the main loop.
volatile ConsoleLogicPtr currentLogic = logic_Standard; 

// Variables de contrôle globales 
uint8_t scbuf[12] = { 0 };
uint8_t hysteresis = 0;

  //Initializing values ​​for region code injection timing
#define DELAY_BETWEEN_BITS 4000      // 250 bits/s (microseconds) (ATtiny 8Mhz works from 3950 to 4100) PU-23 PU-22 MAX 4250 MIN 3850
#define DELAY_BETWEEN_INJECTIONS 90  // The sweet spot is around 80~100. For all observed models, the worst minimum time seen is 72, and it works well up to 250.


/*------------------------------------------------------------------------------------------------
                         Code section
------------------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------
  Function: board_detection

  This function distinguishes motherboard generations by detecting 
  the nature of the WFCK signal:

  WFCK: __-----------------------  // CONTINUOUS (PU-7 .. PU-20)(GATE)

  WFCK: __-_-_-_-_-_-_-_-_-_-_-_-  // FREQUENCY  (PU-22 or newer)

  Traditionally, the WFCK signal was called GATE. This is because, on early models, 
  modchips acted like a gate that would open to pull the signal down
   at the exact moment the region code was being passed (which is still the case today).

  During the initialization and region protection zone reading phases, 
  the WFCK clock frequency is approximately 7.3 kHz.
  During normal data reading, the frequency shifts to 14.6 kHz.

-----------------------------------------------------------------------*/

void board_detection(){

  uint16_t pulses = 0;                      // Counter for detected falling edges (transitions to 0)      
  uint8_t  last_state = 0;                  // Stores the previous state to detect logic level changes
  uint32_t totalSamples = 600000;           // Timeout/Sampling window to limit detection duration

 // Runs until 600,000 cycles pass OR 600 pulses transitions are found
  while (totalSamples > 0 && pulses < 600){

    // Check if the current pin state differs from the last recorded state
    if (PIN_WFCK_READ != last_state){
      last_state = PIN_WFCK_READ;              // Update state history
 
      // If the new state is LOW (0), a falling edge has occurred
      if (last_state == 0){
        pulses++;
      }
    }
    totalSamples--;                        // Decrement the loop counter (timeout mechanism)
  }

  // High count (> 500)  oscillating signal (Newer boards)
  if (pulses > 500) {
    wfck_mode = 1;                         // Target: PU-22 or newer
  }

  // Low count implies a static signal (Older boards)
  else {
    wfck_mode = 0;                         // Target: PU-7 to PU-20
  }
}



//******************************************************************************************************************
// Reads a complete 12-byte SUBQ transmission from the CD drive.
// Uses clock-edge synchronization and includes a safety timeout for malformatted streams.
//******************************************************************************************************************
void captureSubQ(void) {
  //uint8_t bitpos = 0;
  uint8_t scpos = 0;
  uint8_t bitbuf = 0;

  do {
      bitbuf = 0;
      for (uint8_t i = 0; i < 8; i++) {
          // Wait for Clock to go LOW then HIGH (Sampling on Rising Edge)
          while (PIN_SQCK_READ != 0); 
          while (PIN_SQCK_READ == 0); 
          
          // Shift buffer and sample the SUBQ pin
          bitbuf >>= 1; 
          if (PIN_SUBQ_READ) {
              bitbuf |= 0x80; 
          }
      }
      scbuf[scpos++] = bitbuf;
  } while (scpos < 12);

}



/**************************************************************************************
 * Processes sector data for the SCPH-5903 (Dual-interface PS1) to differentiate
 * between PlayStation games and Video CDs (VCD).
 * 
 * This heuristic uses an 'hysteresis' counter to stabilize disc detection:
 * - Increases when a PSX Lead-In or valid game sector is identified.
 * - Remains neutral/ignores VCD-specific Lead-In patterns.
 * - Decreases (fades out) when the data does not match known patterns.
 *
 *  isDataSector Boolean flag indicating if the current sector contains data.
 
**************************************************************************************/
void logic_SCPH_5903(uint8_t isDataSector) {
    // Identify VCD Lead-In: Specific SCBUF patterns (0xA0/A1/A2) with sub-mode 0x02
    bool isVcdLeadIn = isDataSector && scbuf[1] == 0x00 && scbuf[6] == 0x00 &&
                       (scbuf[2] == 0xA0 || scbuf[2] == 0xA1 || scbuf[2] == 0xA2) &&
                       (scbuf[3] == 0x02);

    // Identify PSX Lead-In: Same SCBUF patterns but different sub-mode (!= 0x02)
    bool isPsxLeadIn = isDataSector && scbuf[1] == 0x00 && scbuf[6] == 0x00 &&
                       (scbuf[2] == 0xA0 || scbuf[2] == 0xA1 || scbuf[2] == 0xA2) &&
                       (scbuf[3] != 0x02);

    if (isPsxLeadIn) {
        hysteresis++;
    }
    else if (hysteresis > 0 && !isVcdLeadIn && 
             ((scbuf[0] == 0x01 || isDataSector) && scbuf[1] == 0x00 && scbuf[6] == 0x00)) {
        hysteresis++;   // Maintain/Increase confidence for valid non-VCD sectors
    }
    else if (hysteresis > 0) {
        hysteresis--;    // Patterns stop matching
    }
}
/******************************************************************************************
 * Heuristic logic for standard PlayStation hardware (Non-VCD models).
 * 
 * This function monitors disc sectors to identify genuine PlayStation discs:
 * 1. Checks for specific Lead-In markers (Point A0, A1, A2 or Track 01).
 * 2. Uses an incrementing 'hysteresis' counter to confirm disc validity.
 * 3. Includes a 'fade-out' mechanism to reduce the counter if valid patterns are lost,
 *    effectively filtering out noise or read errors.
 *
 *  isDataSector Boolean flag: true if the current sector is a data sector.

******************************************************************************************/

void logic_Standard(uint8_t isDataSector) {
    // Detect specific Lead-In patterns 
    if ((isDataSector && scbuf[1] == 0x00 && scbuf[6] == 0x00) &&
        (scbuf[2] == 0xA0 || scbuf[2] == 0xA1 || scbuf[2] == 0xA2 ||
        (scbuf[2] == 0x01 && (scbuf[3] >= 0x98 || scbuf[3] <= 0x02)))) {
        hysteresis++;
    }
    // Maintain confidence if general valid sector markers are found
    else if (hysteresis > 0 && 
             ((scbuf[0] == 0x01 || isDataSector) && scbuf[1] == 0x00 && scbuf[6] == 0x00)) {
        hysteresis++;
    }
    else if (hysteresis > 0) {
        hysteresis--;
    }
}


/**
 * Executes the SCEX injection sequence once the confidence threshold (hysteresis) is met.
 * Supports both legacy boards (Gate logic) and newer boards (WFCK synchronization).
 */
void performInjectionSequence() {

  // SCEX data patterns for different regions (SCEE: Europe, SCEA: America, SCEI: Japan)
  // Each array contains the specific bit sequence required to bypass region locking.
  #if (REGION_SETTING == 2)
  static const uint8_t SCEData[] = { //  NTSC U/C    | America.
    0b01011001,
    0b11001001,
    0b01001011,
    0b01011101,
    0b11101010,
    0b00000010
  };
  #elif (REGION_SETTING == 1)
  static const uint8_t SCEData[] = { //  PAL         | Europ.
    0b01011001,
    0b11001001,
    0b01001011,
    0b01011101,
    0b11111010,
    0b00000010
  };
  #else
  static const uint8_t SCEData[] = { //  NTSC J      | Asia.
    0b01011001,
    0b11001001,
    0b01001011,
    0b01011101,
    0b11011010,
    0b00000010
  };
  #endif
  // Use 'SCEData' directly in your loops
  const uint8_t* ByteSet = SCEData;
// Select the appropriate data pointer based on the region character.
  // Using a single char variable avoids pointer-to-integer comparison errors.
//  const uint8_t* ByteSet = (region == '1') ? SCEEData : (region == '2') ? SCEAData : SCEIData;
// Select the bitstream based on the first character of the region string.
// We access region[0] to avoid pointer-to-integer comparison warnings.
//const uint8_t* ByteSet = (region[0] == 'e') ? SCEEData : (region[0] == 'a') ? SCEAData : SCEIData;


  if (hysteresis >= HYSTERESIS_MAX) {
    hysteresis = 11; // Reset to 11 for faster re-injection if head stays in TOC

#ifdef LED_RUN
    PIN_LED_ON;
#endif

    // Initialize pins for injection
    PIN_DATA_OUTPUT; 
    PIN_DATA_CLEAR;
    if (!wfck_mode) { PIN_WFCK_OUTPUT; PIN_WFCK_CLEAR; }

    _delay_ms(DELAY_BETWEEN_INJECTIONS);

    // Perform 2 injection cycles for reliability
    for (uint8_t scei = 0; scei < 2; scei++) {
       
      // Iterate through the 44 bits of the SCEX sequence
      for (uint8_t bit_counter = 0; bit_counter < 44; bit_counter++) { 

        // Extract bit using bitwise shift
        bool currentBit = (ByteSet[bit_counter / 8] & (1 << (bit_counter % 8)));

        if (!wfck_mode) {
          // MODE: OLDER BOARDS (PU-7 to PU-20) - Standard Gate Logic
          if (currentBit == 0) {
            // For OLD boards, bit 0 is a forced LOW signal
            PIN_DATA_OUTPUT;         
            PIN_DATA_CLEAR;   
          } 
          else {
            // For OLD boards, bit 1 is High-Z (Pin set as input)
            PIN_DATA_INPUT;
          }
          _delay_us(DELAY_BETWEEN_BITS);
        }
        else if (wfck_mode) {
          // MODE: NEWER BOARDS (PU-22+) - WFCK Clock Synchronization
          PIN_DATA_OUTPUT; 
          
          if (currentBit == 0) {
            // Bit 0: Constant LOW signal
            PIN_DATA_CLEAR;   
            _delay_us(DELAY_BETWEEN_BITS);
          } 
          else {
            // Bit 1: High-speed modulation synchronized with WFCK clock
            uint8_t count = 30;
            uint8_t last_wfck = PIN_WFCK_READ;

            // Optimized loop: Data line only toggles on WFCK edge detection
            while (count > 0) {
              uint8_t current_wfck = PIN_WFCK_READ;
              
              if (current_wfck != last_wfck) { 
                // Only update DATA if WFCK state changed
                if (current_wfck) {
                  PIN_DATA_SET;
                } else {
                  PIN_DATA_CLEAR;
                }

                // Count rising edges to track bit duration
                if (current_wfck) {
                  count--;
                }
                last_wfck = current_wfck;
              }
            }
          }
        }
      }
      // After injecting SCEX data, set DATA pin as output and clear (low)
      PIN_DATA_OUTPUT;
      PIN_DATA_CLEAR;
      _delay_ms(DELAY_BETWEEN_INJECTIONS);
    }
    // Cleanup: Release the bus
    if (!wfck_mode) {
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


void Init() {
#if defined(ATmega328_168) 

  // Optimization test in progress...
  A;
  B;
  D;
  E;
  //F;
  G;
#endif

#ifdef SCPH_5903
  currentLogic = logic_SCPH_5903;
#else
  currentLogic = logic_Standard;
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

  GLOBAL_INTERRUPT_DISABLE;
  
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

  Init();

#ifdef BIOS_PATCH

#ifdef LED_RUN
  PIN_LED_ON;
#endif

  if (Flag_Switch == 0) {
    Bios_Patching();
  }

#ifdef LED_RUN
  PIN_LED_OFF;
#endif

#endif

  board_detection();

#if defined(PSNEE_DEBUG_SERIAL_MONITOR)
 Debug_Log(lows, wfck_mode);
#endif

  while (1) {

    _delay_ms(1); /* Start with a small delay, which can be necessary 
                    in cases where the MCU loops too quickly and picks up the laster SUBQ trailing end*/
    captureSubQ();

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
      
  // 2. Execute selected logic through function pointer
  currentLogic(isDataSector);

#ifdef LED_RUN
  PIN_LED_ON;
#endif

  // 3. Check hysteresis and inject if necessary
  performInjectionSequence();

#ifdef LED_RUN
  PIN_LED_OFF;
#endif

#if defined(PSNEE_DEBUG_SERIAL_MONITOR)
 Debug_Inject();
#endif

  }
}
