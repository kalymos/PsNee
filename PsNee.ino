//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM|||||
//||||||M*************************************************************M|||||
//||||||M*************************************************************M|||||
//||||||M***QQQQQQQQQ***************Q******Q**************************M|||||
//||||||M***Q********Q**************QQ*****Q**************************M|||||
//||||||M***Q********Q**************QQ*****Q**************************M|||||
//||||||M***Q********Q**************Q*Q****Q**************************M|||||
//||||||M***Q********Q**************Q*Q****Q**************************M|||||
//||||||M***Q********Q**************Q*Q****Q**************************M|||||
//||||||M***Q********Q**************Q**Q***Q**************************M|||||
//||||||M***QQQQQQQQQ***************Q**Q***Q**************************M|||||
//||||||M***Q***********QQQQQQQQ****Q***Q**Q******QQQQ*******QQQQ*****M|||||
//||||||M***Q**********QQ******Q****Q***Q**Q*****QQ**Q******QQ**Q*****M|||||
//||||||M***Q**********Q************Q***Q**Q****QQ***Q*****Q***QQ*****M|||||
//||||||M***Q**********QQQ**********Q****Q*Q***QQ**QQQ****QQQQQQ******M|||||
//||||||M***Q************QQQQQ******Q****Q*Q***QQQQQ******QQ**********M|||||
//||||||M***Q****************QQQ****Q*****QQ**QQ**********Q***********M|||||
//||||||M***Q******************Q****Q*****QQ**Q**********QQ***********M|||||
//||||||M***Q**********Q*******QQ***Q*****QQ**Q******QQ**Q******QQ****M|||||
//||||||M***Q**********QQ******QQ***Q******Q**QQ****QQ***QQ****QQ*****M|||||
//||||||M***Q***********QQQQQQQQ****Q******Q***QQQQQ******QQQQQQ******M|||||
//||||||M*************************************************************M|||||
//||||||M*************************************************************M|||||
//||||||MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM|||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||| VERSION 2! |||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//BETA BETA BETA BETA BETA BETA BETA BETA BETA BETA BETA BETA BETA BETA BETA
//Updated on September 10, coded by TheFrietMan, as usual.
//
//PsNee, an open source stealth modchip for the Sony Playstation 1, usable on
//all platforms supported by Arduino, preferably ATTiny. Finally something modern!
//
//
/////////// TO DO: /////////////
// - Make SCEx-arrays smaller by commoning up the common parts of the arrays and thus use less flash
// - Tidy everything up by using functions instead of code-knitting
// - Implement what needs to be done when it is detected that PsNee is connected to a PU-22, PU-23 or PSOne
// - Slice the data signal in sync with an internal Playstation clock (there should be one on the gate-pin,
//   this is the 7.35kHz WFCK) instead of using a free running clock from the modchip
////////////////////////////////
//
//
/////////// VERSION 2! :D ///////////
//What has changed?
// - Thanks to TriMesh, the gate-pin is now also used to determine in which model of
//   Playstation PsNee is installed. The modchip algorithm thus can be optimized for
//   optimal performance on specific Playstation revisions. This works by monitoring
//   whether a clock signal is present on this pin - when there is one, the modchip is
//   installed in a PU-22, PU-23 or PSOne Playstation, else it is installed in an older
//   model Playstation. In this version of PsNee, nothing is actually done with this information.
// - Thanks to -again- TriMesh, NTSC support for PAL SCPH-102 Playstations is added! This uses
//   the same method the OneChip modchip used for achieving this:
//     1. Monitor the XLAT signal from the CD mechanism controller chip. This requires
//        another connection to the Playstation. When this signal is 0, the first CD copy
//        protection is passed! After this, there is another one.
//     2. After this, watch the Address18-pin (pin 31) on the BIOS-chip. When this signal
//        is high, this means the second CD copy protection is about to run.
//     3. Wait a short time.
//     4. Pull the Data2-pin (pin 15) on the BIOS-chip to 0. This effectively blocks the
//        execution of the region check of the inserted disc.
//     5. The Playstation plays the inserted disc and doesn't care whether it's PAL or NTSC!
//     6. Release the 0 of the Data2-pin.
//   To correctly output a PAL video color signal for a PAL TV on a PAL PSOne with an NTSC disc
//   inserted, Pin 3 of IC502 must be grounded with an external switch. The modchip also could do
//   this, although we would need a device with more pins available.
// - The outputted data signal is now "sliced up" to improve (or less distort) the tracking
//   signal from the CD mechanism: later Playstations use the CD tracking signal for transmitting
//   the SCEx-string to the Playstation instead of using a seperate connection, so when the modchip
//   forces a 0 on the data-pin, the tracking signal also is gone temporarily. By slicing the data-
//   signal up in little pieces at least some of the tracking signal remains and the Playstation can
//   read discs more easily.
// - The two big for-loops are combined into one with an OR-statement describing the two conditions
//   modchip should be active: when flagFirstCycle = 0 or when flagFirstCycle = 1 and the lid is opened
//   and closed again. This makes code maintenance easier.
// - The pin-out of the modchip is changed slightly to be able to use an interrupt for the PAL=>NTSC
//   BIOS-patch for PAL SCPH-102. Please use the revised pin-out found below with this code.
//~TheFrietMan, The Netherlands
/////////////////////////////////////
//
//
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
//namely is not the case, the laser itself needs to move a bit to keep track of the track and
//reliably read the data off the disc.
//If you wonder how the laser knows when it follows the track optimally: four photodiodes, light
//intensity measurement, difference measurements, servo. There.
//To the point: the Sony engineers decidedly "fumbled up" the track of sector 4 on a Playstation
//disc (the track was modulated in nerd-speak) so that the error correction circuit outputs a
//recognisable signal, as the laser needs to be corrected to follow the track optimally.
//This outputted signal actually is a 250bps serial bitstream (with 1 startbit and 2 stopbits) which
//in plain ASCII says SCEA (Sony Computer Entertainment of America), SCEE (Sony Computer Entertainment
//of Europe) or SCEI (Sony Computer Entertainment of Japan), depending on the region of the disc inserted.
//The security thus functions not only as copy protection, but also as region protection.
//The text string from the disc is compared with the text string that is embedded in the Playstation
//hardware. When these text strings are the same, the disc is interpreted to be authentic and from
//the correct region. Bingo!
//
//HOW THE MODCHIP TRICKS THE PLAYSTATION:
//The modchip isn't all that of a complicated device: clever reverse engineers found the point on the
//Playstation motherboard that carried the text string from the disc and found a way to temporarily block
//this signal (by grounding an input of an op-amp buffer) to be able to inject the signal from the modchip
//The modchip injects after about 1500ms the text strings SCEE SCEA SCEI on the motherboard point and stops
//with this after about 25 seconds. Because all the possible valid region options are outputted on the
//motherboard the Playstation gets a bit confused and simply accepts the inserted disc as authentic; after all,
//one of the codes was the same as that of the Playstation hardware...
//Early modchips applied the text strings as long as there was applied power to them, whereby later Playstation
//software could detect whether a modchip was installed. This is circumvented in this application by idling the
//modchip after about 25 seconds. The text strings are only tranmitted again when the CD lid is opened and closed
//again, to enable playing multi-disc games. This is also called a stealth modchip in marketing-speak.
//
//This code is verified on an ATTiny45 with the 8MHz internal oscillator using a Saleae Logic Analyser, timing is
//reasonable important in this application.
//Version 1 was kindly coded and documented by TheFrietMan, August 20 2015, The Netherlands.
//The Playstation is great but nothing beats our national pride, the Philips CDi! Cheesiness for the win!
//
//
//PINOUT IC:
//  ATTiny45, DIP package:
//    Pin 1: OUT - PAL SCPH-102 BIOS Data2
//    Pin 2: IN - PAL SCPH-102 BIOS Address18
//    Pin 3: IN - CD lid
//    Pin 4: Ground
//    Pin 5: OUT - Data
//    Pin 6: OUT - Gate
//    Pin 7: IN - XLAT      !This is also where INT0 lives on ATTiny45!
//    Pin 8: Vcc
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#include <Flash.h>     //Include the Flash library to conveniently store the SCExData-arrays in PROGMEM, see http://arduiniana.org/libraries/flash/
 
