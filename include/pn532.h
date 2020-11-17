/**
 *  @file pn532.h
 * ---------------------
 * @brief Interfaces with pn532 using SPI interface (Pins 7-10)
 */

#ifndef _PN532_H
#define _PN532_H

#include <spi.h>
#include <strings.h>
#include <gpio.h>
#include <timer.h>
#include <stdbool.h>
#include <printf.h>

// SPI Constants
#define _SPI_STATREAD (0x02)
#define _SPI_DATAWRITE (0x01)
#define _SPI_DATAREAD (0x03)
#define _SPI_READY (0x01)
#define _SPI_CHANNEL (0)

// Communication bytes
#define PN532_PREAMBLE (0x00)
#define PN532_STARTCODE1 (0x00)
#define PN532_STARTCODE2 (0xFF)
#define PN532_POSTAMBLE (0x00)

#define PN532_HOSTTOPN532 (0xD4)
#define PN532_PN532TOHOST (0xD5)

// PN532 Commands
#define PN532_COMMAND_DIAGNOSE (0x00)
#define PN532_COMMAND_GETFIRMWAREVERSION (0x02)
#define PN532_COMMAND_GETGENERALSTATUS (0x04)
#define PN532_COMMAND_READREGISTER (0x06)
#define PN532_COMMAND_WRITEREGISTER (0x08)
#define PN532_COMMAND_READGPIO (0x0C)
#define PN532_COMMAND_WRITEGPIO (0x0E)
#define PN532_COMMAND_SETSERIALBAUDRATE (0x10)
#define PN532_COMMAND_SETPARAMETERS (0x12)
#define PN532_COMMAND_SAMCONFIGURATION (0x14)
#define PN532_COMMAND_POWERDOWN (0x16)
#define PN532_COMMAND_RFCONFIGURATION (0x32)
#define PN532_COMMAND_RFREGULATIONTEST (0x58)
#define PN532_COMMAND_INJUMPFORDEP (0x56)
#define PN532_COMMAND_INJUMPFORPSL (0x46)
#define PN532_COMMAND_INLISTPASSIVETARGET (0x4A)
#define PN532_COMMAND_INATR (0x50)
#define PN532_COMMAND_INPSL (0x4E)
#define PN532_COMMAND_INDATAEXCHANGE (0x40)
#define PN532_COMMAND_INCOMMUNICATETHRU (0x42)
#define PN532_COMMAND_INDESELECT (0x44)
#define PN532_COMMAND_INRELEASE (0x52)
#define PN532_COMMAND_INSELECT (0x54)
#define PN532_COMMAND_INAUTOPOLL (0x60)
#define PN532_COMMAND_TGINITASTARGET (0x8C)
#define PN532_COMMAND_TGSETGENERALBYTES (0x92)
#define PN532_COMMAND_TGGETDATA (0x86)
#define PN532_COMMAND_TGSETDATA (0x8E)
#define PN532_COMMAND_TGSETMETADATA (0x94)
#define PN532_COMMAND_TGGETINITIATORCOMMAND (0x88)
#define PN532_COMMAND_TGRESPONSETOINITIATOR (0x90)
#define PN532_COMMAND_TGGETTARGETSTATUS (0x8A)

// Other Error Definitions
#define PN532_STATUS_ERROR (-1)
#define PN532_STATUS_OK (0)

// Constants
typedef unsigned int size_t;
typedef unsigned char byte_t;

/**
 * @fn pn532_init
 * ---------------------
 * Initializes spi interface and resets PN532 module.
 */
void pn532_init(unsigned int reset_pin, unsigned int nss_pin);

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
 * @fn rpi_spi_rw
 * ---------------------
 * Transmits data to peripheral and overwrites data with received bytes.
 */
void rpi_spi_rw(byte_t *data, size_t bufsize);

/**
 * @fn pn532_read_data
 * ---------------------
 * Sends read signal to pn532 and writes response into data buffer.
 */
void pn532_read_data(byte_t *data, size_t bufsize);

/**
 * @fn pn532_read_data
 * ---------------------
 * Writes data data to spi module.
 */
void pn532_write_data(byte_t *data, size_t bufsize);

/**
 * @fn pn532_wait_ready
 * ---------------------
 * 
 */
bool pn532_wait_ready(unsigned int timeout);

/**
 * @fn pn532_write_frame
 * ---------------------
 * @returns PN532_STATUS_ERROR if failed and PN532_STATUS_OK if suceeded
 * Structures message to send across spi by adding certain communication bytes.
 */
int pn532_write_frame(byte_t *data, size_t bufsize);

/**
 * @fn pn532_write_frame
 * ---------------------
 * @returns PN532_STATUS_ERROR if failed and PN532_STATUS_OK if suceeded
 * Reads response frame of at most bufsize bytes.
 */
int pn532_read_frame(byte_t *data, size_t bufsize);

/**
 * @fn pn532_send_command
 * ---------------------
 * @returns PN532_STATUS_ERROR if failed and PN532_STATUS_OK if suceeded
 * Sends command to pn532 and writes response into response.
 */
int pn532_send_commmad(byte_t command, byte_t *response, size_t response_length, byte_t *params, size_t params_length, unsigned int timeout);

/**
 * @fn pn532_get_firmware
 * ---------------------
 * @param version is 4 bytes but only uses bytes 1 and 2 not 0 and 3.
 * Requests pn532 firmware version and fills version. 
 */
int pn532_get_firmware_version(byte_t *version);

#endif // _PN532_H