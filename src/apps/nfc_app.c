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
#include <nfc.h>

static const unsigned int RESET_PIN = GPIO_PIN20;
static const unsigned int NSS_PIN = GPIO_PIN4;

void main(void)
{
    uart_init();
    interrupts_init();
    gpio_interrupts_init();
    interrupts_global_enable(); // everything fully initialized, now turn on interrupts
    keyboard_init(GPIO_PIN5, GPIO_PIN6);
    shell_init(printf);
    nfc_init(RESET_PIN, NSS_PIN);

    shell_run();

    uart_putchar(EOT);
}