//BLOODY GLOBAL VARIABLES
//Pins for the Arduino Uno
//int data = 0;        //The pin that outputs the SCEE SCEA SCEI string
//int gate = 1;        //The pin that gets pulled low to enable data injection on the Playstation motherboard
//int xlat = 2;        //On Arduino Uno, Arduino pin 2 is connected to INT0
//int lid = 3;         //The pin that gets connected to the internal CD lid signal; active high
//int biosA18 = 4;     //Only used in SCPH-102 PAL mode
//int biosD2 = 5;      //Only used in SCPH-102 PAL mode
 
//SET THIS TO 1 IF YOU HAVE A PAL SCPH-102 PLAYSTATION AND WANT TO PLAY NTSC DISCS ON A PAL TV WITHOUT A BOOT DISC
//THIS REQUIRES A MODCHIP WITH MORE PINS. I SUGGEST USING AN ATTINY2313. THE PIN-OUT FOR THIS CHIP IS FOUND A BIT UP
boolean PALSCPH102 = 0;
 
//Arduino pins for ATTiny45
int data = 0;        //The pin that outputs the SCEE SCEA SCEI string
int gate = 1;        //The pin that gets pulled low to enable data injection on the Playstation motherboard
int xlat = 2;
int biosA18 = 3;     //Only used in SCPH-102 PAL mode
int lid = 4;         //The pin that gets connected to the internal CD lid signal; active high
int biosD2 = 5;      //Only used in SCPH-102 PAL mode
 
 
//Flags
boolean flagFirstCycle = 0;               //This flag is 1 when the system completes the first cycle of SCEx-string outputting; this is done to stealth the chip up
boolean flagNewPlaystationModel = 0;      //This flag is 1 when it is detected that the modchip is running on PU-22, PU-23 or PSOne hardware
boolean flagBIOSAlreadyFooled = 0;        //This flag is 1 when the NTSC=>PAL BIOS-patch for PAL SCPH-102 consoles has been applied. It is reset to 0 when the lid is opened and closed again or when the console is turned off and on again.
 
 
//The SCPH102BIOSFooler()-function fools the PAL SCPH-102 into thinking that it also can read NTSC discs
//We execute this code when XLAT goes from high to low, indicating that the first copy protection check has been passed
//delay() doesn't work in an interrupt handler, we use delayMicroseconds() instead, that doesn't rely on using interrupts itself
void SCPH102BIOSFooler()
{
  boolean biosA18Status = digitalRead(biosA18);
  int addressSignalWaitCounter;
 
  if (flagBIOSAlreadyFooled == 0)
  {
    for (addressSignalWaitCounter = 0; addressSignalWaitCounter < 50; addressSignalWaitCounter = addressSignalWaitCounter + 1)
    {
      if (biosA18Status == 1)
      {
        delayMicroseconds(50);      //Wait a short time
        pinMode(biosD2, OUTPUT);
        digitalWrite(biosD2, 0);    //Pull the Data2-pin low for a millisecond
        delayMicroseconds(1000);
        pinMode(biosD2, INPUT);     //...And release it again
        flagBIOSAlreadyFooled = 1;
        return;
      }      
    }
  }
  else
  {
    return;
  }
}
 
 
void setup()
{
  pinMode(data, INPUT);    //The pins are high-impedance when configured as inputs so they don't interfere with the Playstation mortherboard circuitry
  pinMode(gate, INPUT);    
  pinMode(lid, INPUT);
  pinMode(xlat, INPUT);
 
  if (PALSCPH102 != 0)
  {
    pinMode(biosA18, INPUT);    //Only activate the BIOS-pins when connected to a PAL SCPH-102
    pinMode(biosD2, INPUT);
   
    //We make an interrupt for XLAT, as we need this signal to detect when we can do our BIOS-magic. The function SCPH102BIOSFooler is ran when the XLAT-pin makes
    //a transistion from high to low, the other code is temporarily paused. Only use this when the code is used on a PAL SCPH-102
    attachInterrupt(0, SCPH102BIOSFooler, FALLING);        //Interrupt 0 is pin 7 on ATTiny45
  }
 
  //Determine on which Playstation we are working by watching the gate-signal; we are running on a PU-22, PU-23 or PSOne when there is a clock signal present
  //We are taking 10 samples of the gate-signal. As we can't really know the frequency of the clock signal that may or may not be present on this signal, we
  //simply take ten samples of the signal and store them in an array. When both 0's and 1's are stored in the array, this means that there is a clock signal
  //on the gate-pin. When all the elements in the array are the same, there is no clock signal available and PsNee is installed in an older model Playstation.
  boolean gateSamples[10];
  int gateSamplesCounter;
  boolean sampleLowDetected = 0;
  boolean sampleHighDetected = 0;
 
  for (gateSamplesCounter = 0; gateSamplesCounter < 10; gateSamplesCounter = gateSamplesCounter + 1)
  {
    gateSamples[gateSamplesCounter] = digitalRead(gate);
  }
 
  for(gateSamplesCounter = 0; gateSamplesCounter < 10; gateSamplesCounter = gateSamplesCounter + 1)
  {
    if (gateSamples[gateSamplesCounter] == 0)
    {
      sampleLowDetected = 1;
    }
    else
    {
      sampleHighDetected = 1;
    }
  }  
 
  if ((sampleLowDetected && sampleHighDetected) == 1)
  {
    flagNewPlaystationModel = 1;
  }
 
 
  delay(1000);            //Wait a second before we're really heading off  
}
 
