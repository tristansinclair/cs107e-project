/**
 *  @file test_nfc.c
 * ---------------------
 * @brief Tests nfc and pn532 capabilities.
 */

#include <uart.h>
#include <spi.h>
#include <gpio.h>
#include <pn532.h>

const unsigned int RESET_PIN = GPIO_PIN20;
const unsigned int NSS_PIN = GPIO_PIN21;

void test_spi_transfer()
{
    char *frame[20];
    memcpy(frame, 0, 20);
}

void main(void)
{
    pn532_init();
    test_spi_transfer();
    uart_putchar(EOT);
}