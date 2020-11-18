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

// void test2()
// {
//     byte_t response[4];

//     // Build frame data with command and parameters.
//     byte_t buff[255];
//     buff[0] = PN532_HOSTTOPN532;
//     buff[1] = PN532_COMMAND_GETFIRMWAREVERSION & 0xFF;

//     // Send frame and wait for response.
//     // if (PN532_WriteFrame(pn532, buff, 2) != PN532_STATUS_OK)
//     pn532_write_frame(buff, 2);

//     if (!pn532_wait_ready(500))
//     {
//         // return PN532_STATUS_ERROR;
//     }
//     // Verify ACK response and wait to be ready for function response.
//     pn532_read_data(buff, sizeof(PN532_ACK));
//     for (int i = 0; i < sizeof(PN532_ACK); i++)
//     {
//         if (PN532_ACK[i] != buff[i])
//         {
//             // pn532->log("Did not receive expected ACK from PN532!");
//             printf("no ACK");
//             // return PN532_STATUS_ERROR;
//         }
//     }
//     if (!pn532_wait_ready(500))
//     {
//         printf("bad status");
//         // return PN532_STATUS_ERROR;
//     }
//     // Read response bytes.
//     int frame_len = pn532_read_frame(buff, 4 + 2);

//     // Check that response is for the called function.
//     if (!((buff[0] == PN532_PN532TOHOST) && (buff[1] == (PN532_COMMAND_GETFIRMWAREVERSION + 1))))
//     {
//         // pn532->log("Received unexpected command response!");
//         // return PN532_STATUS_ERROR;
//     }
//     // Return response data.
//     for (int i = 0; i < 4; i++)
//     {
//         response[i] = buff[i + 2];
//     }
//     // The the number of bytes read
//     printf("%d", frame_len - 2);

//     print_message(response, 4);
// }

void test3()
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

void test_SamConfig() {
    if (PN532_GetFirmwareVersion(&pn532, buff) == PN532_STATUS_OK) {
        printf("Found PN532 with firmware version: %d.%d\r\n", buff[1], buff[2]);
    } else {
        return -1;
    }
    pn532_SamConfig();
}


void main(void)
{
    pn532_init(RESET_PIN, NSS_PIN);
    // test_spi_transfer();
    // test2();
    test3();
    uart_putchar(EOT);
}