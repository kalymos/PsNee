# PsNee
                        PPPPPPPPPPPPPPPP                  P            P      
                       P              P                  PP           P        
                      P              P                  P P          P        
                     P              P                  P  P         P          
                    P              P                  P   P        P          
                   P              P                  P    P       P            
                  P              P                  P     P      P            
                 PPPPPPPPPPPPPPPP  PPPPPPPPPPP     P      P     P  PPPPPPPPPPP  PPPPPPPPPPP
                P                 P               P       P    P  P            P
               P                 P               P        P   P  P            P  
              P                 P               P         P  P  P            P  
             P                 P               P          P P  P            P    
            P                 PPPPPPPPPPPPPP  P           PP  PPPPPPP      PPPPPPP    
           P                              P  P            P  P            P      
          P                              P  P            P  P            P      
         P                              P  P            P  P            P        
        P                              P  P            P  P            P        
       P                              P  P            P  P            P      
      P                              P  P            P  P            P        
     P                   PPPPPPPPPPPP  P            P  PPPPPPPPPPP  PPPPPPPPPPP
---------------------------------------
This version is from
http://www.psxdev.net/forum/viewtopic.php?f=47&t=1262&start=40
Is developed by the psxdev team
 
 
-------------------------------------------------

 This PsNee version is meant for Arduino boards.
 
  - Arduino Pro Mini @8Mhz and @16Mhz (supported, tested)
  - Arduino Uno @8Mhz and @16Mhz (supported, tested)
  - Arduino Pro Micro has a different pin assignment and needs some easy porting. (ToDo)
  
  - ATtiny85: Should work the same as ATtiny45 (supported, untested)
  - ATtiny45: LFUSE 0xE2  HFUSE 0xDF > internal oscillator, full 8Mhz speed (supported, tested)
  - ATtiny25: Should work the same as ATtiny45 but doesn't have enough Flash nor RAM for PSNEEDEBUG (supported, untested)
  
 Some extra libraries might be required, depending on the board / chip used.
 PAL PM-41 support isn't implemented yet. (ToDo)
 This code defaults to multi-region, meaning it will unlock PAL, NTSC-U and NTSC-J machines.
 You can optimize boot times for your console further. See "// inject symbols now" in the main loop.
----------------------------------------------------------------------

BIOS patch for PM-41

For now it only supports Arduino boards (ATmega chips).
Also, the Arduino must either be powered on first or have no bootloader present (flashed using SPI) since I expect a signal ~1 second after power on.
8Mhz boards are also supported.
 
-------------------------------------------------------------------

    Choose your hardware!
    You must uncomment the line that corresponds to your cips.
    
 2 main branches available:
  - ATmega based > easy to use, fast and nice features for development
  - ATtiny based > less features, internal clock has 10% variation

 This code is multi-region, meaning it will unlock PAL, NTSC-U and NTSC-J machines.

-----------------------------------------------------

                                Pin assignments

     MultiMode V3                  PSNee psxdev                   PlayStation
    chip pin   name            Arduino pin       name         ps pin         Name

    pin-1  =   3.5v            Arduino pin-vin = 3.5v         3.5v         = supply

    pin-5  = « Gate »          Arduino pin-9   = gate_wfck 9  IC732.Pin-5  = WFCK           
    pin-6  =   data output     Arduino pin-8   = data 8       IC732.Pin-42 = CEO
                               Arduino pin-7   = subq 7       IC304.Pin-24 = SUBQ
                               Arduino pin-6   = sqck 6       IC304.Pin-26 = SQCK
                               Arduino pin-5   = D2           IC102.Pin-15 = D2
                               Arduino pin-4   = A18          IC102.Pin-31 = A18
    pin-8  =   gnd             Arduino Pin-Gnd = gnd          GND          = gnd
------------------------------------------------
                 General info!
--------------------------------------------------
PLAYSTATION 1 SECURITY - HOW IT DOES IT'S THING:
Sony didn't really go through great lenghts to protect it's precious Playstation
from running unauthorised software; the main security is based on a simple ASCII
string of text that is read from a part of an original Playstation disc that cannot
be reproduced by an ordinary PC CD burner.
As most of you will know, a CD is basically a very long rolled up (carrier) string in which very
little pits and ehm... little not-pits are embedded that represent the data stored on the disc.
The nifty Sony engineers did not use the pits and stuff to store the security checks for
Playstation discs but went crazy with the rolled up carrier string. In an ordinary CD, the
string is rolled up so that the spacing between the tracks is as equal as possible. If that
is not the case, the laser itself needs to move a bit to keep track of the track and
reliably read the data off the disc.
If you wonder how the laser knows when it follows the track optimally: four photodiodes, light
intensity measurement, difference measurements, servo. There.
To the point: the Sony engineers decidedly "fumbled up" the track of sector 4 on a Playstation
disc (the track was modulated in nerd-speak) so that the error correction circuit outputs a
recognisable signal, as the laser needs to be corrected to follow the track optimally.
This output signal actually is a 250bps serial bitstream (with 1 start bit and 2 stop bits) which
in plain ASCII says SCEA (Sony Computer Entertainment of America), SCEE (Sony Computer Entertainment
of Europe) or SCEI (Sony Computer Entertainment of Japan), depending on the region of the disc inserted.
The security thus functions not only as copy protection, but also as region protection.
The text string from the disc is compared with the text string that is embedded in the Playstation
hardware. When these text strings are the same, the disc is interpreted to be authentic and from
the correct region. Bingo!


 
