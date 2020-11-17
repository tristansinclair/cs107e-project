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

/**
 * @fn print_bytes
 * ---------------------
 * Prints buffer of bytes in a nice readable format.
 */
void print_bytes(byte_t *buf, size_t bufsize);

#endif // _NFC_H