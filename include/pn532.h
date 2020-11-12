/**
 *  @file pn532.h
 * ---------------------
 * @brief Interfaces with pn532 using SPI interface (Pins 7-10)
 */

#include "spi.h"

/**
 * @fn pn532_init
 * ---------------------
 * Initializes spi interface and resets PN532 module.
 */
void pn532_init();