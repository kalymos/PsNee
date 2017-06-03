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
//                     PPPPPPPPPPPP  P            P  PPPPPPPPPPP  PPPPPPPPPPP   VERSION 6!

//Update 15th of May 2017
//PSNee now watches the subchannel data and looks at the position information contained within.
//This allows deterministic SCEX injections. It knows (almost) exactly when to inject the SCEX string.
//Therefore it is now a stealth modchip :)
//Required connections: GND, VCC, data, gate, SQCL, SUBQ
//No more need to watch the PSX reset or lid open signals or any other typical modchip points (like "sync")
//WIP! Only tested on PU-18 board. Should work fine on PU-7, PU-8, PU-18 and PU-20.
//Will need adaption for PU-22 to PU-41 (SCPH-750x, 900x and PSOne).
//Note: Once this is installed in a PSX, mind the Pin13 LED that many Arduino boards have. Do not upload new sketches while the PSX is on!
//(If the PSX is on while uploading a sketch (making the LED blink), a voltage will be fed back into the SCLK pin on the HC-05 in the PSX.
//This didn't break my PSX in testing but it does stun the chip and halt CD operation. I'm thinking of a better method to do this but for now I need Arduino pin13..)
//Very much recommended to install a 3.3V chip!

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


//--------------------------------------------------
//                 General info!
//--------------------------------------------------
//PLAYSTATION 1 SECURITY - HOW IT DOES IT'S THING:
//Sony didn't really go through great lenghts to protect it's precious Playstation
//from running unauthorised software; the main security is based on a simple ASCII
//string of text that is read from a part of an original Playstation disc that cannot
//be reproduced by an ordinary PC CD burner.
//As most of you will know, a CD is basically a very long rolled up (carrier) string in which very
//little pits and ehm... little not-pits are embedded that represent the data stored on the disc.
//The nifty Sony engineers did not use the pits and stuff to store the security checks for
//Playstation discs but went crazy with the rolled up carrier string. In an ordinary CD, the
//string is rolled up so that the spacing between the tracks is as equal as possible. If that
//is not the case, the laser itself needs to move a bit to keep track of the track and
//reliably read the data off the disc.
//If you wonder how the laser knows when it follows the track optimally: four photodiodes, light
//intensity measurement, difference measurements, servo. There.
//To the point: the Sony engineers decidedly "fumbled up" the track of sector 4 on a Playstation
//disc (the track was modulated in nerd-speak) so that the error correction circuit outputs a
//recognisable signal, as the laser needs to be corrected to follow the track optimally.
//This output signal actually is a 250bps serial bitstream (with 1 start bit and 2 stop bits) which
//in plain ASCII says SCEA (Sony Computer Entertainment of America), SCEE (Sony Computer Entertainment
//of Europe) or SCEI (Sony Computer Entertainment of Japan), depending on the region of the disc inserted.
//The security thus functions not only as copy protection, but also as region protection.
//The text string from the disc is compared with the text string that is embedded in the Playstation
//hardware. When these text strings are the same, the disc is interpreted to be authentic and from
//the correct region. Bingo!

//HOW THE MODCHIP TRICKS THE PLAYSTATION:
//The modchip isn't all that of a complicated device: clever reverse engineers found the point on the
//Playstation motherboard that carried the text string from the disc and found a way to temporarily block
//this signal (by grounding an input of an op-amp buffer) to be able to inject the signal from the modchip
//The modchip injects after about 1500ms the text strings SCEE SCEA SCEI on the motherboard point and stops
//with this after about 25 seconds. Because all the possible valid region options are outputted on the
//motherboard the Playstation gets a bit confused and simply accepts the inserted disc as authentic; after all,
//one of the codes was the same as that of the Playstation hardware...
//Early modchips applied the text strings as long as power was applied to them, whereby later Playstation
//software could detect whether a modchip was installed. This is circumvented in this application by idling the
//modchip after about 25 seconds. The text strings are only tranmitted again when the CD lid is opened and closed
//again, to enable playing multi-disc games. This is also called a stealth modchip in marketing-speak.


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


//--------------------------------------------------
//                  Pinouts!
//--------------------------------------------------
//FOR ARDUINO UNO (WITH ATMEGA328):
// - Arduino pin 8  = data    = ATMega pin 14
// - Arduino pin 9  = gate    = ATMega pin 15
// - Arduino pin 10 = lid     = ATMega pin 16
// - Arduino pin 11 = biosA18 = ATMega pin 17
// - Arduino pin 12 = biosD2  = ATMega pin 18

//FOR ATTINY25/45/85:
// - Arduino pin 0 = data    = ATTiny pin 5
// - Arduino pin 1 = gate    = ATTiny pin 6
// - Arduino pin 2 = lid     = ATTiny pin 7
// - Arduino pin 3 = biosA18 = ATTiny pin 2
// - Arduino pin 4 = biosD2  = ATTiny pin 3

