/**
 *  @file test_nfc.c
 * ---------------------
 * @brief Tests nfc and pn532 capabilities.
 */

#include <uart.h>
<<<<<<< HEAD
#include <spi.h>
#include <gpio.h>
#include <pn532.h>

const unsigned int RESET_PIN = GPIO_PIN20;
const unsigned int NSS_PIN = GPIO_PIN21;

void test_spi_transfer()
{
    char *frame[20];
    memcpy(frame, 0, 20);
=======
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
>>>>>>> 8e9316de716064c7fc9bb3cc337a372dad4ed882
}

void main(void)
{
<<<<<<< HEAD
    pn532_init();
    test_spi_transfer();
=======
    uart_init();
    timer_init();

    printf("Testing NFC\n");
    basic_tests();

>>>>>>> 8e9316de716064c7fc9bb3cc337a372dad4ed882
    uart_putchar(EOT);
}