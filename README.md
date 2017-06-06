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
 
 ------------------------------------------------
-This version is from the forum 
http://www.emu-land.net/forum/index.php?topic=76537.0
And is developed by postal2201
 -------------------------------------------------

PsNee, an open source stealth modchip for the Sony Playstation 1, usable on
all platforms supported by Arduino, preferably ATTiny. Finally something modern!
 
 
--------------------------------------------------
                    TL;DR
--------------------------------------------------
Look for the "Arduino selection!" section and verify the target platform. Hook up your target device and hit Upload!
BEWARE: when using ATTiny45, make sure the proper device is selected (Extra=>Board=>ATTiny45 (internal 8MHz clock))
and the proper fuses are burnt (use Extra=>Burn bootloader for this), otherwise PsNee will malfunction. A tutorial on
uploading Arduino code via an Arduino Uno to an ATTiny device: http://highlowtech.org/?p=1695
Look at the pinout for your device and hook PsNee up to the points on your Playstation.
 
 
--------------------------------------------------
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
 
HOW THE MODCHIP TRICKS THE PLAYSTATION:
The modchip isn't all that of a complicated device: clever reverse engineers found the point on the
Playstation motherboard that carried the text string from the disc and found a way to temporarily block
this signal (by grounding an input of an op-amp buffer) to be able to inject the signal from the modchip
The modchip injects after about 1500ms the text strings SCEE SCEA SCEI on the motherboard point and stops
with this after about 25 seconds. Because all the possible valid region options are outputted on the
motherboard the Playstation gets a bit confused and simply accepts the inserted disc as authentic; after all,
one of the codes was the same as that of the Playstation hardware...
Early modchips applied the text strings as long as power was applied to them, whereby later Playstation
software could detect whether a modchip was installed. This is circumvented in this application by idling the
modchip after about 25 seconds. The text strings are only tranmitted again when the CD lid is opened and closed
again, to enable playing multi-disc games. This is also called a stealth modchip in marketing-speak.
