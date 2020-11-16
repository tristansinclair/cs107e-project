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

unsigned int reset_pin = GPIO_PIN20;
unsigned int nss_pin = GPIO_PIN21;

int GetFirmwareVersion(void)
{
    // length of version: 4
    if (PN532_CallFunction(pn532, PN532_COMMAND_GETFIRMWAREVERSION,
                           version, 4, NULL, 0, 500) == PN532_STATUS_ERROR)
    {
        pn532->log("Failed to detect the PN532");
        return PN532_STATUS_ERROR;
    }
    return PN532_STATUS_OK;
}

void basic_tests(void)
{
    printf("NFC Firmware Testing\n");

    uint8_t buff[255];
    buff[0] = 0xD4;
    buff[1] = command & 0xFF;

    pn532_reset();
    pn532_wakeup();
    spi_transfer();

    // if (PN532_WriteFrame(pn532, buff, params_length + 2) != PN532_STATUS_OK)
    // {
    //     pn532->wakeup();
    //     pn532->log("Trying to wakeup");
    //     return PN532_STATUS_ERROR;
    // }

    if (PN532_GetFirmwareVersion(&pn532, buff) == PN532_STATUS_OK)
    {
        printf("Found PN532 with firmware version: %d.%d\r\n", buff[1], buff[2]);
    }
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