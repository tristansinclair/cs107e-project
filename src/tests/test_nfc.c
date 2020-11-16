/**
 *  @file test_nfc.c
 * ---------------------
 * @brief Tests nfc and pn532 capabilities.
 */

#include <uart.h>
#include <pn532.h>
#include <assert.h>
#include <gpio.h>
#include <timer.h>
#include "printf.h"
#include "pn532.h"

const unsigned int RESET_PIN = GPIO_PIN20;
const unsigned int NSS_PIN = GPIO_PIN4;

byte_t reverse_bit(byte_t num)
{
    byte_t result = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        result <<= 1;
        result += (num & 1);
        num >>= 1;
    }
    return result;
}

int GetFirmwareVersion(void)
{
    byte_t frame[7] = {0x00, 0x00, 0xff, 0x02, 0xe0, 0x2a, 0x00};
    printf("\n{");

    for (int i = 0; i < 7; i++)
    {
        frame[i] = reverse_bit(frame[i]);
    }

    for (int i = 0; i < 7; i++)
    {
        printf("%x, ", frame[i]);
        if (i % 10 == 0)
            printf("\n");
    }

    gpio_write(NSS_PIN, 0);
    spi_transfer(frame, frame, 7);
    gpio_write(NSS_PIN, 1);

    // byte_t read_data[262];
    // memset(read_data, 0, 262);
    // pn532_read_data(read_data, 262);

    printf("\n{");
    for (int i = 0; i < 7; i++)
    {
        printf("%x, ", frame[i]);
        if (i % 10 == 0)
            printf("\n");
    }
    return PN532_STATUS_OK;
}

void basic_tests(void)
{
    // printf("\n----------------------------------------\n");
    // printf("|           NFC Basic Testing          |\n");
    // printf("|   CONSOLE SIZE: 40 x 40 (in chars)   |\n");
    // printf("----------------------------------------\n");
}

void main(void)
{
    uart_init();
    timer_init();
    pn532_init(reset_pin, nss_pin);

    //printf("Testing NFC\n");

    basic_tests();

    uart_putchar(EOT);
}