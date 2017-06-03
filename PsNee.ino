// This PsNee version is meant for Arduino boards.
// 16Mhz and 8Mhz variants are supported. "Pro Micro" etc supported and recommended
// ATtinys should be able to do this as well; requires a bit of porting and testing

// PAL PU-41 support isn't implemented here yet. Use PsNee v6 for them.

// Uncomment the correct inject_SCEI(), inject_SCEA(), inject_SCEE() in loop(), depending on your console region.
// Uncomment #define PU22_MODE for PU-22, PU-23, PU-41 mainboards.

//#define PU22_MODE

#include <Flash.h> // requires Arduino Flash library installed

//Pins
int data = 8;         // Arduino pin 8, ATmega PB0 injects SCEX string. point 6 in old modchip Diagrams
int spidata = 10;     // Arduino pin 10, ATmega PB2 "SUBQ" Mechacon pin 24 (PU-7 and early PU-8 Mechacons: pin 39)
int spiclock = 11;    // Arduino pin 11, ATmega PB3 "SQCK" Mechacon pin 26 (PU-7 and early PU-8 Mechacons: pin 41)
int wfck = 12;        // Arduino pin 12, ATmega PB4 point 5 in old modchip Diagrams
//Timing
int delay_between_bits = 4000; // 250 bits/s (microseconds)
int delay_between_injections = 74; // 74 original, 72 in oldcrow (milliseconds)

// clock pulse timeout for sampling of the SUBQ packets: All PSX will transmit 12 packets of 8 bit / 1 byte each, once CD reading is stable.
// If the pulses take too much time, we drop the entire 12 packet stream and wait for a better chance. 10000 is a good value.
#define TIMEOUT_CLOCK 10000

// ToDo: merge into 1 function
void inject_SCEE()
{
  //SCEE-array                                                                                                                   //      Start            Data     Stop
  FLASH_ARRAY (boolean, SCEEData, 1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0);      //SCEE: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01011101 00   44 bits total
 
  for (byte bit_counter = 0; bit_counter < 44; bit_counter = bit_counter + 1)
  {
    if (SCEEData[bit_counter] == 0)
    {
      bitClear(PORTB,0); // pull data low
      delayMicroseconds(delay_between_bits);
    }
    else
    {
      unsigned long now = micros();
      do {
#ifdef PU22_MODE
        bool wfck_sample = bitRead(PINB, 4);       
        bitWrite(PORTB,0,wfck_sample); // output wfck signal on data pin
#else
        bitSet(PORTB,0); // drag data pin high
#endif
      }
      while ((micros() - now) < delay_between_bits); // range: 3900us - 4200us
    }
  }

  bitClear(PORTB,0); // pull data low
  delay(delay_between_injections);
}

void inject_SCEA()
{
  //SCEE-array                                                                                                                   //      Start            Data     Stop
  FLASH_ARRAY (boolean, SCEAData, 1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,1,1,1,1,0,1,0,0);      //SCEA: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01111101 00
 
  for (byte bit_counter = 0; bit_counter < 44; bit_counter = bit_counter + 1)
  {
    if (SCEAData[bit_counter] == 0)
    {
      bitClear(PORTB,0); // pull data low
      delayMicroseconds(delay_between_bits);
    }
    else
    {
      unsigned long now = micros();
      do {
#ifdef PU22_MODE
        bool wfck_sample = bitRead(PINB, 4);       
        bitWrite(PORTB,0,wfck_sample); // output wfck signal on data pin
#else
        bitSet(PORTB,0); // drag data pin high
#endif
      }
      while ((micros() - now) < delay_between_bits); // range: 3900us - 4200us
    }
  }

  bitClear(PORTB,0); // pull data low
  delay(delay_between_injections);
}

void inject_SCEI()
{
  //SCEI-array                                                                                                                   //      Start            Data     Stop
  FLASH_ARRAY (boolean, SCEIData, 1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,1,0,1,1,0,1,0,0);      //SCEI: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01101101 00
 
  for (byte bit_counter = 0; bit_counter < 44; bit_counter = bit_counter + 1)
  {
    if (SCEIData[bit_counter] == 0)
    {
      bitClear(PORTB,0); // pull data low
      delayMicroseconds(delay_between_bits);
    }
    else
    {
      unsigned long now = micros();
      do {
#ifdef PU22_MODE
        bool wfck_sample = bitRead(PINB, 4);       
        bitWrite(PORTB,0,wfck_sample); // output wfck signal on data pin
#else
        bitSet(PORTB,0); // drag data pin high
#endif
      }
      while ((micros() - now) < delay_between_bits); // range: 3900us - 4200us
    }
  }

  bitClear(PORTB,0); // pull data low
  delay(delay_between_injections);
}

//--------------------------------------------------
//     Setup
//--------------------------------------------------
void setup()
{
  pinMode(data, INPUT); // Arduino pin 8, ATmega PB0
  pinMode(spidata, INPUT); // spi data in Arduino pin 10, ATmega PB2
  pinMode(spiclock, INPUT); // spi clock Arduino pin 11, ATmega PB3
 
  // PU-22+ mode: Input the sync signal here (point 5 in old modchip diagrams).
  // The signal will be used in SCEX injections, blocking license strings from original discs.
  // Leave this input unconnected for PU-7, PU-8, PU-18, PU-20 mainboards.
  pinMode(wfck, INPUT); // Arduino pin 12, ATmega PB4
 
  Serial.begin (115200);
  Serial.println("Start ");
 
  // Power saving
  // Disable the ADC by setting the ADEN bit (bit 7)  of the
  // ADCSRA register to zero.
  ADCSRA = ADCSRA & B01111111;
  // Disable the analog comparator by setting the ACD bit
  // (bit 7) of the ACSR register to one.
  ACSR = B10000000;
  // Disable digital input buffers on all analog input pins
  // by setting bits 0-5 of the DIDR0 register to one.
  DIDR0 = DIDR0 | B00111111;
}

