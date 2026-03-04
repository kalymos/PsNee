//                           PSNee-8.7

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
//#define SCPH_xxxx  //              | Universal.
//#define SCPH_xxx1  //  NTSC U/C    | America.
//#define SCPH_xxx2  //  PAL         | Europ.
//#define SCPH_xxx3  //  NTSC J      | Asia.
//#define SCPH_5903  //  NTSC J      | Asia VCD.

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
//#define SCPH_5000       // DX - D0  | AX - A5          | AX - A4          | 2.2j - CRC 24FC7E17
//#define SCPH_3500       // DX - D0  | AX - A5          | AX - A4          | 2.1j - CRC BC190209 
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
  MCU             | High | Low | Extended
  --------------------------------------------------
  ATmega 328/168  | DF   | EE  | FD 
  ATmega32U4_16U4 | DF   | EE  | FB
  ATtiny          | DF   | E2  | FF
  ATmega 328PB    | DF   | EE  | F5

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
#include <util/delay.h>


//Flag initializing for automatic console generation selection 0 = old, 1 = pu-22 end  ++
volatile uint8_t wfck_mode = 0;


// --- Prototypes (Forward declarations) ---
// These tell the compiler that the functions exist later in the code.


// Function pointer type definition for the console detection logic.
// This allows switching between 'Standard' and 'SCPH-5903' heuristics dynamically.
// Définition du type de pointeur de fonction pour la logique de console
typedef void (*ConsoleLogicPtr)(uint8_t isDataSector);

// Déclarations forward des deux fonctions
void processStandardLogic(uint8_t isDataSector);
void processScph5903Logic(uint8_t isDataSector);

// Pointeur global vers la fonction active
#ifdef SCPH_5903
  volatile ConsoleLogicPtr currentLogic = processScph5903Logic;
#else
  volatile ConsoleLogicPtr currentLogic = processStandardLogic;
#endif


// Variables de contrôle globales 
// Global buffer to store the 12-byte Sub-Q channel data
uint8_t subqBuffer[12]; 
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


void board_detection() {
  // Default to static signal (PU-7 to PU-20)
  wfck_mode = 0;

  /* 
    INITIAL STABILIZATION DELAY (300ms)
    PU-7 to PU-20: Voltage climbs slowly (up to 54ms) then stays HIGH (static).
    PU-22+: Signal only starts oscillating (~7.3kHz) after approximately 297ms.
    Waiting 300ms bypasses all power-up transients and initial noise.
  */
  _delay_ms(300); 

  // Sampling window to detect the oscillating signal
  uint16_t detectionWindow = 10000; 
  
  while (--detectionWindow) {
    /* 
       On older boards (PU-7/20), the signal is now a solid HIGH.
       If we detect a LOW state, it's a potential oscillation from a newer board.
    */
    if (!PIN_WFCK_READ) {
      // Small debounce delay to filter out micro-glitches or remaining noise
      uint8_t debounce = 100;
      while (--debounce);

      /* 
         VERIFICATION: If the signal is STILL low, it confirms a real 
         clock cycle (WFCK). Older boards will never reach this state 
         once stabilized at HIGH.
      */
      if (!PIN_WFCK_READ) {
        wfck_mode = 1; // Target: PU-22 or newer
        return;
      }
    }
  }

#if defined(PSNEE_DEBUG_SERIAL_MONITOR)
  Debug_Log(wfck_mode);
#endif
}






//******************************************************************************************************************
// Reads a complete 12-byte SUBQ transmission from the CD drive.
// Uses clock-edge synchronization and includes a safety timeout for malformatted streams.
//******************************************************************************************************************

