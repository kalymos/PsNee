
//                           PSNee-V9.0
/*********************************************************************************************************************
 *   CONSOLE MODEL SELECTION (SCPH Hardware Configuration)
 *********************************************************************************************************************/
/*--------------------------------------------------------------------------------------------------------------------
 * Models below do not require BIOS patching. 
 * Standard USB injection is supported.
 *
 *  SCPH model number //  region code | region
 *--------------------------------------------------------------------------------------------------------------------*/
// #define SCPH_xxx1  //  NTSC U/C    | America.
// #define SCPH_xxx2  //  PAL         | Europ.
// #define SCPH_xxx3  //  NTSC J      | Asia.
// #define SCPH_xxxx  //  Universal

// #define SCPH_5903  //  NTSC J      | Asia VCD:


/*------------------------------------------------------------------------------------------------------------------
 * WARNING: These models REQUIRE a BIOS patch.
 *
 * ISP programming is MANDATORY. 
 * The Arduino bootloader introduces a delay that prevents the BIOS patch injection.
 * Using an ISP programmer eliminates this delay.
 * 
 * Note: BIOS version is more critical than the SCPH number for patch success.
 *-------------------------------------------------------------------------------------------------------------------
 *
 *                                    |                Adres pin            |
 *   SCPH model number    // Data pin |    32-pin BIOS   |   40-pin BIOS    | BIOS version
 *------------------------------------------------------------------------------------------------------------------*/
// #define SCPH_102        // DX - D0  | AX - A7          |                  | 4.4e - CRC 0BAD7EA9, 4.5e -CRC 76B880E5
// #define SCPH_100        // DX - D0  | AX - A7          |                  | 4.3j - CRC F2AF798B
// #define SCPH_7500_9000  // DX - D0  | AX - A7          |                  | 4.0j - CRC EC541CD0
// #define SCPH_7000       // DX - D0  | AX - A7          |                  | 4.0j - CRC EC541CD0  Enables hardware support for disabling BIOS patching.
// #define SCPH_3500_5500  // DX - D0  | AX - A16         | AX - A15         | 3.0j - CRC FF3EEB8C, 2.2j - CRC 24FC7E17, 2.1j - CRC BC190209 
// #define SCPH_3000       // DX - D5  | AX - A7, AY - A8 | AX - A6, AY - A7 | 1.1j - CRC 3539DEF6 
// #define SCPH_1000       // DX - D5  | AX - A7, AY - A8 | AX - A6, AY - A7 | 1.0j - CRC 3B601FC8

/*******************************************************************************************************************
 *                           Options
 *******************************************************************************************************************/

#define LED_RUN                       // Turns on the LED when injections occur.
//                                       D13 for Arduino, ATtiny add a led between PB3 (pin 2) and gnd with a 1k resistor in series,
//                                       ATmega32U4 (Pro Micro) add a led between PB6 (pin 10) and gnd with a 1k resistor in series.

// #define DEBUG_SERIAL_MONITOR  // Enables serial monitor output. 

/******************************************************************************************************************
 *  Requires compilation with Arduino libs!
 *  For Arduino connect TXD and GND, for ATtiny PB3 (pin 2) and GND, to your serial card RXD and GND.
 *
 *   For Arduino (Uno/Nano/Pro Mini):
 *   TX (Pin 1)  ----->  RX (Serial Card)
 *   GND         ----->  GND
 *
 *   For ATtiny (25/45/85):
 *   Pin 2 (PB3) ----->  RX (Serial Card)
 *   Pin 4 (GND) ----->  GND
 *
 *******************************************************************************************************************/

