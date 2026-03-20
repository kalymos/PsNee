# PSNee V9.0
THE modechip supports the largest number of Playstation 1 variants, and the largest number of microcontrollers
# For all useful information consult the ![Wiki](https://github.com/kalymos/PsNee/wiki)

![Logo](/images/PSNee-9.0-logo.png)

## Features
- Remove Disk Region Protection
- Patch BIOS 
- A specific library for card support, to solve the fuse setting problem.
- The mode does not take care of changing PAL <-> NTSC video output (in other words if you use a Japanese console and you put European games, or in older European models you use American or Japanese games... the display will not be correct)

## Supported Playstation 1
All US models, all European models, and the Japanese models.

## Supported platforms
- ATmega328(A/P/PA) @16Mhz  
- ATmega168(A/P/PA) @16Mhz
- Atmega32U4        @16Mhz
- ATtiny25/45/85    @8Mhz no BIOS patch!
> [!CAUTION]
> **BIOS Patching:** Timing for PB-series (328PB/128PB) is currently unoptimized.  
> For stable BIOS patching, use A/P/PA or 32U4 variants.


## Model that I personally tested
![test](images/test-mat.png)
Example of gray image  
![gray](https://github.com/kalymos/PsNee/blob/master/images/issue/gray-screens.png)
