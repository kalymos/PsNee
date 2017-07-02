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
 16Mhz and 8Mhz variants are supported. "Pro Micro" etc supported and recommended
 "Arduino Pro Micro" has a different pin assignment and needs porting. (ToDo)

 PAL PM-41 support isn't implemented yet. (ToDo)

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