/******************************************************************************************************************
 *           Summary of practical information. Fuses. Pinout
 *******************************************************************************************************************
 * Fuses  
 * MCU    | High | Low | Extended
 * --------------------------------------------------
 * ATmega | DF   | EE  | FF 
 * ATtiny | DF   | E2  | FF
 *
 * Pinout
 * Arduino   | PSNee     |
 * ---------------------------------------------------
 * VCC       | VCC       |
 * GND       | GND       |
 * RST       | RESET     | Only for JAP_FAT
 * D2        | BIOS AX   | Only for Bios patch
 * D3        | BIOS AY   | Only for BIOS patch SCPH_1000, SCPH_3000
 * D4        | BIOS DX   | Only for Bios patch
 * D5        | SWITCH    | Optional for disabling Bios patch
 * D6        | SQCK      |
 * D7        | SUBQ      |
 * D8        | DATA      |
 * D9        | WFCK      |
 * D13 ^ D10 | LED       | D10 only for ATmega32U4_16U4
 *
 * ATtiny | PSNee        | ISP   |
 * ---------------------------------------------------
 * Pin1   |              | RESET |
 * Pin2   | LED ^ serial |       | serial only for DEBUG_SERIAL_MONITOR
 * Pin3   | WFCK         |       |
 * Pin4   | GND          | GND   |
 * Pin5   | SQCK         | MOSI  |
 * Pin6   | SUBQ         | MISO  |
 * Pin7   | DATA         | SCK   |
 * Pin8   | VCC          | VCC   |
 *******************************************************************************************************************/

/*******************************************************************************************************************
 *                        pointer and variable section
 *******************************************************************************************************************/

#include "MCU.h"
#include "settings.h"

#define HYSTERESIS_MAX 25 // Now coupled with post-injection reset; allows for higher 
                           // initial accumulation targets without the alignment drift 
                           // (desync) previously affecting SCPH-100x models.

uint8_t wfck_mode = 0;  //Flag initializing for automatic console generation selection 0 = old, 1 = pu-22 end  ++
uint8_t SUBQBuffer[12]; // Global buffer to store the 12-byte SUBQ channel data

uint8_t hysteresis = 0;

#if defined(DEBUG_SERIAL_MONITOR)
  uint16_t global_window = 0; // Stores the remaining cycles from the detection window
#endif

/*******************************************************************************************************************
 *                         Code section
 ********************************************************************************************************************/
/****************************************************************************************
 * FUNCTION    : Bios_Patching()
 *
 * OPERATION   : Real-time Data Bus (DX) override via Address Bus (AX / AY)
 *
 * KEY PHASES:
 *    1. STABILIZATION & ALIGNMENT (AX): 
 *       Synchronizes the execution pointer with the AX rising edge to establish 
 *        a deterministic hardware timing reference.
 *
 *    2. SILENCE DETECTION (BOOT STAGE): 
 *       Validates consecutive silent windows (SILENCE_THRESHOLD) to identify 
 *       the specific boot stage before the target address call.
 *
 *    3. HARDWARE COUNTING & OVERDRIVE (AX): 
 *       Engages INT0 to count AX pulses. On the final pulse, triggers a 
 *       bit-aligned delay to force a custom state on the DX line.
 *
 *    4. SECONDARY SILENCE (GAP DETECTION): 
 *       If PHASE_TWO_PATCH is active, monitors for a secondary silent gap 
 *       (CONFIRM_COUNTER_TARGET_2) between patching windows.
 *
 *    5. SECONDARY OVERDRIVE (AY): 
 *       Engages INT1 (AY) for the final injection stage, synchronizing the 
 *       patch with the secondary memory address cycles.
 *
 *  CRITICAL TIMING & TIMER-LESS ALIGNMENT:
 *    - DETERMINISTIC SILENCE: Uses cycle-accurate polling to filter boot jitter 
 *      and PS1 initialization noise, replacing unstable hardware timers.
 *
 *    - CYCLE STABILIZATION (16MHz LIMIT): Uses '__builtin_avr_delay_cycles' to 
 *      prevent compiler reordering. At 16MHz, the CPU has zero margin; a single 
 *      instruction displacement would break high-speed bus alignment.
 *
 **************************************************************************************/


