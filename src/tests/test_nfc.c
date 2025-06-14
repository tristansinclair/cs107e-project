/**
 * @file test_nfc.c
 * ---------------------
 * @brief Tests nfc and pn532 capabilities.
 */

#include <uart.h>
#include <spi.h>
#include <gpio.h>
#include <pn532.h>
#include <timer.h>
#include <printf.h>
#include <nfc.h>
#include <assert.h>

#include <stdint.h> //use standard integer library

static const unsigned int RESET_PIN = GPIO_PIN20;
static const unsigned int NSS_PIN = GPIO_PIN4;

/** 
 * @fn test_firmware_version
 * ---------------------
 * @description: request a firmware response from the pn532 module
 */
static void test_firmware_version(void)
{
    uint8_t buf[4];
    pn532_get_firmware_version(buf);
    print_bytes(buf, 4);

    printf("Found PN532 with firmware version: %d.%d\r\n", buf[1], buf[2]);
    assert(buf[1] == 1);
    assert(buf[2] == 6);
}

/** 
 * @fn test_sam_config
 * ---------------------
 * @description: configure sam mode so the hat is in normal mode
 */
static void test_sam_config(void)
{
    uint8_t buf[4];

    //make sure we can configure the HAT
    if (pn532_get_firmware_version(buf) == PN532_STATUS_OK)
    {
        printf("Found PN532 with firmware version: %d.%d\r\n", buf[1], buf[2]);
    }
    else if (pn532_get_firmware_version(buf) != PN532_STATUS_OK)
    {
        printf("failed to find firmware version");
        return;
    }
    if (pn532_config_normal() == PN532_STATUS_OK)
    {
        printf("SamConfig successefully executed. HAT should now be in normal mode.\n");
    };
}

/** 
 * @fn test_get_card_uid
 * ---------------------
 * @description: make sure a MiFare card UID can be obtained using pn532_ReadPassiveTarget
 */
static void test_get_card_uid(void)
{
    uint8_t uid[MIFARE_UID_MAX_LENGTH];
    int32_t uid_len;

    assert(pn532_config_normal() == PN532_STATUS_OK);

    printf("SamConfig successefully executed. HAT should now be in normal mode.\n");
    printf("Waiting for RFID/NFC card...\r\n");

    while (1)
    {
        // Check if a card is available to read
        uid_len = pn532_read_passive_target(uid, PN532_MIFARE_ISO14443A, 1000);
        if (uid_len == PN532_STATUS_ERROR)
        {
            printf(".");
        }
        else
        {
            printf("\nFound card with UID: ");
            for (uint8_t i = 0; i < uid_len; i++)
            {
                printf("%02x ", uid[i]);
            }
            printf("\r\n");
            break;
        }
    }
}

/** 
 * @fn test_get_block_info
 * ---------------------
 * @description: prints block data read from card
 */
