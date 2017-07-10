//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(-1, 3); // RX, TX
// This PsNee version is meant for Arduino boards.
// 16Mhz and 8Mhz variants are supported. "Pro Micro" etc supported and recommended
// "Arduino Pro Micro" has a different pin assignment and needs porting. (ToDo)

// PAL PM-41 support isn't implemented yet. (ToDo)

// This code is multi-region, meaning it will unlock PAL, NTSC-U and NTSC-J machines.

// Use PU22_MODE for PU-22, PU-23, PM-41 mainboards.
boolean pu22mode;

//#define ARDUINO_UNO_BOARD
#define ATTINY_CHIP

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
#ifdef ATTINY_CHIP
  // board pins
  #define sqck 0
  #define subq 1
  #define data 2
  #define gate_wfck 4
  // MCU input / output
  #define SUBQPORT PINB
  #define SQCKBIT 0
  #define SUBQBIT 1
  #define GATEWFCKPORT PINB
  #define DATAPORT PORTB
  #define GATEWFCKBIT 4
  #define DATABIT 2
#endif

//Timing
const int delay_between_bits = 4000;      // 250 bits/s (microseconds)
const int delay_between_injections = 90;  // 72 in oldcrow. PU-22+ work best with 80 to 100 (milliseconds)

void inject_SCEX(char region, boolean firstPart)
{
  //SCEE: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01011101 00
  //SCEA: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01111101 00
  //SCEI: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01101101 00
  const boolean SCE[36] = {1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1};
  const boolean EData[8] = {0,1,1,1,0,1,0,0}; //SCEE
  const boolean AData[8] = {1,1,1,1,0,1,0,0}; //SCEA
  const boolean IData[8] = {1,0,1,1,0,1,0,0}; //SCEI
 
  const boolean *SCEXData;
  byte limit;
  if (firstPart) {
    SCEXData = SCE;
    limit = 36;
  }
  else {
    switch (region){
      case 'e': SCEXData = EData; break;
      case 'a': SCEXData = AData; break;
      case 'i': SCEXData = IData; break;
    }
    limit = 8;
  }
 
  // pinMode(data, OUTPUT) is used more than it has to be but that's fine.
  for (byte bit_counter = 0; bit_counter < limit; bit_counter++)
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

  if (firstPart){
    inject_SCEX(region, false);
  }

  pinMode(data, OUTPUT);
  bitClear(GATEWFCKPORT,DATABIT); // pull data low
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
 
  //mySerial.begin (19200);
  //mySerial.print("f "); mySerial.print(F_CPU);
 
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

  //Serial.print("highs: "); Serial.print(highs); Serial.print(" lows: "); Serial.println(lows);
  // typical readouts
  // PU-22: highs: 2449 lows: 2377
  if (lows > 100) {
    pu22mode = 1;
  }
  else {
    pu22mode = 0;
  }
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

    __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t");
    // sample the bit now!
    sample = bitRead(SUBQPORT, SUBQBIT);
    bitbuf |= sample << bitpos;
   
    do {
      // nothing
    } while ((bitRead(SUBQPORT, SQCKBIT)) == 0); // and high again..
   
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
//  if (!(scbuf[0] == 0 && scbuf[1] == 0 && scbuf[2] == 0 && scbuf[3] == 0)){ // a bad sector read is all 0 except for the CRC fields. Don't log it.
//    for (int i = 0; i<12;i++) {
//      if (scbuf[i] < 0x10) mySerial.print("0"); // padding
//        mySerial.print(scbuf[i], HEX);
//        mySerial.print(" ");
//      }
//      mySerial.println("");
//  }
 
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
   
    //mySerial.println("!");
   
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
      inject_SCEX('e', true); // e = SCEE, a = SCEA, i = SCEI
      inject_SCEX('a', true); // injects all 3 regions by default
      inject_SCEX('i', true); // makes it easier for people to get working
    }

    if (!pu22mode){
      pinMode(gate_wfck, INPUT); // high-z the line, we're done
    }
    pinMode(data, INPUT); // high-z the line, we're done
  }
// keep catching SUBQ packets forever
}