#ifdef BIOS_PATCH

  /**
  * Shared state variables between ISRs and the main patching loop.
  * Declared 'volatile' to prevent compiler optimization during busy-wait loops.
  */
  volatile uint8_t impulse = 0; // Down-counter for physical address pulses
  volatile uint8_t patch = 0;   // Synchronization flag (0: Idle, 1: AX Done, 2: AY Done)

  /**
  * PHASE 3: Primary Interrupt Service Routine (AX)
  * Triggered on rising edges to perform the real-time bus override.
  */
  ISR(PIN_AX_INTERRUPT_VECTOR) {
      if (--impulse == 0) {
          // Precise bit-alignment delay within the memory cycle
          __builtin_avr_delay_cycles(BIT_OFFSET_CYCLES);

          #ifdef PHASE_TWO_PATCH
              PIN_DX_SET; // Pre-drive high if required by specific logic
          #endif

          // DATA OVERDRIVE: Pull the DX bus to the custom state
          PIN_DX_OUTPUT;
          __builtin_avr_delay_cycles(OVERRIDE_CYCLES);

          #ifdef PHASE_TWO_PATCH
              PIN_DX_CLEAR;
          #endif

          // BUS RELEASE: Return DX to High-Z (Input) mode
          PIN_DX_INPUT;

          PIN_AX_INTERRUPT_DISABLE; // Stop tracking AX pulses
          PIN_LED_OFF;
          patch = 1; // Signal Phase 3 completion
      }
  }

  #ifdef PHASE_TWO_PATCH
  /**
  * PHASE 5: Secondary Interrupt Service Routine (AY)
  * Handles the second injection stage if multi-patching is active.
  */
  ISR(PIN_AY_INTERRUPT_VECTOR) {
      if (--impulse == 0) {
          __builtin_avr_delay_cycles(BIT_OFFSET_2_CYCLES);

          PIN_DX_OUTPUT;
          __builtin_avr_delay_cycles(OVERRIDE_2_CYCLES);
          PIN_DX_INPUT;

          PIN_AY_INTERRUPT_DISABLE;
          PIN_LED_OFF;
          patch = 2; // Signal Phase 5 completion
      }
  }
  #endif

  void Bios_Patching(void) {

      // --- HARDWARE BYPASS OPTION (SCPH-7000 specific) ---
      #if defined(SCPH_7000)
          PIN_SWITCH_INPUT;              // Configure Pin D5 as Input
          PIN_SWITCH_SET;                // Enable internal Pull-up (D5 defaults to HIGH)
          __builtin_avr_delay_cycles(10); // Short delay for voltage stabilization
          
          /** 
          * Exit immediately if the switch pulls the pin to GND (Logic LOW).
          * This allows the user to disable the BIOS patch on-the-fly.
          */
          if (PIN_SWITCH_READ == 0) { 
              return; 
          }
      #endif

      uint8_t current_confirms = 0;
      //uint16_t count;

      patch = 0;     // Reset sync flag
      sei();         // Enable Global Interrupts
      PIN_AX_INPUT;  // Set AX to monitor mode

      // --- PHASE 1: STABILIZATION & ALIGNMENT (AX) ---
      // Establish a deterministic hardware timing reference.
      if (PIN_AX_READ != 0) {
          while (WAIT_AX_FALLING); // Wait if bus is busy
          while (WAIT_AX_RISING);  // Sync with next pulse start
      } else {
          while (WAIT_AX_RISING);  // Sync with upcoming pulse
      }

      // --- PHASE 2: SILENCE DETECTION ---
      // Validate the exact number of silence windows to identify the boot stage.
      while (current_confirms < CONFIRM_COUNTER_TARGET) {
          uint16_t count = SILENCE_THRESHOLD; 
          while (count > 0) {
              if (PIN_AX_READ != 0) {
                  while (WAIT_AX_FALLING);
                  break; // Impulse detected: retry current silence block
              }
              #ifdef IS_32U4_FAMILY
              __asm__ __volatile__ ("nop"); 
              #endif

              count--;
          }
          if (count == 0) {
              current_confirms++; // Validated one silence window
          }
      }

      // --- PHASE 3: LAUNCH HARDWARE COUNTING (AX) ---
      impulse = PULSE_COUNT;
      PIN_LED_ON;
      PIN_AX_INTERRUPT_CLEAR;
      PIN_AX_INTERRUPT_RISING; // Setup rising-edge trigger
      PIN_AX_INTERRUPT_ENABLE; // Engage ISR

      while (patch != 1);


      // --- PHASE 4 & 5: SECONDARY PATCHING SEQUENCE ---
      #ifdef PHASE_TWO_PATCH
          PIN_AY_INPUT;
          current_confirms = 0;
          impulse = PULSE_COUNT_2;
          // Monitor for the specific silent gap before the second patch window
          while (current_confirms < CONFIRM_COUNTER_TARGET_2) {
              uint16_t count = SILENCE_THRESHOLD;
              while (count > 0) {
                  if (PIN_AX_READ != 0) {
                      while (WAIT_AX_FALLING);
                      break;
                  }

                  #ifdef IS_32U4_FAMILY
                  __asm__ __volatile__ ("nop"); 
                  #endif

                  count--;
              }
              if (count == 0) {
                  current_confirms++;
              }
          }

          PIN_LED_ON;
          PIN_AY_INTERRUPT_CLEAR;
          PIN_AY_INTERRUPT_FALLING;
          PIN_AY_INTERRUPT_ENABLE;
      
          while (patch != 2); // Busy-wait for secondary ISR completion
          return;
      #endif
  }
