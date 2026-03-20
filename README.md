# PSNee V9.0
This modchip supports the widest range of PlayStation 1 models and the largest variety of microcontrollers.

For all useful information, please consult the [Wiki](https://github.com/kalymos/PsNee/wiki).

![Logo](/images/PSNee-9.0-logo.png)

## Features
- Removes Disc Region Protection.
- BIOS Patching.
- Specific library for board support to solve fuse setting issues.
> [!IMPORTANT]
> **Video Signal Note:** This modchip does not handle PAL/NTSC video signal conversion. Playing games from a different region may result in an incorrect display (e.g., NTSC games on early PAL consoles, or PAL games on NTSC-U/C or NTSC-J consoles).
>
> *Note: European models from SCPH-7002 onwards typically handle NTSC signals correctly without display issues, but SCPH-102 requires a BIOS patch.*
>
> See the **NTSC/PAL Display Mismatch** example at the bottom of the page.


## Supported PlayStation 1
All models are supported.

## Supported Platforms
- ATmega328(A/P/PA) @16MHz  
- ATmega168(A/P/PA) @16MHz
- ATmega32U4        @16MHz
- ATtiny25/45/85    @8MHz (Note: No BIOS patch support!)

> [!CAUTION]
> **BIOS Patching:** PB-series (328PB/128PB) is currently unoptimized.  
> For stable BIOS patching, please use A/P/PA or 32U4 variants.

### NTSC/PAL Display Mismatch

Typical result when playing a game from a different region without a signal converter.
![gray](https://github.com/kalymos/PsNee/blob/master/images/issue/gray-screens.png)

### Verified Hardware (Personal Test Collection)
![test](images/test-mat.png)
From left to right: NTSC-J, PAL, and NTSC-U/C & Asia models.
