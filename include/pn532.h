/**
 *  @file pn532.h
 * ---------------------
 * @brief Interfaces with pn532 using SPI interface (Pins 7-10)
 */

#ifndef _PN532_H
#define _PN532_H

#include <spi.h>

typedef unsigned int size_t;

/**
 * @fn pn532_init
 * ---------------------
 * Initializes spi interface and resets PN532 module.
 */
void pn532_init();

/**
 * @fn pn532_read_data
 * ---------------------
 * Sends read signal to pn532 and writes response into data buffer.
 */
void pn532_read_data(int *data, size_t bufsize);

/**
 * @fn pn532_read_data
 * ---------------------
 * Writes data data to spi module.
 */
void pn532_write_data(int *data, size_t bufsize);

#endif // _PN532_H