#endif

/*******************************************************************************************************************
 * FUNCTION    : BoardDetection
 * 
 * DESCRIPTION : 
 *    Distinguishes motherboard generations (PU-7 through PU-22+) by analyzing 
 *    the behavior of the WFCK signal.
 *
 * SIGNAL CHARACTERISTICS:
 *    - Legacy Boards (PU-7 to PU-20): WFCK acts as a static GATE signal. 
 *      It remains HIGH.
 *    - Modern Boards (PU-22 or newer): WFCK is an oscillating clock signal 
 *      (Frequency-based).
 * 
 * 
 * WFCK: __-----------------------  // CONTINUOUS (PU-7 .. PU-20)(GATE)
 *
 * WFCK: __-_-_-_-_-_-_-_-_-_-_-_-  // FREQUENCY  (PU-22 or newer)
 *
 * 
 * HISTORICAL CONTEXT:
 *    Traditionally, WFCK was referred to as the "GATE" signal. On early models, 
 *    modchips functioned as a synchronized gate, pulling the signal LOW 
 *    precisely when the region-lock data was being processed.
 * 
 * FREQUENCY DATA:
 *    - Initial/Protection Phase: ~7.3 kHz.
 *    - Standard Data Reading: ~14.6 kHz.
 *
 *******************************************************************************************************************/

void BoardDetection() {
  wfck_mode = 0;           // Default: Legacy (GATE)
  uint8_t pulse_hits = 25; // We need to see 25 oscillations to confirm FREQUENCY mode
  uint16_t detectionWindow = 10000; 
  _delay_ms(300);          // Wait for WFCK to stabilize (High on Legacy, Oscillation on Modern)

  while (--detectionWindow) {
    /**
     * LOGIC BASED ON YOUR ANALYSIS:
     * If WFCK is "CONTINUOUS" (Legacy), it stays HIGH. PIN_WFCK_READ will always be 1.
     * If WFCK is "FREQUENCY" (Modern), it will hit 0 (LOW) periodically.
     */
    if (!PIN_WFCK_READ) {  // Detect a LOW state (only possible in FREQUENCY mode)
      
      pulse_hits--;        // Record one oscillation hit

      if (pulse_hits == 0) {
        wfck_mode = 1;     // Confirmed: FREQUENCY mode (PU-22 or newer)
        #if defined(DEBUG_SERIAL_MONITOR)
          global_window = detectionWindow;
        #endif
        return;            // Exit as soon as we are sure
      }

      /**
       * SYNC: Wait for the signal to go HIGH again.
       * This ensures we count each pulse of the "FREQUENCY" signal only once.
       */
      while (!PIN_WFCK_READ && detectionWindow > 0) {
        detectionWindow--;
      }
    }
  }
  // If the window expires without seeing enough LOW pulses, it remains wfck_mode = 0 (GATE)
}




/******************************************************************************************************************
 * FUNCTION    : CaptureSUBQ
 * 
 * DESCRIPTION : 
 *    Captures a complete 12-byte SUBQ frame from the CD controller.
 *    Synchronizes with the SQCK (SUBQ Clock) pin to shift in serial data from 
 *    the SUBQ pin. This implementation follows the standard PlayStation CDIC 
 *    protocol (Synchronous Serial, LSB first).
 ******************************************************************************************************************/

