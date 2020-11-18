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



#include <stdint.h> //use standard integer library




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


#define PN532_MIFARE_ISO14443A              (0x00)

// Mifare Commands
#define MIFARE_CMD_AUTH_A                   (0x60)
#define MIFARE_CMD_AUTH_B                   (0x61)
#define MIFARE_CMD_READ                     (0x30)
#define MIFARE_CMD_WRITE                    (0xA0)
#define MIFARE_CMD_TRANSFER                 (0xB0)
#define MIFARE_CMD_DECREMENT                (0xC0)
#define MIFARE_CMD_INCREMENT                (0xC1)
#define MIFARE_CMD_STORE                    (0xC2)
#define MIFARE_ULTRALIGHT_CMD_WRITE         (0xA2)

#define MIFARE_UID_MAX_LENGTH               MIFARE_UID_TRIPLE_LENGTH
#define MIFARE_UID_SINGLE_LENGTH            (4)
#define MIFARE_UID_DOUBLE_LENGTH            (7)
#define MIFARE_UID_TRIPLE_LENGTH            (10)
#define MIFARE_KEY_LENGTH                   (6)
#define MIFARE_BLOCK_LENGTH                 (16)


/* Official PN532 Errors Definitions */
#define PN532_ERROR_NONE                   (0x00)

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
int pn532_send_receive(byte_t command, byte_t *response, size_t response_length, byte_t *params, size_t params_length, unsigned int timeout);

/**
 * @fn pn532_get_firmware
 * ---------------------
 * @param version is 4 bytes but only uses bytes 1 and 2 not 0 and 3.
 * Requests pn532 firmware version and fills version. 
 */
int pn532_get_firmware_version(byte_t *version);



//-------------- CALL FUNCTIONS START --------------

/** @description: configure the SAM to normal mode 
 *  @retval: returns PN532_STATUS_OK after completing 
 */
int pn532_SamConfig();

/**
  * @brief: Wait for a MiFare card to be available and return its UID when found.
  *     Will wait up to timeout seconds and return None if no card is found,
  *     otherwise a bytearray with the UID of the found card is returned.
  * @retval: Length of UID, or -1 if error.
  */
int pn532_ReadPassiveTarget(
    uint8_t* response,
    uint8_t card_baud,
    uint32_t timeout
);



/**
  * @brief: Authenticate a specified block number for a MiFare classic card.
  * @param uid: A byte array with the UID of the card.
  * @param uid_length: Length of the UID of the card.
  * @param block_number: The block to authenticate.
  * @param key_number: The key type (like MIFARE_CMD_AUTH_A or MIFARE_CMD_AUTH_B).
  * @param key: A byte array with the key data.
  * @retval: true if the block was authenticated, or false if not authenticated.
  * @retval: PN532 error code.
  */
int pn532_authenticateBlock(
    uint8_t* uid,
    uint8_t uid_length,
    uint16_t block_number,
    uint16_t key_number,
    uint8_t* key
);


/**
  * @brief: Read a block of data from the card. Block number should be the block
  *     to read.
  * @param response: buffer of length 16 returned if the block is successfully read.
  * @param block_number: specify a block to read.
  * @retval: PN532 error code.
  */
int pn532_readBlock(uint8_t* response, uint16_t block_number);



int tag_dataDump();


#endif // _PN532_H