//--------------------------------------------------
//                    Includes!
//--------------------------------------------------
#include <Flash.h>
#include "SPI.h"

byte buf [12]; // We will be capturing PSX "SUBQ" packets, and there are 12 of them per sector.
int pos; // buffer position
int wobbleCounter; // if treshold reached, output SCEX string
//#define DEBUG
//--------------------------------------------------
//               Arduino selection!
//--------------------------------------------------
// ATTINY untested with SPI additions!
//#define ATTINY        //Make that "#define ARDUINO_UNO" if you want to compile for Arduino Uno instead of ATTiny25/45/85
#define ARDUINO_UNO        //Make that "#define ARDUINO_UNO" if you want to compile for Arduino Uno instead of ATTiny25/45/85

#ifdef ARDUINO_UNO
//Pins
int data = 8;         //The pin that outputs the SCEE SCEA SCEI string
int gate = 9;         //The pin that outputs the SCEE SCEA SCEI string
int lid = 10;         //The pin that gets connected to the internal CD lid signal; active high
int biosA18 = 11;     //Address 18; Only used in SCPH-102 PAL mode // ToDo: rearrange. This is the MOSI pin for SPI.
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

//--------------------------------------------------
//              Global variables!
//--------------------------------------------------
//None, just like it should be! // Sorry, couldn't resist
//--------------------------------------------------
//              Seperate functions!
//--------------------------------------------------
void NTSC_fix()
{
  //needs rework, new pin assigments

 
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
  //NTSC_fix(); //needs rework, new pin assigments
 
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

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//--------------------------------------------------
//     Setup function - execution starts here!
//--------------------------------------------------
void setup()
{
  // Arduino docs say all INPUT pins are high impedence by default. Let's be explicit!
  pinMode(data, INPUT);
  pinMode(gate, INPUT);

  // We just want passive sampling of the SPI data. Never output anything nor pull lines high / low.
  pinMode(MOSI, INPUT); // Arduino pin 11
  pinMode(SCK, INPUT); // Arduino pin 13
  pinMode(MISO, INPUT); // just for safety
  pinMode(SS, INPUT); // just for safety
 
  SPCR = 0b01101100; // LSBFIRST, SPI_MODE3, SPI_CLOCK_DIV4, SPI enabled, Slave mode, CPOL = 1, CPHA = 1
 
  //inject_playstation(); // not required when watching subchannel data
#ifdef DEBUG
  Serial.begin (115200);   // debugging
  Serial.print("SPCR setup to ");
  Serial.println(SPCR, HEX);
#endif
  pos = 0;
  wobbleCounter = 0;
}

//----------------------------------------------------------------
//   Loop function - executes after the initial injection cycle
//----------------------------------------------------------------

void loop()
{
  // Hardware SPI interface requires a constant reset (for some reason. Find out why?)
  // This toggles the SPI enable bit
  SPCR = SPCR & 0b10111111;
  SPCR = SPCR | 0b01000000;
  // When not doing this regularly, the captured data gets misaligned with the PSX SCLK. Weird.
 
  while(!(SPSR & (1<<SPIF))); // wait for a byte
 
  byte c = SPDR;
  if (c == 0x41) {
    buf[0] = c; // Is this a psx data cd? if so, align buffer start to this!
    pos = 1;
    return;
  }
  if (pos == 0) return; // 0x41 byte not yet found > loop
 
  buf[pos] = c;
  pos++;
 
  if(pos == 12) // 12 "packets" make up the SUBQ for one sector. pos is 12 because it got incremented once more.
  {
    pos = 0;
#ifdef DEBUG
    for (int i = 0; i<12;i++){ // print the buffer
      Serial.print(buf[i], HEX);
      Serial.print(" ");
    }
    Serial.println("");
#endif
    // check if this is the wobble area
    if (buf[1] == 0x00 && ( buf[2] == 0xA2 || buf[2] == 0xA1 || buf[2] == 0xA0 ) ) { // 0x41, 0x00, 0xA0 etc appears to be the wobble area
      //looks like it!
      wobbleCounter++;
    }
    else if (wobbleCounter > 0){
      wobbleCounter--;
    }
  }
 
  if (wobbleCounter >= 3) {
    // Read head is in wobble area. Inject SCEX.
#ifdef DEBUG
    Serial.println("Inject!");
#endif
    pinMode(gate, OUTPUT);
    digitalWrite(gate, 0);
   
    // loop_counter is a tweak point. More than 6 can trip antimod detection. 2 works. 1 is outputting once per sector in theory, doesn't work.
    for (int loop_counter = 0; loop_counter < 3; loop_counter = loop_counter + 1)
    {
       inject_SCEI();
       //inject_SCEA();
       //inject_SCEE();
    }

    pinMode(gate, INPUT);
    pinMode(data, INPUT);
   
    wobbleCounter = 0;
  }
}
