//                    PPPPPPPPPPPPPPPP                  P            P       
//                   P              P                  PP           P       
//                  P              P                  P P          P         
//                 P              P                  P  P         P         
//                P              P                  P   P        P           
//               P              P                  P    P       P           
//              P              P                  P     P      P             
//             PPPPPPPPPPPPPPPP  PPPPPPPPPPP     P      P     P  PPPPPPPPPPP  PPPPPPPPPPP
//            P                 P               P       P    P  P            P
//           P                 P               P        P   P  P            P
//          P                 P               P         P  P  P            P   
//         P                 P               P          P P  P            P   
//        P                 PPPPPPPPPPPPPP  P           PP  PPPPPPP      PPPPPPP   
//       P                              P  P            P  P            P     
//      P                              P  P            P  P            P       
//     P                              P  P            P  P            P       
//    P                              P  P            P  P            P       
//   P                              P  P            P  P            P       
//  P                              P  P            P  P            P         
//                     PPPPPPPPPPPP  P            P  PPPPPPPPPPP  PPPPPPPPPPP   VERSION 7!

//Update 15th of May 2017
//PSNee now watches the subchannel data and looks at the position information contained within.
//This allows deterministic SCEX injections. It knows (almost) exactly when to inject the SCEX string.
//Therefore it is now a stealth modchip :)
//Required connections: GND, VCC, data, gate, SQCL, SUBQ
//No more need to watch the PSX reset or lid open signals or any other typical modchip points (like "sync")
//WIP! Only tested on PU-18 board. Should work fine on PU-7, PU-8, PU-18 and PU-20.
//Will need adaption for PU-22 to PU-41 (SCPH-750x, 900x and PSOne).

//UPDATED AT MAY 14 2016, CODED BY THE FRIENDLY FRIETMAN :-)

//PsNee, an open source stealth modchip for the Sony Playstation 1, usable on
//all platforms supported by Arduino, preferably ATTiny. Finally something modern!

// - Only AVR PORTB is used for compatibility reasons (almost all the AVR chips available have PORTB)
// todo

//--------------------------------------------------
//                  Pinouts!
//--------------------------------------------------
//FOR ARDUINO UNO (WITH ATMEGA328):
// - Arduino pin 2  = spiclock = ATMega pin 4
// - Arduino pin 8  = data     = ATMega pin 14
// - Arduino pin 9  = gate     = ATMega pin 15
// - Arduino pin 10 = spidata  = ATMega pin 16
// - Arduino pin 11 = biosA18  = ATMega pin 17
// - Arduino pin 12 = biosD2   = ATMega pin 18

//FOR ATTINY25/45/85:

#include <Flash.h>

byte scbuf [12]; // We will be capturing PSX "SUBQ" packets, and there are 12 of them per sector.
byte scpos; // buffer position
byte bitbuf; // SUBQ bits get stored in here as they fly in
byte bitpos; // bitbuf index

//--------------------------------------------------
//               Arduino selection!
//--------------------------------------------------
// ATTINY untested yet!
//#define ATTINY       
#define ARDUINO_UNO

#ifdef ARDUINO_UNO
//Pins
int spiclock = 2;     // PD2 on ATmega328
int spidata = 10;     // PB2 on ATmega328
int data = 8;         //The pin that outputs the SCEE SCEA SCEI string
int gate = 9;         //The pin that outputs the SCEE SCEA SCEI string
int biosA18 = 11;     //Address 18; Only used in SCPH-102 PAL mode
int biosD2 = 12;      //Data 2; Only used in SCPH-102 PAL mode
int delay_ntsc = 2350;
int delay_between_bits = 4; // 250 bits/s
int delay_between_injections = 74; // delay for this time while keeping data line pulled low
#endif

#ifdef ATTINY
//Pins
int data = 0;        //The pin that outputs the SCEE SCEA SCEI string
int gate = 1;
int lid = 2;         //The pin that gets connected to the internal CD lid signal; active high
int biosA18 = 3;     //Only used in SCPH-102 PAL mode
int biosD2 = 4;      //Only used in SCPH-102 PAL mode
int delay_ntsc = 2400;
int delay_between_bits = 4;
int delay_between_injections = 68;
#endif

