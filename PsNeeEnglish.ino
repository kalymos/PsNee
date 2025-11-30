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
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//
//PsNee, an open source stealth modchip for the Sony Playstation 1, usable on
//all platforms supported by Arduino, preferably ATTiny. Finally something modern!
//
/////////// TO DO: /////////////
// - Find out how the NTSC BIOS patch for SCPH-102 works and integrate it in this sketch
//   (supposedly it forces pin 15 (data 2) on the BIOS chip (IC102) low when something happens on pin 31 (address 18) according to http://problemkaputt.de/psx-spx.htm#cdromprotectionmodchips)
// - Make SCEx-arrays smaller by commoning up the common parts of the arrays and thus use less flash
// - Common up the two big for-loops with an OR-statement
////////////////////////////////
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
//Kindly coded and documented by TheFrietMan, August 20 2015, The Netherlands.
//The Playstation is great but nothing beats our national pride, the Philips CDi! Cheesiness for the win!
//
//PINOUT IC:
//  ATTiny45:
//    Pin 1: Not connected
//    Pin 2: Not connected
//    Pin 3: Not connected
//    Pin 4: Ground
//    Pin 5: OUT - Data
//    Pin 6: OUT - Gate
//    Pin 7: IN - CD lid
//    Pin 8: Vcc
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Flash.h>     //Include the Flash library to conveniently store the SCExData-arrays in PROGMEM, see http://arduiniana.org/libraries/flash/

//BLOODY GLOBAL VARIABLES
//Pins for the Arduino Uno
//int data = 5;        //The pin that outputs the SCEE SCEA SCEI string
//int gate = 6;        //The pin that gets pulled low to enable data injection on the Playstation motherboard
//int lid = 7;         //The pin that gets connected to the internal CD lid signal; active high

//Pins for the ATTiny45 Arduino
int data = 0;
int gate = 1;
int lid = 2;
boolean flagFirstCycle = 0;    //This flag is 1 when the system completes the first cycle of SCEx-string outputting; this is done to stealth the chip up

void setup()
{ 
  pinMode(data, INPUT);    //The pins are high-impedance when configured as inputs so they don't interfere with the Playstation mortherboard circuitry
  pinMode(gate, INPUT);    
  pinMode(lid, INPUT);
    
  delay(1200);            //Wait a second before we're really heading off  
}

