/**
 *  @file test_nfc.c
 * ---------------------
 * @brief Tests nfc and pn532 capabilities.
 */

#include <uart.h>
#include <spi.h>
#include <gpio.h>
#include <pn532.h>
#include <timer.h>
#include <printf.h>

const unsigned int RESET_PIN = GPIO_PIN20;
const unsigned int NSS_PIN = GPIO_PIN21;

void test_spi_transfer()
{
    byte_t frame[7] = {0x00, 0x00, 0xff, 0x02, 0xe0, 0x2a, 0x00};
    pn532_write_data(frame, 7);

    byte_t read_data[262];
    memset(read_data, 0, 262);
    pn532_read_data(read_data, 262);

    printf("\n{");
    for (int i = 0; i < 262; i++)
    {
        printf("%x, ", read_data[i]);
        if (i % 10 == 0)
            printf("\n");
    }
    printf("}\n");
}

void basic_tests(void)
{
    printf("\n----------------------------------------\n");
    printf("|           NFC Basic Testing          |\n");
    printf("|   CONSOLE SIZE: 40 x 40 (in chars)   |\n");
    printf("----------------------------------------\n");
}

void main(void)
{
    pn532_init(RESET_PIN, NSS_PIN);
    test_spi_transfer();
    uart_putchar(EOT);
}