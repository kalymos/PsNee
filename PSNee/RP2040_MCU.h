#pragma once

#include <pico/stdlib.h>
#include <stdint.h>

void start_rp2040()
{
    //Enabling this line will reproduce 16MHz as the Arduino (in theory), but the serial will be disabled.
    //set_sys_clock_pll(882000000, 7, 7); //16MHz clock
    const uint32_t gpio_pins_mask = (0xf << 4) | (1U << 25);
    gpio_init_mask(gpio_pins_mask);
}

void _delay_ms(double ms)
{
    sleep_ms(ms);
}

void _delay_us(double us)
{
    sleep_us(us);
}