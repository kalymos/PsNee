# PsNee

PsNee, an open source stealth modchip for the Sony Playstation 1 

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

  - Pro Micro @8Mhz and @16Mhz (supported, tested)
  - Arduino Leonardo @8Mhz and @16Mhz: Should work the same as Pro Micro, but two pins are on the ICSP header (supported, untested)
  
  - ATtiny85: Should work the same as ATtiny45 (supported, untested)
  - ATtiny45: LFUSE 0xE2  HFUSE 0xDF > internal oscillator, full 8Mhz speed (supported, tested)
  - ATtiny25: Should work the same as ATtiny45 but doesn't have enough Flash nor RAM for PSNEEDEBUG (supported, untested)
  
---------------------------------------------------------------------------

Be sure to use the 3.5V / 3.3V PSX power supply, * AND NOT THE 5V! * for your PsNee. (The ps chipset works in 3.3v, and their injected 5v is not good.) The installation pictures include an example.

-----------------------------------------------------------------------------

Some extra libraries might be required, depending on the board / chip used.
This code defaults to multi-region, meaning it will unlock PAL, NTSC-U and NTSC-J machines.
You can optimize boot times for your console further. See "// inject symbols now" in the main loop.

----------------------------------------------------------------------

BIOS patch for PM-41

For now it only supports Arduino boards (ATmega chips).
Also, the Arduino must either be powered on first or have no bootloader present (flashed using SPI) since I expect a signal ~1 second after power on.
8Mhz boards are also supported.
 
-------------------------------------------------------------------

    Choose your hardware!
    You must uncomment the line that corresponds to your chips.
    
 2 main branches are available:
  - ATmega based: easy to use, fast and nice features for development
  - ATtiny based: fewer features, internal clock has 10% variation

 This code is multi-region, meaning it will unlock PAL, NTSC-U and NTSC-J machines.

-------------------------------------------------------------------------------------

                                         pin equivalent tableau
                        
                      PSNee            name in Arduino board or Attiny                             PlayStation
		      
    board name					Uno		     Leonardo
    						Nano                 Micro 
    						Mini		     Pro Micro
						        Pro Mini
    
    name in code                       ATTINY_X5    ARDUINO_328_BOARD    ARDUINO_32UX_BOARD        
    
    microcontroller		  	   ATtiny*5	ATmega328**          ATmega32U*
						   					               ic pin name     track name in schematic
                                       VCC          5V                   5v                        3.5V            supply
                      debugtx          3
                      gate_wfck        4            9                    9                         IC732.Pin-5     WFCK
                      data             2            8                    8                         IC732.Pin-42    CEO
     pin name         subq             1            7                    6                         IC304.Pin-24    SUBQ
                      sqck             0            6                    4                         IC304.Pin-26    SQCK
                      BIOS D2                       5                    3                         IC102.Pin-15    D2 
                      BIOS A18                      4                    2                         Ic102.Pin-31    A18
	                                   GND          GND                  GND            	       GND             GND













## General Info

### The PlayStation 1 security, how does it work?

To protect its precious PlayStation from running unauthorized software, Sony implemented a copy protection mechanism based on a simple ASCII string of text that is read from a part of an original PlayStation disc that cannot be reproduced by an ordinary PC CD burner (in theory).

Basically, a CD is made from a really long spiral of pits and lands that represent the data stored on the disc. Inside the Optical Pickup Unit (OPU), a laser diode emits infrared light that goes through lenses and mirrors, hits the disc, bounces back and shines on a sensor made from at least 4 photodiodes. These photodiodes receive different amounts of light depending on pits and lands' reflectivity allowing to recover the data but they are also used to perform self-adjustment to keep the track aligned and in focus.
This is done by comparing photodiodes outputs in a way to create the Tracking Error (TE) and Focus Error (FE) signals and then send them to the CD controller to move the lens head horizontally for the tracking and vertically for the focus.

On a CD-ROM (read-only disc pressed inside a CD factory), the spiral is a simple rolled up line. However, on a blank CD-R, the spiral i.e. the pregroove is wobbling on all over the disc and the CD controller follows it. This wobble picked up by the TE signal actually carries data known as the Absolute Time In Pregroove (ATIP) and is frequency-modulated at 22kHz (1x read speed). The ATIP contains information about the disc characteristics and time codes but is useful only for burning.

Now, what is the relation between these explanations and the PlayStation security?

As CD-ROMs do not have the wobble nor the ATIP, Sony engineers surely thought:

    Why not reuse the wobble that burners can't reproduce to encode some sort of information that will provide copy protection?

And that's what they did! In the lead-in area of a PlayStation disc (negative sectors), the spiral is wobbling at the same 22kHz carrier frequency like a CD-R but instead of storing the ATIP, they stored a 250bps amplitude-modulated serial bitstream also known as SCEx signal (with 1 start bit and 2 stop bits) that repeats until sector 0 (end of lead-in).

The important thing to understand is the bitstream is not part of the game data which is encoded using pits and lands' length. Instead, it is made by slightly shifting the track i.e. pits and lands' position in a sinusoidal way. The self-adjusting system simply follows this wobbling track creating a modulated TE signal that needs to be demodulated using a 22kHz filter. Then, it goes to the PlayStation sub-CPU for decoding.

The bitstream in plain ASCII says "SCEA" (Sony Computer Entertainment of America), "SCEE" (Sony Computer Entertainment of Europe) or "SCEI" (Sony Computer Entertainment Inc. (i.e. Japan)), depending on the region of the disc.
The security thus functions not only as copy protection, but also as region protection.

The text string from the disc is compared with the one that is embedded in the PlayStation sub-CPU. If these text strings are the same, the disc is interpreted to be authentic and from the correct region. Bingo!

### The master branch is completely redesigned!

The original code doesn't have a mechanism to turn the injections off. It bases everything on a timer.
After power on, it will start sending injections for some time, then turns off.
It also doesn't know when it's required to turn on again (except for after a reset), so it gets detected by anti-mod games.

### The mechanism to know when to inject and when to turn it off.

This is the 2 wires for SUBQ / SQCK. The PSX transmits the current subchannel Q data on this bus. It tells the console where on the disc the read head is. We know that the protection symbols only exist on the earliest sectors, and that anti-mod games exploit this by looking for the symbols elsewhere on the disk. If they get those symbols, a modchip must be generating them!

So with that information, my code knows when the PSX wants to see the unlock symbols, and when it's "fake" / anti-mod. The chip is continously looking at that subcode bus, so you don't need the reset wire or any other timing hints that other modchips use. That makes it compatible and fully functional with all revisions of the PSX, not just the later ones. Also with this method, the chip knows more about the current CD. This allows it to not send unlock symbols for a music CD, which means the BIOS starts right into the CD player, instead of after a long delay with other modchips.

This has some drawbacks, though:
 * It's more logic / code. More things to go wrong. The testing done so far suggests it's working fine though.
 * It's not a good example anymore to demonstrate PSX security, and how modchips work in general.