void CaptureSUBQ(void) {
  uint8_t bytesRemaining = 12; // Total frame size for a complete SUBQ
  uint8_t* bufferPtr = SUBQBuffer;

  do {
    uint8_t bitsToRead = 8;
    
    // Direct byte initialization to save a register copy
    *bufferPtr = 0; 

    while (bitsToRead--) {
      /**
        * PHASE 1: BIT SYNCHRONIZATION (SQCK)
        * Wait for falling edge then sample on the RISING EDGE of SQCK.
        */
      while (PIN_SQCK_READ);   // Wait for falling edge
      while (!PIN_SQCK_READ);  // Wait for rising edge
      
      /**
        * PHASE 2: BIT ACQUISITION & SHIFTING
        * LSB first: shift the buffer pointer directly to avoid 'currentByte' overhead.
        */
      *bufferPtr >>= 1; 
      if (PIN_SUBQ_READ) {
        *bufferPtr |= 0x80; 
      }
    }
    // Advance pointer for the next byte
    bufferPtr++;

  } while (--bytesRemaining); // Efficient countdown for AVR binary size

  #if defined(DEBUG_SERIAL_MONITOR)
    CaptureSUBQLog(SUBQBuffer);
  #endif
}

/******************************************************************************************
 * FUNCTION    : Filter_SUBQ_Samples() [SCPH_5903 Dual-Interface Variant]
 * 
 * DESCRIPTION : 
 *    Parses and filters the raw serial data stream from the SUBQ pin specifically 
 *    for the SCPH-5903 model to differentiate between Game Discs and Video CDs (VCD).
 * 
 *    1. VCD EXCLUSION: Specifically filters out VCD Lead-In patterns (sub-mode 0x02) 
 *       to prevent incorrect region injection on non-game media.
 *    2. SUBQ HIT COUNTING: Increments 'hysteresis' for valid PlayStation TOC (A0-A2) 
 *       markers or active game tracking.
 *    3. SIGNAL DECAY: Decrements the counter when SUBQ samples match VCD patterns 
 *       or unknown data, ensuring stable disc identification.
 * 
 * INPUT       : isDataSector (bool) - Filtered flag based on raw sector control bits.
 ******************************************************************************************/
#ifdef SCPH_5903

  void FilterSUBQSamples(uint8_t controlByte) {
      
      // --- STEP 0: Data/TOC Validation ---
      // Optimized mask (0xD0) to verify bit6 is SET while bit7 and bit4 are CLEARED.
      uint8_t isDataSector = ((controlByte & 0xD0) == 0x40);

      // --- STEP 1: SUBQ Frame Synchronization ---
      // Fast filtering: ignore raw data if sync markers (index 1 and 6) are not 0x00.
      if (SUBQBuffer[1] == 0x00 && SUBQBuffer[6] == 0x00) {

          /** 
          * HIT INCREMENT CONDITIONS:
          * A. VALID PSX LEAD-IN: Data sector AND Point A0-A2 range AND NOT VCD (sub-mode != 0x02).
          *    (uint8_t)(SUBQBuffer[2] - 0xA0) <= 2 is an optimized check for 0xA0, 0xA1, 0xA2.
          * B. TRACKING MAINTENANCE: Keeps count if already synced and reading Mode 0x01 or Data.
          */
          if ( (isDataSector && (uint8_t)(SUBQBuffer[2] - 0xA0) <= 2 && SUBQBuffer[3] != 0x02) ||
              (hysteresis > 0 && (SUBQBuffer[0] == 0x01 || isDataSector)) ) 
          {
              hysteresis++; // Direct increment: faster on 16MHz AVR
              return;
          }
      }

      // --- STEP 2: Signal Decay / Pattern Mismatch ---
      // Decrement the hit counter if no valid PSX pattern is detected in the SUBQ stream.
      if (hysteresis > 0) {
          hysteresis--; // Direct decrement: saves CPU cycles
      }
  }
#else