void loop()
{                                                                                                                                //The bitstreams are reversed (because LSB) and inverted (because the buffer we're injecting our signal after also inverted it's input)
  //VARIABLES                                                                                                                    //      Start            Data     Stop
  FLASH_ARRAY (boolean, SCEEData, 1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0);      //SCEE: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01011101 00   44 bits total
  FLASH_ARRAY (boolean, SCEAData, 1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,1,1,1,1,0,1,0,0);      //SCEA: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01111101 00
  FLASH_ARRAY (boolean, SCEIData, 1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,1,0,1,1,0,1,0,0);      //SCEI: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01101101 00
  int arraycounter;
  int datacounter;
  int slicercounter;
  boolean lidstatus = digitalRead(lid);
 
  if ((flagFirstCycle == 0) || ((flagFirstCycle != 0) && (lidstatus != 0)))       //Only execute the modchip code when the Playstation has just booted or when a new disc needs to be inserted on an already running machine
  {
    if ((flagFirstCycle != 0) && (lidstatus != 0))
    {
      flagBIOSAlreadyFooled == 0;         //When this isn't the first time the modchip does it's thing and the lid has been opened, indicate that the PAL=>NTSC BIOS-patch may be applied again
    }
   
    delay(50);                      //Extra delay to compensate for the state-switching of the CD lid
    if (lidstatus == 0)             //Only do your thing when the lid is closed again
    {
      delay(50);                    //Just wait a second until the coast is clear
      pinMode(gate, OUTPUT);
      digitalWrite(gate, 0);        //Pull to ground to enable data injecting
     
      for (datacounter = 0; datacounter < 31; datacounter = datacounter + 1)        //One cycle of SCEx-string outputting takes approximately 744 ms; we want to go on for about 25 seconds so we output the cycle 30 times
      {
        for (arraycounter = 0; arraycounter < 44; arraycounter = arraycounter + 1)
        {
          if (SCEEData[arraycounter] == 0)
          {
            for (slicercounter = 0; slicercounter < 15; slicercounter = slicercounter + 1)    //This for-loop slices the data signal when it is 0 in small chunks of 130us and do this 14 times; this way, each block of sliced data is about 4ms long, of 250bps
            {
              pinMode(data, OUTPUT);                //We pull the data pin to ground to force a 0
              digitalWrite(data, 0);
              delayMicroseconds(130);
             
              pinMode(data, INPUT);                //We make it high-impedance again to let at least some of the CD tracking signal through
              delayMicroseconds(130);
            }
          }
          else
          {
            pinMode(data, INPUT);                  //We make the data pin high-impedance to let the pull-up of the Playstation motherboard make a 1
            delay(4);
          }
        }
        delay(64);                                //According to the logic analyser the time between two bitstreams now approximates 72 ms on an ATTiny45 with built-in 8MHz oscillator, just like the doctor ordered
       
        for (arraycounter = 0; arraycounter < 44; arraycounter = arraycounter + 1)
        {
          if (SCEAData[arraycounter] == 0)
          {
            for (slicercounter = 0; slicercounter < 15; slicercounter = slicercounter + 1)    //This for-loop slices the data signal when it is 0 in small chunks of 130us; this way, each block of sliced data is about 4ms long, of 250bps
            {
              pinMode(data, OUTPUT);                //We pull the data pin to ground to force a 0
              digitalWrite(data, 0);
              delayMicroseconds(130);
             
              pinMode(data, INPUT);                 //We make it high-impedance again to let at least some of the CD tracking signal through
              delayMicroseconds(130);
            }
          }
          else
          {
            pinMode(data, INPUT);                  //We make the data pin high-impedance to let the pull-up of the Playstation motherboard make a 1
            delay(4);
          }
        }
        delay(64);
       
        for (arraycounter = 0; arraycounter < 44; arraycounter = arraycounter + 1)
        {
          if (SCEIData[arraycounter] == 0)
          {
            for (slicercounter = 0; slicercounter < 15; slicercounter = slicercounter + 1)    //This for-loop slices the data signal when it is 0 in small chunks of 130us; this way, each block of sliced data is about 4ms long, of 250bps
            {
              pinMode(data, OUTPUT);                //We pull the data pin to ground to force a 0
              digitalWrite(data, 0);
              delayMicroseconds(130);
             
              pinMode(data, INPUT);                 //We make it high-impedance again to let at least some of the CD tracking signal through
              delayMicroseconds(130);
            }
          }
          else
          {
            pinMode(data, INPUT);                   //We make the data pin high-impedance to let the pull-up of the Playstation motherboard make a 1
            delay(4);
          }
        }
        delay(64);
      }
      pinMode(data, INPUT);             //Make all outputting pins high-impedance again when we're finished
      pinMode(gate, INPUT);
      flagFirstCycle = 1;               //We completed the initial round of SCEx-outputting; only do this again when a new CD is inserted while the Playstation is turned on!
    }
  }
}