void loop()
{
  static unsigned int num_resets = 0; // debug / testing
  static byte scbuf [12] = { 0 }; // We will be capturing PSX "SUBQ" packets, there are 12 bytes per valid read.
  static byte scpos = 0;          // scbuf position

  unsigned int timeout_clock_low_counter = 0;
  byte bitbuf = 0;         // SUBQ bit storage

  // Try to capture 8 bits per loop run.
  // unstable clock, bootup, reset and disc changes are ignored
  // The console will output consistent SUBQ data eventually.
  for (byte bitpos = 0; bitpos<8; bitpos++) {
    do {
      // waste/count cycles, reset on timeout
      timeout_clock_low_counter++;
      if (timeout_clock_low_counter > TIMEOUT_CLOCK){
        scpos = 0;
        num_resets++;
        return;
      }
    }
    while (bitRead(PINB, 3)); // wait for clock to go low

    // waste a few cpu cycles > better readings in tests
    __asm__("nop\n\t");
    __asm__("nop\n\t");

    // sample the bit.
    bool sample = bitRead(PINB, 2);
    bitbuf |= sample << bitpos;

    do {
      // waste cycles
    } while (!(bitRead(PINB, 3))); // Note: Even if sampling is bad, it will not get stuck here. There will be clock pulses eventually.

    timeout_clock_low_counter = 0; // This bit came through fine.
  }

  scbuf[scpos] = bitbuf;
  scpos++;

  if (scpos == 12){
    // end of time critical section. We now have all 12 subchannel packets. It will be 13.3ms until the next ones.
    // print out some debug stats if a serial terminal is connected
    for (int i = 0; i<12;i++) {
      Serial.print(scbuf[i], HEX);
      Serial.print(" ");
    }
    Serial.print(" resets:  ");
    Serial.println(num_resets);
    num_resets = 0;
    scpos = 0;
  }
  else return;
 
  // check if this is the wobble area
  // 3 bytes would be enough to recognize it. The extra checks just ensure this isn't a garbage reading.
  if ( scbuf[0] == 0x41 &&  scbuf[1] == 0x00 &&  scbuf[6] == 0x00 && // 0x41 = psx game, beginning of the disc, sanity check
    (scbuf[2] == 0xA0 || scbuf[2] == 0xA1 || scbuf[2] == 0xA2) ){ // lead in / wobble area is marked by 0xA0, 0xA1, 0xA2
   
    Serial.println("INJECT!");

    pinMode(data, OUTPUT); // prepare for SCEX injection

    bitClear(PORTB,0); // pull data low
    delay(74); // HC-05 is waiting for a bit of silence (pin Low) before it begins decoding. (66 min required on 7000 series)
   
    for (int loop_counter = 0; loop_counter < 2; loop_counter++) // 1 "loop" would be sufficient from my limited testing
    {
       inject_SCEI();
       //inject_SCEA();
       //inject_SCEE();
    }
   
    pinMode(data, INPUT); // high-z the data line, we're done
  }

// keep catching SUBQ packets forever
}


// Old readme!

//UPDATED AT MAY 14 2016, CODED BY THE FRIENDLY FRIETMAN :-)

//PsNee, an open source stealth modchip for the Sony Playstation 1, usable on
//all platforms supported by Arduino, preferably ATTiny. Finally something modern!


//--------------------------------------------------
//                    TL;DR
//--------------------------------------------------
//Look for the "Arduino selection!" section and verify the target platform. Hook up your target device and hit Upload!
//BEWARE: when using ATTiny45, make sure the proper device is selected (Extra=>Board=>ATTiny45 (internal 8MHz clock))
//and the proper fuses are burnt (use Extra=>Burn bootloader for this), otherwise PsNee will malfunction. A tutorial on
//uploading Arduino code via an Arduino Uno to an ATTiny device: http://highlowtech.org/?p=1695
//Look at the pinout for your device and hook PsNee up to the points on your Playstation.

//The modchip injects after about 1500ms the text strings SCEE SCEA SCEI on the motherboard point and stops
//with this after about 25 seconds. Because all the possible valid region options are outputted on the
//motherboard the Playstation gets a bit confused and simply accepts the inserted disc as authentic; after all,
//one of the codes was the same as that of the Playstation hardware...

//--------------------------------------------------
//               New in this version!
//--------------------------------------------------
//A lot!
// - The PAL SCPH-102 NTSC BIOS-patch works flawlessly! For speed reasons this is implemented in bare
//   AVR C. It is functionally identical to the OneChip modchip, this modchip firmware was disassembled,
//   documented (available on request, but written in Dutch...) and analyzed with a logic analyzer to
//   make sure PsNee works just as well.
// - The code now is segmented in functions which make the program a lot more maintable and readable
// - Timing is perfected, all discs (both backups and originals of PAL and NTSC games) now work in the
//   PAL SCPH-102 test machine
// - It was found out that the gate signal doesn't havbe to be hooked up to a PAL SCPH-102 Playstation
//   to circumvent the copy protection. This is not tested on other Playstation models so the signal still
//   is available
// - The /xlat signal is no longer required to time the PAL SCPH-102 NTSC BIOS-patch
// - Only AVR PORTB is used for compatibility reasons (almost all the AVR chips available have PORTB)
