    // This PsNee version is meant for Arduino boards.
    // 16Mhz and 8Mhz variants are supported. "Pro Micro" etc supported and recommended
    // "Arduino Pro Micro" has a different pin assignment and needs porting. (ToDo)

    // PAL PM-41 support isn't implemented yet. (ToDo)

    // This code is multi-region, meaning it will unlock PAL, NTSC-U and NTSC-J machines.

    // Use PU22_MODE for PU-22, PU-23, PM-41 mainboards.
    boolean pu22mode;

    #define ARDUINO_UNO_BOARD

    #ifdef ARDUINO_UNO_BOARD
      // board pins
      #define sqck 6          // connect to PSX HC-05 SQCK pin
      #define subq 7          // connect to PSX HC-05 SUBQ pin
      #define data 8          // connect to point 6 in old modchip diagrams
      #define gate_wfck 9     // connect to point 5 in old modchip diagrams
      // MCU input / output
      #define SUBQPORT PIND       // Atmel MCU port for the 2 SUBQ sampling inputs
      #define SQCKBIT 6           // ATmega PD6 "SQCK" Mechacon pin 26 (PU-7 and early PU-8 Mechacons: pin 41)
      #define SUBQBIT 7           // ATmega PD7 "SUBQ" Mechacon pin 24 (PU-7 and early PU-8 Mechacons: pin 39)
      #define GATEWFCKPORT PINB   // Atmel MCU port for the gate input (used for WFCK)
      #define DATAPORT PORTB      // Atmel MCU port for the gate input (used for WFCK)
      #define GATEWFCKBIT 1       // ATmega PB1
      #define DATABIT 0           // ATmega PB0
    #endif

    //Timing
    const int delay_between_bits = 4000;      // 250 bits/s (microseconds)
    const int delay_between_injections = 90;  // 72 in oldcrow. PU-22+ work best with 80 to 100 (milliseconds)

    //SCEE: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01011101 00
    //SCEA: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01111101 00
    //SCEI: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01101101 00
    const boolean SCEEData[44] = {1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0}; //SCEE
    const boolean SCEAData[44] = {1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,1,1,1,1,0,1,0,0}; //SCEA
    const boolean SCEIData[44] = {1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,1,0,1,1,0,1,0,0}; //SCEI

    void inject_SCEX(char region)
    {
      const boolean *SCEXData;
      switch (region){
        case 'e': SCEXData = SCEEData; break;
        case 'a': SCEXData = SCEAData; break;
        case 'i': SCEXData = SCEIData; break;
      }

      digitalWrite(LED_BUILTIN, HIGH); // Arduino UNO Pin 13 / PB5
     
      // pinMode(data, OUTPUT) is used more than it has to be but that's fine.
      for (byte bit_counter = 0; bit_counter < 44; bit_counter = bit_counter + 1)
      {
        if (*(SCEXData+bit_counter) == 0)
        {
          pinMode(data, OUTPUT);
          bitClear(GATEWFCKPORT,DATABIT); // data low
          delayMicroseconds(delay_between_bits);
        }
        else
        {
          if (pu22mode) {
            pinMode(data, OUTPUT);
            unsigned long now = micros();
            do {
              bool wfck_sample = bitRead(GATEWFCKPORT, GATEWFCKBIT);
              bitWrite(DATAPORT,DATABIT,wfck_sample); // output wfck signal on data pin
            }
            while ((micros() - now) < delay_between_bits);
          }
          else { // not PU 22 mode
            pinMode(data, INPUT);
            delayMicroseconds(delay_between_bits);
          }
        }
      }
      pinMode(data, OUTPUT);
      bitClear(GATEWFCKPORT,DATABIT); // pull data low
      digitalWrite(LED_BUILTIN, LOW);
      delay(delay_between_injections);
    }

    //--------------------------------------------------
    //     Setup
    //--------------------------------------------------
    void setup()
    {
      pinMode(data, INPUT);
      pinMode(gate_wfck, INPUT);
      pinMode(subq, INPUT); // PSX spi data in
      pinMode(sqck, INPUT); // PSX spi clock in
     
      pinMode(LED_BUILTIN, OUTPUT); // Blink on injection / debug.
      digitalWrite(LED_BUILTIN, HIGH); // mark begin of setup
     
      Serial.begin (1000000);
      Serial.print("MCU frequency: "); Serial.print(F_CPU); Serial.println(" Hz");
      Serial.println("Waiting for SQCK..");
     
      // Board detection
      while (!digitalRead(sqck));   // wait for console power on (in case Arduino is powered externally)
      while (!digitalRead(gate_wfck));   // wait for gate / WFCK signal to appear

      // GATE: __-----------------------  // this is a PU-7 .. PU-20 board!
      //
      // WFCK: __-_-_-_-_-_-_-_-_-_-_-_-  // this is a PU-22 or newer board!

      unsigned int highs, lows = 0;
      unsigned long now = millis();
      do {
        if (digitalRead(gate_wfck) == 1) highs++;
        if (digitalRead(gate_wfck) == 0) lows++;
        delayMicroseconds(200);   // good for ~5000 reads in 1s
      }
      while ((millis() - now) < 1000); // sample 1s

      Serial.print("highs: "); Serial.print(highs); Serial.print(" lows: "); Serial.println(lows);
      // typical readouts
      // PU-22: highs: 2449 lows: 2377
      if (lows > 100) {
        pu22mode = 1;
      }
      else {
        pu22mode = 0;
      }
      Serial.print("pu22mode: "); Serial.println(pu22mode);

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

      digitalWrite(LED_BUILTIN, LOW); // setup complete
    }

    void loop()
    {
      static byte scbuf [12] = { 0 }; // We will be capturing PSX "SUBQ" packets, there are 12 bytes per valid read.
      static unsigned int timeout_clock_counter = 0;
      static byte bitbuf = 0;   // SUBQ bit storage
      static bool sample = 0;
     
      byte scpos = 0;           // scbuf position
     
      noInterrupts(); // start critical section
    // yes, a goto jump label. This is to avoid a return out of critical code with interrupts disabled.
    // It prevents bad behaviour, for example running the Arduino Serial Event routine without interrupts.
    // Using a function makes shared variables messier.
    // SUBQ sampling is essential for the rest of the functionality. It is okay for this to take as long as it does.
    start:
      for (byte bitpos = 0; bitpos<8; bitpos++) { // Capture 8 bits for 12 runs > complete SUBQ transmission
        do {
          // nothing, reset on timeout
          timeout_clock_counter++;
          if (timeout_clock_counter > 1000){
            scpos = 0;  // reset SUBQ packet stream
            timeout_clock_counter = 0;
            bitpos = 0;
            goto start;
          }
        }
        while (bitRead(SUBQPORT, SQCKBIT) == 1); // wait for clock to go low..

        do {
          // nothing
        } while ((bitRead(SUBQPORT, SQCKBIT)) == 0); // and high again..
       
        // sample the bit now!
        sample = bitRead(SUBQPORT, SUBQBIT);
        bitbuf |= sample << bitpos;
       
        timeout_clock_counter = 0; // no problem with this bit
      }
     
      scbuf[scpos] = bitbuf;
      scpos++;
      bitbuf = 0;

      // repeat for all 12 bytes
      if (scpos < 12){
        goto start;
      }

      interrupts(); // end critical section

      // log SUBQ packets
      if (!(scbuf[0] == 0 && scbuf[1] == 0 && scbuf[2] == 0 && scbuf[3] == 0)){ // a bad sector read is all 0 except for the CRC fields. Don't log it.
        for (int i = 0; i<12;i++) {
          if (scbuf[i] < 0x10) Serial.print("0"); // padding
            Serial.print(scbuf[i], HEX);
            Serial.print(" ");
          }
          Serial.println("");
      }

      // check if read head is in wobble area
      // We only want to unlock game discs (0x41) and only if the read head is in the outer TOC area.
      // We want to see a TOC sector repeatedly before injecting (helps with timing and marginal lasers).
      static byte hysteresis  = 0;

      // All this logic is because we don't know if the HC-05 is actually processing a getSCEX() command.
      // Hysteresis is used because older drives exhibit more wiggle room. They might see a few TOC sectors when they shouldn't.
      if (
        (scbuf[0] == 0x41 &&  scbuf[1] == 0x00 &&  scbuf[6] == 0x00) &&   // [0] = 41 means psx game disk. the other 2 checks are garbage protection
        (scbuf[2] == 0xA0 || scbuf[2] == 0xA1 || scbuf[2] == 0xA2 ||      // if [2] = A0, A1, A2 ..
        (scbuf[2] == 0x01 && (scbuf[3] >= 0x98 || scbuf[3] <= 0x02) ) )   // .. or = 01 but then [3] is either > 98 or < 02
      ) {
        hysteresis++;
      }
      else if ( hysteresis > 0 &&
              ((scbuf[0] == 0x01 || scbuf[0] == 0x41) && (scbuf[1] == 0x00 /*|| scbuf[1] == 0x01*/) &&  scbuf[6] == 0x00)
      ) {  // This CD has the wobble into CD-DA space. (started at 0x41, then went into 0x01)
        hysteresis++;
      }
      else if (hysteresis > 0) {
        hysteresis--; // None of the above. Initial detection was noise. Decrease the counter.
      }

      // Some anti mod routines position the laser very close to the TOC area. Only inject if we're pretty certain it is required.
      // hysteresis below 10 occasionally triggers injections in Silent Hill (NTSC-J) when using a worn drive
      if (hysteresis >= 14){
        hysteresis = 0;
        Serial.println("INJECT!INJECT!INJECT!INJECT!INJECT!INJECT!INJECT!INJECT!INJECT!");

        pinMode(data, OUTPUT);
        digitalWrite(data, 0); // pull data low
        if (!pu22mode){
          pinMode(gate_wfck, OUTPUT);
          digitalWrite(gate_wfck, 0);
        }
       
        // HC-05 is waiting for a bit of silence (pin low) before it begins decoding.
        delay(delay_between_injections);
        for (byte loop_counter = 0; loop_counter < 2; loop_counter++)
        {
          inject_SCEX('e'); // e = SCEE, a = SCEA, i = SCEI
          inject_SCEX('a'); // injects all 3 regions by default
          inject_SCEX('i'); // makes it easier for people to get working
        }

        if (!pu22mode){
          pinMode(gate_wfck, INPUT); // high-z the line, we're done
        }
        pinMode(data, INPUT); // high-z the line, we're done
      }
    // keep catching SUBQ packets forever
    }


    case 'e': SCEXData = SCEEData; break;
    case 'a': SCEXData = SCEAData; break;
    case 'i': SCEXData = SCEIData; break;
  }

  digitalWrite(LED_BUILTIN, HIGH); // this is Arduino Pin 13 / PB5

  for (byte bit_counter = 0; bit_counter < 44; bit_counter = bit_counter + 1)
  {
    if (*(SCEXData+bit_counter) == 0)
    {
      bitClear(PORTB,0); // pull data low
      delayMicroseconds(delay_between_bits);
    }
    else
    {
      unsigned long now = micros();
      do {
#ifdef PU22_MODE
        bitWrite(PORTB,0,1); // output high
        delayMicroseconds(gate_high);
        bitWrite(PORTB,0,0); // output low
        delayMicroseconds(gate_low);
#else
        bitSet(PORTB,0); // drag data pin high
#endif
      }
      while ((micros() - now) < delay_between_bits);
      //Serial.println((micros() - now));
    }
  }
  bitClear(PORTB,0); // pull data low
  delay(delay_between_injections);
 
  digitalWrite(LED_BUILTIN, LOW);
}