#if F_CPU == 8000000
  #define TIMEOUT_CLOCK_LOW 24 // minimum 18
  #define TIMEOUT_CLOCK_HIGH 6 // minimum 3
#elif F_CPU == 16000000
  #define TIMEOUT_CLOCK_LOW 72 // minimum 54
  #define TIMEOUT_CLOCK_HIGH 14 // minimum 7
#endif

void NTSC_fix() //needs rework, new pin assigments
{
 
  //Make sure all pins are inputs
  DDRB = 0x00;
 
  //Wait until just before the pulse on BIOS A18 arrives
  delay(delay_ntsc);
 
  //...And wait here until it actually happened
  while(!(PINB & B00001000))
  {
    ;  //Wait
  }
  delayMicroseconds(12);
  PORTB = B00000000;
  DDRB = B00010000;
  delayMicroseconds(5);
  DDRB = 0x00;
}

void inject_SCEE()
{
  //SCEE-array                                                                                                                   //      Start            Data     Stop
  FLASH_ARRAY (boolean, SCEEData, 1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0);      //SCEE: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01011101 00   44 bits total
 
  int bit_counter;

  for (bit_counter = 0; bit_counter < 44; bit_counter = bit_counter + 1)
  {
    if (SCEEData[bit_counter] == 0)
    {       
      pinMode(data, OUTPUT);
      digitalWrite(data, 0);
      delay(delay_between_bits);
    }
    else
    {
      pinMode(data, INPUT);                //We make the data pin high-impedance to let the pull-up of the Playstation motherboard make a 1
      delay(delay_between_bits);
    }
  }

  pinMode(data, OUTPUT);
  digitalWrite(data, 0);
  delay(delay_between_injections);
}

void inject_SCEA()
{
  //SCEE-array                                                                                                                   //      Start            Data     Stop
  FLASH_ARRAY (boolean, SCEAData, 1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,1,1,1,1,0,1,0,0);      //SCEA: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01111101 00
 
  int bit_counter;

  for (bit_counter = 0; bit_counter < 44; bit_counter = bit_counter + 1)
  {
    if (SCEAData[bit_counter] == 0)
    {       
      pinMode(data, OUTPUT);
      digitalWrite(data, 0);
      delay(delay_between_bits);
    }
    else
    {
      pinMode(data, INPUT);                //We make the data pin high-impedance to let the pull-up of the Playstation motherboard make a 1
      delay(delay_between_bits);
    }
  }

  pinMode(data, OUTPUT);
  digitalWrite(data, 0);
  delay(delay_between_injections);
}

void inject_SCEI()
{
  //SCEI-array                                                                                                                   //      Start            Data     Stop
  FLASH_ARRAY (boolean, SCEIData, 1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,1,0,1,1,0,1,0,0);      //SCEI: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01101101 00
 
  int bit_counter;
 
  for (bit_counter = 0; bit_counter < 44; bit_counter = bit_counter + 1)
  {
    if (SCEIData[bit_counter] == 0)
    {       
      pinMode(data, OUTPUT);
      digitalWrite(data, 0);
      delay(delay_between_bits);
    }
    else
    {
      pinMode(data, INPUT);                //We make the data pin high-impedance to let the pull-up of the Playstation motherboard make a 1
      delay(delay_between_bits);
    }
  }

  pinMode(data, OUTPUT);
  digitalWrite(data, 0);
  delay(delay_between_injections);
}

void inject_multiple_times(int number_of_injection_cycles)
{
  int cycle_counter;
 
  for(cycle_counter = 0; cycle_counter < number_of_injection_cycles; cycle_counter = cycle_counter + 1)
  {
    inject_SCEE();
    inject_SCEA();
    inject_SCEI();
  }
}

void inject_playstation()
{
  //NTSC_fix();
 
  delay(6900);
  pinMode(data, OUTPUT);
  pinMode(gate, OUTPUT);
  digitalWrite(data, 0);
  digitalWrite(gate, 0);

  for (int loop_counter = 0; loop_counter < 235; loop_counter = loop_counter + 1)
  {
    inject_SCEI();
    //inject_SCEA();
    //inject_SCEE();
  }
 
  pinMode(gate, INPUT);
  pinMode(data, INPUT);
}

