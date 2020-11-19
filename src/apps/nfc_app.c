/**
 * @file nfc_app.c
 * ---------------------
 * @brief Demo application for displaying nfc capabilities.
 */

#include <uart.h>
#include <shell.h>
#include <printf.h>
#include <gpio.h>
#include <keyboard.h>
#include <interrupts.h>
#include <gpio_interrupts.h>

void main(void)
{
    uart_init();
    interrupts_init();
    interrupts_global_enable(); // everything fully initialized, now turn on interrupts
    gpio_interrupts_enable();
    keyboard_init(GPIO_PIN5, GPIO_PIN6);
    shell_init(printf);

    shell_run();
    uart_putchar(EOT);
}