/******************************************************************************************
 * FUNCTION    : FilterSUBQSamples()
 * 
 * DESCRIPTION : 
 *    Parses and filters the raw serial data stream from the SUBQ pin.
 *    Increments a hit counter (hysteresis) when specific patterns are identified 
 *    in the SUBQ stream, confirming the laser is reading the region-check area.
 * 
 *    1. RAW BUS FILTERING: Validates SUBQ framing by checking sync markers (index 1 & 6).
 *    2. PATTERN MATCHING: Detects Lead-In TOC (A0-A2) or Track 01 at the spiral start.
 *    3. SIGNAL DECAY: Decrements the counter if the current SUBQ sample does not 
 *       match expected PlayStation protection patterns.
 * 
 * INPUT       : isDataSector (bool) - Filtered flag based on raw sector control bits.
 ******************************************************************************************/

  void FilterSUBQSamples(uint8_t controlByte) {
    
    // --- STEP 0: Data/TOC Validation ---
    // Optimized mask (0xD0) to verify bit6 is SET while bit7 and bit4 are CLEARED.
    uint8_t isDataSector = ((controlByte & 0xD0) == 0x40);

    // --- STEP 1: SUBQ Frame Synchronization ---
    // Ignore the raw bitstream unless sync markers (1 & 6) are 0x00.
    if (SUBQBuffer[1] == 0x00 && SUBQBuffer[6] == 0x00) {

        /*
          * HIT INCREMENT CONDITIONS:
          * A. LEAD-IN PATTERNS: Detects TOC markers (A0-A2) or Track 01 at spiral start.
          *    (uint8_t)(SUBQBuffer[3] - 0x03) >= 0xF5 handles the 0x98 to 0x02 wrap-around.
          * B. TRACKING LOCK: Maintains count if already synced and reading valid sectors.
          */
        if ( (isDataSector && (SUBQBuffer[2] >= 0xA0 || (SUBQBuffer[2] == 0x01 && ( (uint8_t)(SUBQBuffer[3] - 0x03) >= 0xF5)))) || 
              (hysteresis > 0 && (SUBQBuffer[0] == 0x01 || isDataSector)) ) 
        {
            hysteresis++; // Direct increment: saves CPU cycles
            return;
        }
    }

    // --- STEP 2: Signal Decay / Missed Hits ---
    // Reduce the hit counter if the current SUBQ sample fails validation.
    if (hysteresis > 0) {
        hysteresis--; // Direct decrement: faster on 16MHz AVR
    }
  }

#endif
/*********************************************************************************************
* FUNCTION    : PerformInjectionSequence
 * 
 * DESCRIPTION :
 *    Executes the SCEx injection sequence to bypass the CD-ROM regional lockout.
 *    Supports two hardware-specific injection methods:
 * 
 *    1. Legacy Gate Mode (PU-7 to PU-20): Modchip acts as a logic gate to pull 
 *       the signal down. WFCK is simulated by the chip if necessary.
 *    2. WFCK Modulation (PU-22+): Modchip modulates the DATA signal in 
 *       sync with the console's real WFCK clock (7.3 kHz or 14.6 kHz).
 * 
 * NOTE: WFCK frequency is approx. 7.3 kHz during initialization/region check, 
 *       but doubles to 14.6 kHz during normal data reading. The modulation loop 
 *       handles both speeds as it syncs directly to the signal edges.
 *********************************************************************************************/

