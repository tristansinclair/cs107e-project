/**
 *  @file pn532.h
 * ---------------------
 * @brief Interfaces with pn532 using SPI interface (Pins 7-10)
 */

#ifndef _PN532_H
#define _PN532_H

#include <spi.h>
#include <strings.h>

// Constants
typedef unsigned int size_t;
#define _SPI_STATREAD (0x02)
#define _SPI_DATAWRITE (0x01)
#define _SPI_DATAREAD (0x03)
#define _SPI_READY (0x01)
#define _SPI_CHANNEL (0)

/**
 * @fn pn532_init
 * ---------------------
 * Initializes spi interface and resets PN532 module.
 */
void pn532_init();

/**
 * @fn pn532_reset
 * ---------------------
 * Resets pn532 module.
 */
void pn532_reset();

/**
 * @fn pn532_wakeup
 * ---------------------
 * Wakes pn532 module.
 */
void pn532_wakeup();

/**
 * @fn pn532_read_data
 * ---------------------
 * Sends read signal to pn532 and writes response into data buffer.
 */
void pn532_read_data(char *data, size_t bufsize);

/**
 * @fn pn532_read_data
 * ---------------------
 * Writes data data to spi module.
 */
void pn532_write_data(char *data, size_t bufsize);

#endif // _PN532_H