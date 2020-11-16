/**
 *  @file nfc.c
 * ---------------------
 * @brief Implements nfc.h
 */

#include <nfc.h>

void nfc_init(unsigned int reset_pin, unsigned int nss_pin)
{
    pn532_init(reset_pin, nss_pin);
}