void PerformInjectionSequence(uint8_t injectSCEx) {
  /* 
      Security strings (44-bit SCEx) for the three main regions:
      0: NTSC-J   | SCEI | Sony Computer Entertainment Inc
      1: NTSC-U/C | SCEA | Sony Computer Entertainment America
      2: PAL      | SCEE | Sony Computer Entertainment Europe
      Stored in 6 bytes (48 bits); only the first 44 bits are used during injection.
  */
  static const uint8_t allRegionsSCEx[3][6] = {
      { 0x59, 0xC9, 0x4B, 0x5D, 0xDA, 0x02 }, // SCEI
      { 0x59, 0xC9, 0x4B, 0x5D, 0xFA, 0x02 }, // SCEA
      { 0x59, 0xC9, 0x4B, 0x5D, 0xEA, 0x02 }  // SCEE
  };
  
  const uint16_t BIT_DELAY = 4000; // 4000us is the standard bit timing for SCEx signal (approx. 250 bps)
  const uint8_t isWfck = wfck_mode; // Cache wfck_mode to save CPU cycles during the bit loop
 /**
    * HYSTERESIS COUPLING:
    * Resets hysteresis close to MAX after injection. This allows for a wider 
    * tuning range of HYSTERESIS_MAX without drifting out of synchronization 
    * on SCPH-100x models.
    */
  hysteresis = (HYSTERESIS_MAX - 6);

  #ifdef LED_RUN
      PIN_LED_ON;
  #endif

  PIN_DATA_OUTPUT; 
  PIN_DATA_CLEAR;

  // Legacy boards require the chip to drive the simulated WFCK/Gate
  if (!isWfck) { 
      PIN_WFCK_OUTPUT; 
      PIN_WFCK_CLEAR; 
  }

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
          _delay_us(BIT_DELAY);
      } else {
        // BIT 1: Handle based on board generation
        if (!isWfck) {
          // Legacy Mode: Set DATA to High-Z (floating)
            PIN_DATA_INPUT; 
            _delay_us(BIT_DELAY);
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

    // EXIT CONDITION: Stop after the first successful region unless Universal mode (3)
    if (injectSCEx != 3) {

      PIN_DATA_OUTPUT; 
      PIN_DATA_CLEAR;

      if (!isWfck) {
        PIN_WFCK_INPUT;
        PIN_DATA_INPUT;
      }
      break;
    }


    // Inter-region delay for Universal mode
    PIN_DATA_OUTPUT; 
    PIN_DATA_CLEAR;
    _delay_ms(180);

  }

  // Restore pins to Input/High-Z to avoid signal interference after injection
  if (!isWfck) {
      PIN_WFCK_INPUT;
      PIN_DATA_INPUT;
  }

  #ifdef LED_RUN
      PIN_LED_OFF;
  #endif


  #if defined(DEBUG_SERIAL_MONITOR)
  InjectLog();
  #endif
}

void Init() {

  // --- Hardware Power & Peripheral Optimization ---

  OptimizePeripherals();


  #ifdef LED_RUN
    PIN_LED_OUTPUT;
  #endif

  // --- Critical Boot Patching ---
  #ifdef BIOS_PATCH

// #ifdef LED_RUN
//   PIN_LED_ON;
// #endif

  // Execute BIOS patching 
  Bios_Patching();
// #ifdef LED_RUN
//   PIN_LED_OFF;
// #endif
  #endif

  PIN_SQCK_INPUT;
  PIN_SUBQ_INPUT;

  // --- Debug Interface Setup ---
  #if defined(DEBUG_SERIAL_MONITOR) && defined(IS_ATTINY_FAMILY)
    pinMode(debugtx, OUTPUT); // software serial tx pin
    mySerial.begin(115200); // 13,82 bytes in 12ms, max for softwareserial
  #elif defined(DEBUG_SERIAL_MONITOR) && defined(IS_32U4_FAMILY)
    // On 32U4, Serial1 is the hardware UART on pins D0 (RX) and D1 (TX)
    Serial1.begin(500000); 
  #elif defined(DEBUG_SERIAL_MONITOR) && defined(IS_328_168_FAMILY)
    Serial.begin(500000); // Standard hardware UART for 328/168
  #endif

  // Identify board revision (PU-7 to PU-22+) to set correct injection timings
  BoardDetection();
}

int main() {

  Init();

  #if defined(DEBUG_SERIAL_MONITOR)
    // Display initial board detection results (Window remaining & WFCK mode)
    BoardDetectionLog( global_window, wfck_mode, INJECT_SCEx);
  #endif

  while (1) {

    // Timing Sync: Prevent reading the tail end of the previous SUBQ packet
    _delay_ms(1); 

    // Data Acquisition: Capture 12-byte SUBQ channel stream into buffer
    CaptureSUBQ();

    // Confidence Filtering: Accumulate hits toward HYSTERESIS_MAX
    // Validation and Data/TOC masking (0xD0) are handled inside the filter.
    FilterSUBQSamples(SUBQBuffer[0]);

    // Execution: Trigger SCEx injection once confidence (hysteresis) is reached
    if (hysteresis >= HYSTERESIS_MAX) {
        PerformInjectionSequence(INJECT_SCEx);
    }
  }
  return 0;
}
