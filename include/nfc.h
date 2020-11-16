/**
 *  @file nfc.h
 * ---------------------
 * @brief API for reading and writing nfc tags.
 */
#ifndef _NFC_H
#define _NFC_H

#include "pn532.h"

/**
 * @fn nfc_init
 * ---------------------
 * Initializes nfc capabilities.
 */
void nfc_init(unsigned int reset_pin, unsigned int nss_pin);

#endif // _NFC_H