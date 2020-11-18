/**
 *  @file test_nfc.c
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


#include <stdint.h> //use standard integer library


const unsigned int RESET_PIN = GPIO_PIN20;
const unsigned int NSS_PIN = GPIO_PIN4;

void test_spi_transfer()
{
    byte_t frame[7] = {0x00, 0x00, 0xff, 0x02, 0xe0, 0x2a, 0x00};
    pn532_write_data(frame, 7);

    byte_t read_data[262];
    memset(read_data, 0, 262);
    pn532_read_data(read_data, 262);

    printf("\n{");
    for (int i = 0; i < 262; i++)
    {
        printf("%x, ", read_data[i]);
        if (i % 10 == 0)
            printf("\n");
    }
    printf("}\n");
}

void test_firmwareVersion()
{
    byte_t buf[4];
    pn532_get_firmware_version(buf);
    print_bytes(buf, 4);
}

void basic_tests(void)
{
    printf("\n----------------------------------------\n");
    printf("|           NFC Basic Testing          |\n");
    printf("|   CONSOLE SIZE: 40 x 40 (in chars)   |\n");
    printf("----------------------------------------\n");
}

void test_SamConfig() { //make sure we can configure the HAT
    byte_t buff[4];
    if (pn532_get_firmware_version(buff) == PN532_STATUS_OK) {
        printf("Found PN532 with firmware version: %d.%d\r\n", buff[1], buff[2]);
    } else {
        printf("failed to find firmware version");
        return;
    }
    if(pn532_SamConfig() == PN532_STATUS_OK) {
        printf("SamConfig successefully executed. HAT should now be in normal mode.");
    };
}






void test_getCardUID() { //make sure a MiFare card UID can be obtained using pn532_ReadPassiveTarget
    
    int32_t uid_len = 0; //length of UID returned
    uint8_t uid[MIFARE_UID_MAX_LENGTH]; //holds the UID received from the HAT

    
    if(pn532_SamConfig() == PN532_STATUS_OK) {
        printf("SamConfig successefully executed. HAT should now be in normal mode.");
    }
    else {
        printf("Couldn't configure HAT");
        return;
    }
    
    printf("Waiting for RFID/NFC card...\r\n");

    while (1)
    {
        // Check if a card is available to read
        uid_len = pn532_ReadPassiveTarget(uid, PN532_MIFARE_ISO14443A, 1000);
        if (uid_len == PN532_STATUS_ERROR) {
            printf(".");
        } else {
            printf("Found card with UID: ");
            for (uint8_t i = 0; i < uid_len; i++) {
                printf("%02x ", uid[i]);
            }
            printf("\r\n");
            break;
        }
    } 
}


void main(void)
{
    pn532_init(RESET_PIN, NSS_PIN);
    // test_spi_transfer();
    // test2();
    // test_firmwareVersion();


    //----------VX Tests----------
    // test_SamConfig();
    test_getCardUID();

    uart_putchar(EOT);
}