void loop()
{                                                                                                                                //The bitstreams are reversed (because LSB) and inverted (because the buffer we're injecting our signal after also inverted it's input)
  //VARIABLES                                                                                                                    //      Start            Data     Stop
  FLASH_ARRAY (boolean, SCEEData, 1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0);      //SCEE: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01011101 00   44 bits total
  FLASH_ARRAY (boolean, SCEAData, 1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,1,1,1,1,0,1,0,0);      //SCEA: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01111101 00
  FLASH_ARRAY (boolean, SCEIData, 1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,1,0,1,1,0,1,0,0);      //SCEI: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01101101 00
  int arraycounter;
  int datacounter;
  boolean lidstatus = digitalRead(lid);
  
  if (flagFirstCycle == 0)
  {
    pinMode(gate, OUTPUT);
    digitalWrite(gate, 0);          //Pull to ground to enable data injecting
   
    for (datacounter = 0; datacounter < 31; datacounter = datacounter + 1)        //One cycle of SCEx-string outputting takes approximately 744 ms; we want to go on for about 25 seconds so we output the cycle 30 times
    {
      for (arraycounter = 0; arraycounter < 44; arraycounter = arraycounter + 1)
      {
        if (SCEEData[arraycounter] == 0)
        {
          pinMode(data, OUTPUT);                //We pull the data pin to ground to force a 0
          digitalWrite(data, 0);
          delay(4);                             //Send the signal for 4 ms
        }
        else
        {
          pinMode(data, INPUT);                //We make the data pin high-impedance to let the pull-up of the Playstation motherboard make a 1
          delay(4);
        }
      }
      delay(64);        //According to the logic analyser the time between two bitstreams now approximates 72 ms on an ATTiny45 with built-in 8MHz oscillator, just like the doctor ordered
      
      for (arraycounter = 0; arraycounter < 44; arraycounter = arraycounter + 1)
      {
        if (SCEAData[arraycounter] == 0)
        {
          pinMode(data, OUTPUT);                //We pull the data pin to ground to force a 0
          digitalWrite(data, 0);
          delay(4);
        }
        else
        {
          pinMode(data, INPUT);                //We make the data pin high-impedance to let the pull-up of the Playstation motherboard make a 1
          delay(4);
        }
      }
      delay(64);
      
      for (arraycounter = 0; arraycounter < 44; arraycounter = arraycounter + 1)
      {
        if (SCEIData[arraycounter] == 0)
        {
          pinMode(data, OUTPUT);                //We pull the data pin to ground to force a 0
          digitalWrite(data, 0);
          delay(4);
        }
        else
        {
          pinMode(data, INPUT);                //We make the data pin high-impedance to let the pull-up of the Playstation motherboard make a 1
          delay(4);
        }
      }
      delay(64);
    }
    pinMode(data, INPUT);    //Make all outputting pins high-impedance again when we're finished
    pinMode(gate, INPUT);
    flagFirstCycle = 1;      //We completed the initial round of SCEx-outputting; only do this again when a new CD is inserted while the Playstation is turned on!
  }
  
  //Only force authentication when this isn't the first cycle and the lid has been opened and closed again
  if ((flagFirstCycle != 0) && (lidstatus != 0))
  {
    delay(50);                      //Extra delay to compensate for the state-switching of the CD lid
    if (lidstatus == 0)            //Only do your thing when the lid is closed again
    {
      delay(100);                     //Just wait a second until the coast is clear
      pinMode(gate, OUTPUT);
      digitalWrite(gate, 0);          //Pull to ground to enable data injecting
   
      for (datacounter = 0; datacounter < 31; datacounter = datacounter + 1)        //One cycle of SCEx-string outputting takes approximately 744 ms; we want to go on for about 25 seconds so we output the cycle 30 times
      {
        for (arraycounter = 0; arraycounter < 44; arraycounter = arraycounter + 1)
        {
          if (SCEEData[arraycounter] == 0)
          {
            pinMode(data, OUTPUT);                //We pull the data pin to ground to force a 0d
            digitalWrite(data, 0);
            delay(4);
          }
          else
          {
            pinMode(data, INPUT);                //We make the data pin high-impedance to let the pull-up of the Playstation motherboard make a 1
            delay(4);
          }
        }
        delay(64);        //Volgens de logic analyser is de tijd tussen de twee bitstreams nu precies 72 ms op een ATTiny45 met ingebouwde 8MHz oscillator, zoals het zou moeten zijn
        
        for (arraycounter = 0; arraycounter < 44; arraycounter = arraycounter + 1)
        {
          if (SCEAData[arraycounter] == 0)
          {
            pinMode(data, OUTPUT);                //We pull the data pin to ground to force a 0
            digitalWrite(data, 0);
            delay(4);
          }
          else
          {
            pinMode(data, INPUT);                //We make the data pin high-impedance to let the pull-up of the Playstation motherboard make a 1
            delay(4);
          }
        }
        delay(64);
        
        for (arraycounter = 0; arraycounter < 44; arraycounter = arraycounter + 1)
        {
          if (SCEIData[arraycounter] == 0)
          {
            pinMode(data, OUTPUT);                //We pull the data pin to ground to force a 0
            digitalWrite(data, 0);
            delay(4);
          }
          else
          {
            pinMode(data, INPUT);                //We make the data pin high-impedance to let the pull-up of the Playstation motherboard make a 1
            delay(4);
          }
        }
        delay(64);
      }
    pinMode(data, INPUT);    //Make all outputting pins high-impedance again when we're finished
    pinMode(gate, INPUT);
    }
  }
}
