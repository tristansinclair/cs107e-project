/**
 * @file nfc.h
 * ---------------------
 * @brief API for reading and writing nfc tags.
 */
#ifndef _NFC_H
#define _NFC_H

#include "pn532.h"

/**
 * @fn nfc_init
 * ---------------------
 * @description: Initializes pn532 and nfc capabilities.
 */
void nfc_init(unsigned int reset_pin, unsigned int nss_pin);

/**
 * @fn print_bytes
 * ---------------------
 * @description: Prints buffer of bytes in a nice readable format.
 */
void print_bytes(uint8_t *buf, size_t bufsize);

/**
 * @fn pn532_read_passive_target
 * ---------------------
 * @desciption: Blocks until Mifare card is available and fills response with UID when found
 *     Will wait up to timeout seconds and return None if no card is found,  *     otherwise a bytearray with the UID of the found card is returned.
 * @returns Length of UID, or -1 if error.
 */
int pn532_read_passive_target(uint8_t *response, uint8_t card_baud, size_t timeout);

/**
 * @fn pn532_authenticate_block
 * ---------------------
 * @description: Authenticate a specified block number for a MiFare classic card.
 * @param uid: A byte array with the UID of the card.
 * @param uid_length: Length of the UID of the card.
 * @param block_number: The block to authenticate.
 * @param key_number: The key type (like MIFARE_CMD_AUTH_A or MIFARE_CMD_AUTH_B).
 * @param key: A byte array with the key data.
 * @returns: true if the block was authenticated, or false if not authenticated.
 * @retval: PN532 error code.
 */
int pn532_authenticate_block(uint8_t *uid, size_t uid_length, size_t block_number, size_t key_number, uint8_t *key);

/**
 * @fn pn532_read_block
 * ---------------------
 * @description: Read a block of data from the card. Block number should be the block     to read.
 * @param response: bufer of length 16 returned if the block is successfully read.
 * @param block_number: specify a block to read.
 * @returns: PN532 error code.
 */
int pn532_read_block(uint8_t *response, size_t block_number);

#endif // _NFC_H