static void test_get_block_info(void)
{
    pn532_config_normal();

    uint8_t uid[MIFARE_UID_MAX_LENGTH];
    int32_t uid_len;

    while (1)
    {
        // Check if a card is available to read
        uid_len = pn532_read_passive_target(uid, PN532_MIFARE_ISO14443A, 1000);
        if (uid_len == PN532_STATUS_ERROR)
        {
            printf(".");
        }
        else
        {
            printf("\nFound card with UID: ");
            for (uint8_t i = 0; i < uid_len; i++)
            {
                printf("%02x ", uid[i]);
            }
            printf("\r\n");
            break;
        }
    }

    uint8_t buf[255];
    uint8_t key_a[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // store the default passcode for the tag blocks
    unsigned int pn532_error = PN532_ERROR_NONE;

    printf("Reading blocks...\r\n");

    uint8_t buf2[1024];

    for (size_t block_number = 0; block_number < 64; block_number++)
    {
        //int timer = timer_get_ticks();
        pn532_error = pn532_authenticate_block(uid, uid_len, block_number, MIFARE_CMD_AUTH_A, key_a);
        // assert(pn532_error == PN532_ERROR_NONE);
        // timer = timer_get_ticks() - timer;
        // printf("%d\n", timer);

        pn532_error = pn532_read_block(buf, block_number);
        // assert(pn532_error == PN532_ERROR_NONE);

        memcpy(buf2 + 16 * block_number, buf, 16);
    }
    print_bytes(buf2, 1024);
    printf("\r\n");
    if (pn532_error)
    {
        printf("Error: 0x%02x\r\n", pn532_error);
    }
}

/** 
 * @fn test_rw_mifare
 * ---------------------
 * @description: tests reading and writing to a mifare card
 */
static int test_rw_mifare(void)
{
    uint8_t buf[255];
    uint8_t uid[MIFARE_UID_MAX_LENGTH];
    uint8_t key_a[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    unsigned int pn532_error = PN532_ERROR_NONE;
    int32_t uid_len = 0;

    pn532_config_normal();

    printf("Waiting for RFID/NFC card...\r\n");
    while (1)
    {
        // Check if a card is available to read
        uid_len = pn532_read_passive_target(uid, PN532_MIFARE_ISO14443A, 1000);
        if (uid_len == PN532_STATUS_ERROR)
        {
            printf(".");
        }
        else
        {
            printf("\nFound card with UID: ");
            for (uint8_t i = 0; i < uid_len; i++)
            {
                printf("%02x ", uid[i]);
            }
            printf("\r\n");
            break;
        }
    }
    /**
      * Warning: DO NOT write the blocks of 4N+3 (3, 7, 11, ..., 63)
      * or else you will change the password for blocks 4N ~ 4N+2.
      * Note: 
      * 1.  The first 6 bytes (KEY A) of the 4N+3 blocks are always shown as 0x00,
      * since 'KEY A' is unreadable. In contrast, the last 6 bytes (KEY B) of the 
      * 4N+3 blocks are readable.
      * 2.  Block 0 is unwritable. 
      */
    // Write block #6
    uint8_t block_number = 6;
    uint8_t DATA[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    pn532_error = pn532_authenticate_block(uid, uid_len,
                                           block_number, MIFARE_CMD_AUTH_A, key_a);
    if (pn532_error)
    {
        printf("Error: 0x%02x\r\n", pn532_error);
        return -1;
    }
    pn532_error = pn532_mifare_classic_write_block(DATA, block_number);
    if (pn532_error)
    {
        printf("Error: 0x%02x\r\n", pn532_error);
        return -1;
    }
    pn532_error = pn532_read_block(buf, block_number);
    if (pn532_error)
    {
        printf("Error: 0x%02x\r\n", pn532_error);
        return -1;
    }
    for (uint8_t i = 0; i < sizeof(DATA); i++)
    {
        if (DATA[i] != buf[i])
        {
            printf("Write block %d failed\r\n", block_number);
            return -1;
        }
    }
    printf("Write block %d successfully\r\n", block_number);

    print_bytes(buf, 255);

    return PN532_ERROR_NONE;
}

/** 
 * @fn test_card_balance
 * ---------------------
 * @description: tests setting and reading card balance
 */
static void test_card_balance(void)
{
    int *value = 0;

    printf("Begin by placing fob on the scanner until balance is set.\n");
    gpio_write(16, 1);
    assert(set_balance(100) == PN532_ERROR_NONE);
    gpio_write(16, 0);
    printf("Balance set to 100\n");

    timer_delay(3);
    printf("\nScan fob to show balance\n");
    gpio_write(16, 1);
    assert(get_balance(value) == PN532_ERROR_NONE);
    gpio_write(16, 0);
    printf("Current Balance: %d\n", *value);

    timer_delay(3);
    printf("\nNow scan once more to deduct 25\n");
    gpio_write(16, 1);
    assert(set_balance(75) == PN532_ERROR_NONE);
    gpio_write(16, 0);
    printf("Balanced deducted.\n");


    timer_delay(3);
    printf("\nScan fob to show balance\n");
    gpio_write(16, 1);
    assert(get_balance(value) == PN532_ERROR_NONE);
    gpio_write(16, 0);
    printf("Current Balance: %d\n", *value);
    printf("test_card_balance complete.\n");
}

void main(void)
{
    nfc_init(RESET_PIN, NSS_PIN);

    gpio_init();
    gpio_set_output(16);

    printf("\n\n------------- Firmware Version Test -------------\n");
    test_firmware_version(); // request and print firmware version
    printf("\n-------------------------------------------------\n\n\n");

    printf("----------------- SamConfig Test ----------------\n");
    test_sam_config();
    printf("\n-------------------------------------------------\n\n\n");

    printf("------------------ Get Card UID -----------------\n");
    test_get_card_uid();
    printf("\n-------------------------------------------------\n\n\n");

    printf("----------------- Get Block Info ----------------\n");
    test_get_block_info();
    printf("\n-------------------------------------------------\n\n\n");

    printf("-------------- Mifare Card R/W Test -------------\n");
    assert(test_rw_mifare() == PN532_ERROR_NONE);
    printf("\n-------------------------------------------------\n\n\n");

    printf("--------------- Card Balance Tests --------------\n");
    test_card_balance();
    printf("\n-------------------------------------------------\n");

    uart_putchar(EOT);
}