//--------------------------------------------------
//     Setup
//--------------------------------------------------
void setup()
{
  pinMode(data, INPUT); // Arduino pin 8, ATmega PB0
  pinMode(SUBQ, INPUT); // spi data in Arduino pin 10, ATmega PB2
  pinMode(SQCK, INPUT); // spi clock Arduino pin 11, ATmega PB3
  pinMode(LED_BUILTIN, OUTPUT); // Blink on injection / debug.
  Serial.begin (1000000);
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

  static unsigned int timeout_clock_counter = 0;
  static byte bitbuf = 0;         // SUBQ bit storage
  static bool sample = 0;

  // Capture 8 bits per loop run.
  // unstable clock, bootup, reset and disc changes are ignored
  noInterrupts(); // start critical section

// yes, a goto jump label. This is to avoid a return out of critical code with interrupts disabled.
// It prevents bad behaviour, for example running the Arduino Serial Event routine without interrupts.
// Using a function makes shared variables messier.
// SUBQ sampling is essential for the rest of the functionality. It is okay for this to take as long as it does.
start:

  for (byte bitpos = 0; bitpos<8; bitpos++) {
    do {
      // nothing, reset on timeout
      timeout_clock_counter++;
      if (timeout_clock_counter > sampling_timeout){
        scpos = 0;  // reset SUBQ packet stream
        timeout_clock_counter = 0;
        num_resets++;
        bitpos = 0;
        goto start;
      }
    }
    while (bitRead(PINB, 3) == 1); // wait for clock to go low
   
#if F_CPU == 16000000 // wait a few cpu cycles > better readings in tests
    __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t");
#endif

    // sample the bit.
    sample = bitRead(PINB, 2);
    bitbuf |= sample << bitpos;

    do {
      // nothing
    } while ((bitRead(PINB, 3)) == 0); // Note: Even if sampling is bad, it will not get stuck here. There will be clock pulses eventually.

    timeout_clock_counter = 0; // This bit came through fine.
  }
 
  scbuf[scpos] = bitbuf;
  scpos++;
  bitbuf = 0;

  if (scpos < 12){
    goto start;
  }

  interrupts(); // end critical section
 
  // logging.
  if (!(scbuf[0] == 0 && scbuf[1] == 0 && scbuf[2] == 0 && scbuf[3] == 0)){ // a bad sector read is all 0 except for the CRC fields. Don't log it.
    for (int i = 0; i<12;i++) {
      Serial.print(scbuf[i], HEX);
      Serial.print(" ");
    }
    Serial.print(" resets:  ");
    Serial.println(num_resets);
  }

  num_resets = 0;
  scpos = 0;
 
  // check if this is the wobble area
  // 3 bytes would be enough to recognize it. The extra checks just ensure this isn't a garbage reading.
  if ( (scbuf[0] == 0x41 &&  scbuf[1] == 0x00 &&  scbuf[6] == 0x00) && // 0x41 = psx game, beginning of the disc, sanity check
    ((scbuf[2] == 0xA0 || scbuf[2] == 0xA1 || scbuf[2] == 0xA2) ||
    (scbuf[2] > 0x00 && scbuf[2] <= 0x99)) ){ // lead in / wobble area
   
    Serial.println("INJECT!");

    pinMode(data, OUTPUT); // prepare for SCEX injection

    bitClear(PORTB,0); // pull data low
   
    // HC-05 is waiting for a bit of silence (pin Low) before it begins decoding.
     // minimum 66ms required on SCPH-7000
     // minimum 79ms required on SCPH-7502 // wrong! got to keep the NRZ signal in mind for PU22+ !
     delay(82);
   
    for (int loop_counter = 0; loop_counter < 2; loop_counter++)
    {
       inject_SCEX('e'); // e = SCEE, a = SCEA, i = SCEI
       inject_SCEX('a'); // injects all 3 regions by default
       inject_SCEX('i'); // makes it easier for people to get working
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
