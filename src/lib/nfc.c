/**
 *  @file nfc.c
 * ---------------------
 * @brief Implements nfc.h
 */

#include <nfc.h>

void nfc_init()
{
    pn532_init();
}