/**
 * @file pn532.h
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
#include <stdint.h>

#define PN532_FRAME_MAX_LENGTH 255
#define PN532_DEFAULT_TIMEOUT 1000

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

#define PN532_MIFARE_ISO14443A (0x00)

// Mifare Commands
#define MIFARE_CMD_AUTH_A (0x60)
#define MIFARE_CMD_AUTH_B (0x61)
#define MIFARE_CMD_READ (0x30)
#define MIFARE_CMD_WRITE (0xA0)
#define MIFARE_CMD_TRANSFER (0xB0)
#define MIFARE_CMD_DECREMENT (0xC0)
#define MIFARE_CMD_INCREMENT (0xC1)
#define MIFARE_CMD_STORE (0xC2)
#define MIFARE_ULTRALIGHT_CMD_WRITE (0xA2)

#define MIFARE_UID_MAX_LENGTH MIFARE_UID_TRIPLE_LENGTH
#define MIFARE_UID_SINGLE_LENGTH (4)
#define MIFARE_UID_DOUBLE_LENGTH (7)
#define MIFARE_UID_TRIPLE_LENGTH (10)
#define MIFARE_KEY_LENGTH (6)
#define MIFARE_BLOCK_LENGTH (16)

/* Official PN532 Errors Definitions */
#define PN532_ERROR_NONE (0x00)

// Other Error Definitions
#define PN532_STATUS_ERROR (-1)
#define PN532_STATUS_OK (0)

// Constants
typedef unsigned int size_t;

/**
 * @fn pn532_init
 * ---------------------
 * @description:Initializes spi interface and resets PN532 module.
 */
void pn532_init(unsigned int reset_pin, unsigned int nss_pin);

/**
 * @fn pn532_reset
 * ---------------------
 * @description: Resets pn532 module.
 */
void pn532_reset();

/**
 * @fn pn532_wakeup
 * ---------------------
 * @description: Wakes pn532 module.
 */
void pn532_wakeup();

/**
 * @fn rpi_spi_rw
 * ---------------------
 * @description: Transmits data to peripheral and overwrites data with received bytes.
 */
void rpi_spi_rw(uint8_t *data, size_t bufsize);

/**
 * @fn pn532_read_data
 * ---------------------
 * @description: Sends read signal to pn532 and writes response into data buffer.
 */
void pn532_read_data(uint8_t *data, size_t bufsize);

/**
 * @fn pn532_read_data
 * ---------------------
 * @description: Writes data data to spi module.
 */
void pn532_write_data(uint8_t *data, size_t bufsize);

/**
 * @fn pn532_wait_ready
 * ---------------------
 * @returns true if this function reads a SPI ready response from pn532 in timeout time
 */
bool pn532_wait_ready(unsigned int timeout);

/**
 * @fn pn532_write_frame
 * ---------------------
 * @description: Structures message to send across spi by adding certain communication bytes.
 * @returns PN532_STATUS_ERROR if failed and PN532_STATUS_OK if suceeded
 */
int pn532_write_frame(uint8_t *data, size_t bufsize);

/**
 * @fn pn532_write_frame
 * ---------------------
 * @description: Reads response frame of at most bufsize bytes.
 * @returns PN532_STATUS_ERROR if failed and PN532_STATUS_OK if suceeded
 */
int pn532_read_frame(uint8_t *data, size_t bufsize);

/**
 * @fn pn532_send_receive
 * ---------------------
 * @description: Sends command to pn532 and writes response into response.
 * @returns number of bytes received back from the HAT, or PN532_STATUS_ERROR if something went wrong
 */
int pn532_send_receive(uint8_t command, uint8_t *response, size_t response_length, uint8_t *params, size_t params_length, unsigned int timeout);

/**
 * @fn pn532_get_firmware
 * ---------------------
 * @description: Requests pn532 firmware version and fills version. 
 * @param version is 4 bytes but only uses bytes 1 and 2 not 0 and 3.
 */
int pn532_get_firmware_version(uint8_t *version);

/* -------------------------------------------------------------------------- */
/*                                COMMAND FUNCTIONS START                                */
/* -------------------------------------------------------------------------- */

/** 
 * @fn pn532_config_normal
 * ---------------------
 * @description: Configures the SAM to normal mode. 
 * @returns PN532_STATUS_OK after completing 
 */
int pn532_config_normal();

/**
 * @fn pn532_sam_config
 * ---------------------
 * @description: Configures SAM to mode based on params.
 * @returns PN532_STATUS_OK after completing 
 */
int pn532_sam_config(uint8_t mode, uint8_t timeout, uint8_t use_irq_pin);

int tag_data_dump();

#endif // _PN532_H