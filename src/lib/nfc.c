/**
 * @file nfc.c
 * ---------------------
 * @brief Implements nfc.h
 */

#include <nfc.h>

void nfc_init(unsigned int reset_pin, unsigned int nss_pin)
{
    pn532_init(reset_pin, nss_pin);
}

void print_bytes(byte_t *buf, size_t bufsize)
{
    // Print vertical line numbers
    printf("\n     ");
    for (int i = 0; i < 16; i++)
    {
        if (i > 9)
            printf("%d ", i);
        else
            printf(" %d ", i);
    }
    printf("\n");

    for (int i = 0; i < bufsize; i++)
    {
        if (i % 16 == 0)
            printf("\n%02d : ", i % 16);
        printf("%02x ", buf[i]);
    }
    printf("\n");
}