//--------------------------------------------------
//     Setup
//--------------------------------------------------
void setup()
{
  // Arduino docs say all INPUT pins are high impedence by default. Let's be explicit!
  pinMode(data, INPUT); // Arduino pin 8
  pinMode(gate, INPUT); // Arduino pin 9
  pinMode(spidata, INPUT); // spi data in Arduino pin 10
  pinMode(spiclock, INPUT); // spi clock Arduino pin 2

  scpos = 0;
  bitpos = 0;
  bitbuf = 0;
 
  Serial.begin (115200);
  Serial.print("Start ");
 
#ifdef ARDUINO_UNO
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
#endif
}

#ifdef ARDUINO_UNO
  #define SUBQ_SDI_BITN 2
  #define SUBQ_SDI_BIT (1<<SUBQ_SDI_BITN)
#else
  // todo: attiny
#endif

void loop()
{
  unsigned int timeout_clock_low_counter = 0;
  unsigned int timeout_clock_high_counter = 0;
 
  for (int i = 0; i<8; i++) {
    do {
      // waste/count cycles. abort and reset if the clock times out.
      timeout_clock_low_counter++;
      if (timeout_clock_low_counter > TIMEOUT_CLOCK_LOW){
        bitbuf = 0;
        bitpos = 0;
        scpos = 0;
        return;
      }
    }
    while ((PIND & SUBQ_SDI_BIT)); // wait for clock to go low
    timeout_clock_low_counter = 0;

    // clock is stable. sample the bit.
    bool sample = (PINB & SUBQ_SDI_BIT);
    bitbuf |= sample << bitpos;
    bitpos++;

    do {
      // waste/count cycles. abort and reset if the clock times out.
      timeout_clock_high_counter++;
      if (timeout_clock_high_counter > TIMEOUT_CLOCK_HIGH){
        bitbuf = 0;
        bitpos = 0;
        scpos = 0;
        return;
      }
    }
    while (!(PIND & SUBQ_SDI_BIT)); // wait for clock to go high
    timeout_clock_high_counter = 0;
  }
 
  // Align the 12 byte buffer to 0x41, which is the start bit for a PSX game disc.
  // This serves following purposes:
  // - removes need for a start condition signal / extra wire
  // - always "rights the ship", for those start conditions where the clock is unstable (ie: autofocus, etc)
  // - it's pointless to unlock a music cd.
  if (bitbuf == 0x41){
    scbuf[0] = bitbuf;
    scpos = 1; // we are aligned. From now on catch the remaining 11 bytes for the full SUBQ readout for this sector.
    bitpos = 0;
    bitbuf = 0;
    return;
  }
 
  if (scpos == 0){ // catch stray packets and realign.
    bitpos = 0;
    bitbuf = 0;
    return;
  }

  scbuf[scpos] = bitbuf;
  bitpos = 0;
  bitbuf = 0;
  scpos++;

  if (scpos == 12){
    // end of time critical section. We now have all 12 subchannel packets. It will be 13.3ms until the next ones.
    for (int i = 0; i<12;i++) {
      Serial.print(scbuf[i], HEX);
      Serial.print(" ");
    }
    Serial.println("");
  }
  else return;

  scpos = 0;
 
  // check if this is the wobble area
  if ( scbuf[0] == 0x41 &&  scbuf[1] == 0x00 &&  scbuf[6] == 0x00 && // 0x41 = psx game, the 0x00 checks make sure (reasonably) that this is a valid 12 packet stream
    (scbuf[2] == 0xA0 || scbuf[2] == 0xA1 || scbuf[2] == 0xA2) ){ // lead in / wobble area is marked by 0xA0, 0xA1, 0xA2

    Serial.println("Inject!");

    pinMode(gate, OUTPUT);
    digitalWrite(gate, 0);

    // loop_counter is a tweak point. More than 6 can trip antimod detection. 2 works. 1 would require different timing.
    for (int loop_counter = 0; loop_counter < 3; loop_counter = loop_counter + 1)
    {
       inject_SCEI();
       //inject_SCEA();
       //inject_SCEE();
    }
   
    pinMode(gate, INPUT);
    pinMode(data, INPUT);
  }
}
