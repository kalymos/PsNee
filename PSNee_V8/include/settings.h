#pragma once

#ifdef UC_ALL
    const char region[3] = {'a', 'a', 'a'};
#endif

#ifdef PAL_FAT
    const char region[3] = {'e', 'e', 'e'};
#endif

#ifdef SCPH_103
    const char region[3] = {'i', 'i', 'i'};
#endif

#ifdef SCPH_102
#define BIOS_PATCH
#define HOLD _delay_us(2.75)
#define PATCHING _delay_us(0.2)
#define CHECKPOINT 83900
#define TRIGGER 48
    const char region[3] = {'e', 'e', 'e'};
#endif

#ifdef SCPH_100
#define BIOS_PATCH
#define HOLD _delay_us(2.7)
#define PATCHING _delay_us(0.2)
#define CHECKPOINT 83900
#define TRIGGER 48
    const char region[3] = {'i', 'i', 'i'};
#endif

#ifdef SCPH_7000_9000
#define BIOS_PATCH
#define HOLD _delay_us(2.85) 
#define PATCHING _delay_us(0.1)
#define CHECKPOINT 75270
#define TRIGGER 16
    const char region[3] = {'i', 'i', 'i'};
#endif

#ifdef SCPH_5500
#define BIOS_PATCH
#define LOW_TRIGGER
#define HOLD _delay_us(2.85)
#define PATCHING _delay_us(0.1)
#define CHECKPOINT 76130
#define TRIGGER 21
    const char region[3] = {'i', 'i', 'i'};
#endif

#ifdef SCPH_3500_5000
#define BIOS_PATCH
#define LOW_TRIGGER
#define HOLD _delay_us(2.85)
#define PATCHING _delay_us(0.1)
#define CHECKPOINT 75260
#define TRIGGER 21
    const char region[3] = {'i', 'i', 'i'};
#endif

#ifdef SCPH_3000
#define BIOS_PATCH
#define HIGH_PATCH
#define DOUBLE_PATCH
#define CHECKPOINT 83000
#define TRIGGER 60
#define HOLD _delay_us(2.75)
#define PATCHING _delay_us(0.1)
#define LOW_TRIGGER2
#define CHECKPOINT2 253300
#define TRIGGER2 43
#define HOLD2 _delay_us(2.88)
#define PATCHING2 _delay_us(0.15)
    const char region[3] = {'i', 'i', 'i'};
#endif

#ifdef SCPH_1000
#define BIOS_PATCH
#define HIGH_PATCH
#define DOUBLE_PATCH
#define CHECKPOINT 83000
#define TRIGGER 92
#define HOLD _delay_us(2.7)
#define PATCHING _delay_us(0.1)
#define LOW_TRIGGER2
#define CHECKPOINT2 272800
#define TRIGGER2 71
#define HOLD2 _delay_us(2.88)
#define PATCHING2 _delay_us(0.15)
    const char region[3] = {'i', 'i', 'i'};
#endif
