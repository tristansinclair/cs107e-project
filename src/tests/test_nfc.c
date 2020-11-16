/**
 *  @file test_nfc.c
 * ---------------------
 * @brief Tests nfc and pn532 capabilities.
 */

#include <uart.h>
#include <pn532.h>
#include <assert.h>
#include <console.h>
#include <fb.h>
#include <gl.h>
#include <timer.h>
#include "printf.h"
#include "pn532.h"

void basic_tests(void)
{
    console_init(40, 40);
    console_printf("\n----------------------------------------\n");
    console_printf("|           NFC Basic Testing          |\n");
    console_printf("|   CONSOLE SIZE: 40 x 40 (in chars)   |\n");
    console_printf("----------------------------------------\n");
}

void main(void)
{
    uart_init();
    timer_init();

    printf("Testing NFC\n");
    basic_tests();

    uart_putchar(EOT);
}