void captureSubQ(void) {
  // Total bytes to read from the CD-ROM subcode channel
  uint8_t bytesRemaining = 12; 
  uint8_t* bufferPtr = subqBuffer;

  do {
    uint8_t currentByte = 0;
    uint8_t bitsToRead = 8;
    
    while (bitsToRead--) {
      // PHASE 1: Wait for Clock (SQCK) to go LOW then HIGH (Sampling on Rising Edge)
      // This ensures data is stable on the SUBQ pin before reading.
      while (PIN_SQCK_READ);   // Wait for falling edge
      while (!PIN_SQCK_READ);  // Wait for rising edge
      
      // PHASE 2: Shift bit into the byte (LSB first)
      currentByte >>= 1; 
      if (PIN_SUBQ_READ) {
        currentByte |= 0x80; 
      }
    }
    // Store reconstructed byte and advance pointer
    *bufferPtr++ = currentByte;
  } while (--bytesRemaining); // Faster than (bytesRemaining < 12)

#if defined(PSNEE_DEBUG_SERIAL_MONITOR)
  logSubQ(subqBuffer);
#endif
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

void processScph5903Logic(uint8_t isDataSector) {
    uint8_t currentHysteresis = hysteresis;

    // Fast filtering: most sectors fail here by checking sync markers (index 1 and 6)
    if (subqBuffer[1] == 0x00 && subqBuffer[6] == 0x00) {
        uint8_t pointAddress = subqBuffer[2];

        /* 
           INCREMENT CONDITIONS:
           1. Valid PSX Lead-In: data sector AND Point A0-A2 range AND NOT VCD (sub-mode 0x02).
           2. Tracking Maintenance: Valid sector (Mode 0x01 or Data) while already synchronized.
        */
        if ( (isDataSector && (uint8_t)(pointAddress - 0xA0) <= 2 && subqBuffer[3] != 0x02) ||
             (currentHysteresis > 0 && (subqBuffer[0] == 0x01 || isDataSector)) ) 
        {
            hysteresis = currentHysteresis + 1;
            return;
        }
    }

    /* 
       DECREMENT CONDITION:
       No match found or VCD Lead-In detected. Slowly decrease confidence level.
    */
    if (currentHysteresis > 0) {
        hysteresis = currentHysteresis - 1;
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

void processStandardLogic(uint8_t isDataSector) {
    uint8_t currentHysteresis = hysteresis;

    // Fast filtering: most sectors fail here by checking sync markers (index 1 and 6)
    if (subqBuffer[1] == 0x00 && subqBuffer[6] == 0x00) {
        uint8_t pointAddress = subqBuffer[2];
        
        /* 
           INCREMENT CONDITIONS:
           1. Lead-In Detection:
              - Point A0-A2 or higher (TOC info).
              - OR Point 01 with a timestamp near the spiral start: 
                Checking if index 3 is >= 98 or <= 02 using unsigned wrap-around:
                (uint8_t)(subqBuffer[3] - 0x03) >= 0xF5 (245) covers 0x98 to 0x02.
           2. Tracking Maintenance:
              - Valid sector (Mode 0x01 or Data) while already synchronized.
        */
        if ( (isDataSector && (pointAddress >= 0xA0 || (pointAddress == 0x01 && ( (uint8_t)(subqBuffer[3] - 0x03) >= 0xF5)))) || 
             (currentHysteresis > 0 && (subqBuffer[0] == 0x01 || isDataSector)) ) 
        {
            hysteresis = currentHysteresis + 1;
            return;
        }
    }

    /* 
       DECREMENT CONDITION:
       No valid match found. Slowly decrease confidence level.
    */
    if (currentHysteresis > 0) {
        hysteresis = currentHysteresis - 1;
    }
}

/*********************************************************************************************
 * Executes the SCEx injection sequence to bypass the CD-ROM regional lockout.
 * 
 * This function supports two hardware-specific injection methods:
 * 1. Legacy Gate Mode (PU-7 to PU-20): Modchip acts as a logic gate to pull 
 *    the signal down. WFCK is simulated by the chip if necessary.
 * 2. WFCK Modulation (PU-22+): Modchip modulates the DATA signal in 
 *    sync with the console's real WFCK clock.
 * 
 * NOTE: WFCK frequency is approx. 7.3 kHz during initialization/region check, 
 * but doubles to 14.6 kHz during normal data reading. The modulation loop 
 * handles both speeds as it syncs directly to the signal edges.
 *********************************************************************************************/

void performInjectionSequence(uint8_t injectSCEx) {
    /* 
       Security strings (44-bit SCEx) for the three main regions:
       0: NTSC-J (SCEI - Sony Computer Entertainment Inc.)
       1: NTSC-U/C (SCEA - Sony Computer Entertainment America)
       2: PAL (SCEE - Sony Computer Entertainment Europe)
       Stored in 6 bytes (48 bits); only the first 44 bits are used during injection.
    */
    static const uint8_t allRegionsSCEx[3][6] = {
        { 0x59, 0xC9, 0x4B, 0x5D, 0xDA, 0x02 }, // SCEI
        { 0x59, 0xC9, 0x4B, 0x5D, 0xFA, 0x02 }, // SCEA
        { 0x59, 0xC9, 0x4B, 0x5D, 0xEA, 0x02 }  // SCEE
    };


    hysteresis = 11;  // Reset hysteresis to mid-point after triggering
    #ifdef LED_RUN
        PIN_LED_ON;
    #endif

    // Cache wfck_mode to save CPU cycles during the bit loop
    uint8_t isWfck = wfck_mode; 
    PIN_DATA_OUTPUT; 
    PIN_DATA_CLEAR;

    // Legacy boards require the chip to drive the simulated WFCK/Gate
    if (!isWfck) { 
        PIN_WFCK_OUTPUT; 
        PIN_WFCK_CLEAR; 
    }
    
    _delay_ms(DELAY_BETWEEN_INJECTIONS);

    // Loop through the selected region(s)
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t regionIndex = (injectSCEx == 3) ? i : injectSCEx;
        const uint8_t* bytePtr = allRegionsSCEx[regionIndex];
        
        uint8_t currentByte = *bytePtr++;
        uint8_t bitIdx = 0;

        // Process the 44-bit SCEx stream
        for (uint8_t totalBits = 44; totalBits > 0; totalBits--) {
            uint8_t currentBit = currentByte & 0x01;
            currentByte >>= 1;
            bitIdx++;

            // Reload next byte every 8 bits (except the last partial one)
            if (bitIdx == 8 && totalBits > 4) {
                currentByte = *bytePtr++;
                bitIdx = 0;
            }

            if (currentBit == 0) {
                // BIT 0: Pull DATA low-
                PIN_DATA_CLEAR;
                if (!isWfck) PIN_DATA_OUTPUT; 
                _delay_us(DELAY_BETWEEN_BITS);
            } else {
                // BIT 1: Handle based on board generation
                if (!isWfck) {
                  // Legacy Mode: Set DATA to High-Z (floating)
                    PIN_DATA_INPUT; 
                    _delay_us(DELAY_BETWEEN_BITS);
                } else {
                    /* 
                       WFCK Modulation Loop: Syncs to 7.3kHz or 14.6kHz.
                       Follows hardware edges to stay bit-perfect with the console.
                    */
                    uint8_t count = 30; 
                    while (count--) {
                        while (PIN_WFCK_READ); // Wait for Falling Edge
                        PIN_DATA_CLEAR;
                        while (!PIN_WFCK_READ); // Wait for Rising Edge
                        PIN_DATA_SET;
                    }
                }
            }
        }

        // Clean up state between region cycles
        PIN_DATA_OUTPUT; 
        PIN_DATA_CLEAR;
        _delay_ms(DELAY_BETWEEN_INJECTIONS);

        /* 
           EXIT CONDITION:
           If we are NOT in Universal mode (3), we stop after the first 
           successful region injection.
        */
        if (injectSCEx != 3) break; 
    }

    // Restore pins to Input/High-Z to avoid signal interference after injection
    if (!isWfck) {
        PIN_WFCK_INPUT;
        PIN_DATA_INPUT;
    }
    #ifdef LED_RUN
        PIN_LED_OFF;
    #endif


#if defined(PSNEE_DEBUG_SERIAL_MONITOR)
 Debug_Inject();
#endif

}






void Init() {

#if defined(ATmega328_168) 
  optimizePeripherals();
#endif

#ifdef LED_RUN
  PIN_LED_OUTPUT;
#endif

#ifdef BIOS_PATCH_3
  uint8_t skipPatch = 0;
  GLOBAL_INTERRUPT_ENABLE;

#ifdef SCPH_7000
  // Check hardware switch for SCPH-7000 models
  PIN_SWITCH_INPUT;
  PIN_SWITCH_SET;
  if (PIN_SWITCH_READ == 0){
   skipPatch =1;           // Disable patching if switch is triggered
  }
#endif

// #ifdef LED_RUN
//   PIN_LED_ON;
// #endif

  // Execute BIOS patching unless bypassed by switch
  if (skipPatch == 0) {
    Bios_Patching();
  }

// #ifdef LED_RUN
//   PIN_LED_OFF;
// #endif
#endif


  // Disable interrupts and set CD-ROM interface pins to Input
  GLOBAL_INTERRUPT_DISABLE;
  PIN_SQCK_INPUT;
  PIN_SUBQ_INPUT;

#if defined(PSNEE_DEBUG_SERIAL_MONITOR) && defined(ATtiny85_45_25)
  //pinMode(debugtx, OUTPUT); // software serial tx pin
  mySerial.begin(115200); // 13,82 bytes in 12ms, max for softwareserial. (expected data: ~13 bytes / 12ms) // update: this is actually quicker
#elif defined(PSNEE_DEBUG_SERIAL_MONITOR) && !defined(ATtiny85_45_25)
  Serial.begin(500000); // 60 bytes in 12ms (expected data: ~26 bytes / 12ms) // update: this is actually quicker
#endif

  // Detect board generation (PU-7 to PU-22+) before starting the main loop
  board_detection();
}

int main() {

  Init();

// #ifdef SCPH_5903
//   currentLogic = logic_SCPH_5903;
// #else
//   currentLogic = logic_Standard;
// #endif


  while (1) {

    // Small delay to prevent re-reading the tail end of the same SUBQ packet
    _delay_ms(1); 

    //Capture the 12-byte Sub-Q channel data into subqBuffer
    captureSubQ();

    // Optimized Sector Filtering:
    // Masking bits 7, 6, and 4 simultaneously using 0xD0 (binary 11010000).
    // This verifies that the "Data/TOC" bit (0x40) is SET, while bits 7 and 4 are CLEARED.
    // Equivalent to: (bit7 == 0 && bit6 == 1 && bit4 == 0).
    //uint8_t isDataSector = ((scbuf[0] & 0xD0) == 0x40);

    uint8_t isDataSector = ((subqBuffer[0] & 0xD0) == 0x40);

      
    // Execute selected logic through function pointer
    currentLogic(isDataSector);

    //Trigger SCEx injection once the confidence threshold is reached
    if (hysteresis >= HYSTERESIS_MAX) {
        performInjectionSequence(INJECT_SCEx);
    }
  }
}
