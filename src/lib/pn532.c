/**
 *  @file pn532.c
 * ---------------------
 * @brief Implements pn532.h
 */

#include <pn532.h>

#define HIGH 1
#define LOW 0
#define PN532_FRAME_MAX_LENGTH 255
#define PN532_DEFAULT_TIMEOUT 1000

// SPI codes
#define _SPI_STATREAD (0x02)
#define _SPI_DATAWRITE (0x01)
#define _SPI_DATAREAD (0x03)
#define _SPI_READY (0x01)
#define _SPI_CHANNEL (0)

const byte_t PN532_ACK[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
const byte_t PN532_FRAME_START[] = {0x00, 0x00, 0xFF};
static unsigned int _RESET_PIN, _NSS_PIN;

void pn532_init(unsigned int reset_pin, unsigned int nss_pin)
{
    // Initialize pins 7-11 for spi
    spi_init(SPI_CE0, 1);

    // Initialize reset and nss pins
    _RESET_PIN = reset_pin;
    _NSS_PIN = nss_pin;
    gpio_set_output(_RESET_PIN);
    gpio_set_output(_NSS_PIN);

    // Reset and wakeup module
    pn532_reset();
    pn532_wakeup();
}

void pn532_reset()
{
    gpio_write(_RESET_PIN, HIGH);
    timer_delay_ms(100);
    gpio_write(_RESET_PIN, LOW);
    timer_delay_ms(500);
    gpio_write(_RESET_PIN, HIGH);
    timer_delay_ms(100);
}

byte_t reverse_byte(byte_t byte)
{
    byte_t result = 0;
    for (char i = 0; i < 8; i++)
    {
        result <<= 1;
        result += (byte & 1);
        byte >>= 1;
    }
    return result;
}

void pn532_wakeup()
{
    // Send any special commands/data to wake up PN532
    byte_t data[] = {0x00};
    timer_delay_ms(1000);
    gpio_write(_NSS_PIN, LOW);
    timer_delay_ms(2); // T_osc_start
    rpi_spi_rw(data, HIGH);
    timer_delay_ms(1000);
}

void rpi_spi_rw(byte_t *data, size_t bufsize)
{
    gpio_write(_NSS_PIN, LOW);
    timer_delay_ms(1);

    for (int i = 0; i < bufsize; i++)
    {
        data[i] = reverse_byte(data[i]);
    }

    byte_t rx[bufsize];
    spi_transfer(data, rx, bufsize);

    for (int i = 0; i < bufsize; i++)
    {
        data[i] = reverse_byte(rx[i]);
    }

    timer_delay_ms(1);
    gpio_write(_NSS_PIN, HIGH);
}

void pn532_read_data(byte_t *data, size_t bufsize)
{
    // Copy data into new frame with read byte as first byte
    byte_t frame[bufsize + 1];
    memset(frame, 0, bufsize + 1);
    frame[0] = _SPI_DATAREAD;

    // Transmits frame bytes and copies response into data
    timer_delay_ms(5);
    rpi_spi_rw(frame, bufsize + 1);
    memcpy(data, frame + 1, bufsize);
}

void pn532_write_data(byte_t *data, size_t bufsize)
{
    // Copy data into new frame with write byte as first byte
    byte_t frame[bufsize + 1];
    memcpy(frame + 1, data, bufsize);
    frame[0] = _SPI_DATAWRITE;

    // Write frame
    rpi_spi_rw(frame, bufsize + 1);
}

int pn532_write_frame(byte_t *data, size_t bufsize)
{
    // Checks for valid bufsize
    if (bufsize > PN532_FRAME_MAX_LENGTH || bufsize < 1)
    {
        return PN532_STATUS_ERROR; // Data must be array of 1 to 255 bytes.
    }
    // Build frame to send as:
    // - Preamble (0x00)
    // - Start code  (0x00, 0xFF)
    // - Command length (1 byte)
    // - Command length checksum
    // - Command bytes
    // - Checksum
    // - Postamble (0x00)

    byte_t frame[PN532_FRAME_MAX_LENGTH + 7];
    byte_t checksum = 0;
    frame[0] = PN532_PREAMBLE;
    frame[1] = PN532_STARTCODE1;
    frame[2] = PN532_STARTCODE2;
    for (byte_t i = 0; i < 3; i++)
    {
        checksum += frame[i];
    }
    frame[3] = bufsize & 0xFF;
    frame[4] = (~bufsize + 1) & 0xFF;
    for (byte_t i = 0; i < bufsize; i++)
    {
        frame[5 + i] = data[i];
        checksum += data[i];
    }
    frame[bufsize + 5] = ~checksum & 0xFF;
    frame[bufsize + 6] = PN532_POSTAMBLE;

    pn532_write_data(frame, bufsize + 7);

    return PN532_STATUS_OK;
}

int pn532_read_frame(byte_t *response, size_t bufsize)
{
    byte_t buf[PN532_FRAME_MAX_LENGTH + 7];
    byte_t checksum = 0;

    // Read frame with expected length of data.
    pn532_read_data(buf, bufsize + 7);

    // Swallow all the 0x00 values that preceed 0xFF.
    byte_t offset = 0;
    while (buf[offset] == 0x00)
    {
        offset += 1;
        if (offset >= bufsize + 8)
        {
            // pn532->log("Response frame preamble does not contain 0x00FF!");
            printf("\nResponse frame preamble does not contain 0x00FF!\n");
            return PN532_STATUS_ERROR;
        }
    }
    if (buf[offset] != 0xFF)
    {
        // pn532->log("Response frame preamble does not contain 0x00FF!");
        printf("\nResponse frame preamble does not contain 0x00FF!\n");
        return PN532_STATUS_ERROR;
    }
    offset += 1;
    if (offset >= bufsize + 8)
    {
        // pn532->log("Response contains no data!");
        printf("\nResponse contains no data\n");
        return PN532_STATUS_ERROR;
    }
    // Check length & length checksum match.
    byte_t frame_len = buf[offset];
    if (((frame_len + buf[offset + 1]) & 0xFF) != 0)
    {
        // pn532->log("Response length checksum did not match length!");
        printf("\nResponse length checksum did not match length!\n");
        return PN532_STATUS_ERROR;
    }
    // Check frame checksum value matches bytes.
    for (byte_t i = 0; i < frame_len + 1; i++)
    {
        checksum += buf[offset + 2 + i];
    }
    checksum &= 0xFF;
    if (checksum != 0)
    {
        // pn532->log("Response checksum did not match expected checksum");
        printf("\nResponse checksum did not match expected checksum\n");
        return PN532_STATUS_ERROR;
    }
    // Return frame data.
    for (byte_t i = 0; i < frame_len; i++)
    {
        response[i] = buf[offset + 2 + i];
    }
    return frame_len;
}

bool pn532_wait_ready(unsigned int timeout)
{
    byte_t status[] = {_SPI_STATREAD, 0x00};

    unsigned int timestart = timer_get_ticks();
    unsigned int timenow;
    while (1)
    {
        timer_delay_ms(10);
        rpi_spi_rw(status, sizeof(status));
        if (status[1] == _SPI_READY)
        {
            return true;
        }
        else
        {
            timer_delay_ms(5);
        }
        timenow = timer_get_ticks();
        if (1000 * (timenow - timestart) > timeout)
            break;
    }
    return false;
}

int pn532_send_commmand(byte_t command, byte_t *response, size_t response_length, byte_t *params, size_t params_length, unsigned int timeout)
{
    // Build frame data with command and parameters.
    byte_t buf[PN532_FRAME_MAX_LENGTH];
    buf[0] = PN532_HOSTTOPN532;
    buf[1] = command & 0xFF;
    // for (int i = 0; i < params_length; i++)
    // {
    //     buf[2 + i] = params[i];
    // }
    memcpy(buf + 2, params, params_length);

    // Send frame and wait for response.
    if (pn532_write_frame(buf, params_length + 2) != PN532_STATUS_OK)
    {
        pn532_wakeup();
        // pn532->log("Trying to wakeup");
        printf("Trying to wakeup");
        return PN532_STATUS_ERROR;
    }

    // Grab status bytes
    if (!pn532_wait_ready(timeout))
        return PN532_STATUS_ERROR;

    // Verify ACK response and wait to be ready for function response.
    pn532_read_data(buf, sizeof(PN532_ACK));
    for (int i = 0; i < sizeof(PN532_ACK); i++)
    {
        if (PN532_ACK[i] != buf[i])
        {
            // pn532->log("Did not receive expected ACK from PN532!");
            printf("Did not receive expected ACK from PN532!");
            return PN532_STATUS_ERROR;
        }
    }
    if (!pn532_wait_ready(timeout))
    {
        return PN532_STATUS_ERROR;
    }

    // Read response bytes.
    int frame_len = pn532_read_frame(buf, response_length + 2);

    // Check that response is for the called function.
    if (!((buf[0] == PN532_PN532TOHOST) && (buf[1] == (command + 1))))
    {
        // pn532->log("Received unexpected command response!");
        printf("Received unexpected command response!");
        return PN532_STATUS_ERROR;
    }

    // Return response data.
    for (int i = 0; i < response_length; i++)
    {
        response[i] = buf[i + 2];
    }
    // The the number of bytes read
    return frame_len - 2;
}

int pn532_get_firmware_version(byte_t *version)
{
    return pn532_send_commmand(PN532_COMMAND_GETFIRMWAREVERSION, version, 4, NULL, 0, 500);
}


// it looks like the example modules only use SAM normal mode
    // for abstraction, we make our own pn532_set_configuration function, and then a
    // "pn532_set_normal_configuration" function within that

/**
  * @brief: Configure the PN532 to read MiFare cards using normal mode
  */

int PN532_SamConfiguration(PN532* pn532) {
    // Send SAM configuration command with configuration for:
    // - 0x01, normal mode
    // - 0x14, timeout 50ms * 20 = 1 second
    // - 0x01, use IRQ pin
    // Note that no other verification is necessary as call_function will
    // check the command was executed as expected.
    uint8_t params[] = {0x01, 0x14, 0x01};
    PN532_CallFunction(pn532, PN532_COMMAND_SAMCONFIGURATION,
                       NULL, 0, params, sizeof(params), PN532_DEFAULT_TIMEOUT);
    return PN532_STATUS_OK;
}





/**
  * @brief: Wait for a MiFare card to be available and return its UID when found.
  *     Will wait up to timeout seconds and return None if no card is found,
  *     otherwise a bytearray with the UID of the found card is returned.
  * @retval: Length of UID, or -1 if error.
  */

/*int PN532_ReadPassiveTarget(
    PN532* pn532,
    uint8_t* response,
    uint8_t card_baud,
    uint32_t timeout
) {
    // Send passive read command for 1 card.  Expect at most a 7 byte UUID.
    uint8_t params[] = {0x01, card_baud};
    uint8_t buff[19];
    int length = PN532_CallFunction(pn532, PN532_COMMAND_INLISTPASSIVETARGET,
                        buff, sizeof(buff), params, sizeof(params), timeout);
    if (length < 0) {
        return PN532_STATUS_ERROR; // No card found
    }
    // Check only 1 card with up to a 7 byte UID is present.
    if (buff[0] != 0x01) {
        pn532->log("More than one card detected!");
        return PN532_STATUS_ERROR;
    }
    if (buff[5] > 7) {
        pn532->log("Found card with unexpectedly long UID!");
        return PN532_STATUS_ERROR;
    }
    for (uint8_t i = 0; i < buff[5]; i++) {
        response[i] = buff[6 + i];
    }
    return buff[5];
}*/



//--------- NFC TAG DUMP ------------

void tag_dump() {
    uint8_t buff[255];
    uint8_t uid[MIFARE_UID_MAX_LENGTH];
    uint8_t key_a[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint32_t pn532_error = PN532_ERROR_NONE;
    int32_t uid_len = 0;
    printf("Hello!\r\n");
    PN532 pn532;
    PN532_SPI_Init(&pn532);

    if (pn532_get_firmware_version(buff) == PN532_STATUS_OK) {
        printf("Found PN532 with firmware version: %d.%d\r\n", buff[1], buff[2]);
    } else {
        return -1;
    }
    PN532_SamConfiguration(&pn532);
    printf("Waiting for RFID/NFC card...\r\n");
    while (1)
    {
        // Check if a card is available to read
        uid_len = PN532_ReadPassiveTarget(&pn532, uid, PN532_MIFARE_ISO14443A, 1000);
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
    printf("Reading blocks...\r\n");
    for (uint8_t block_number = 0; block_number < 64; block_number++) {
        pn532_error = PN532_MifareClassicAuthenticateBlock(&pn532, uid, uid_len,
                block_number, MIFARE_CMD_AUTH_A, key_a);
        if (pn532_error != PN532_ERROR_NONE) {
            break;
        }
        pn532_error = PN532_MifareClassicReadBlock(&pn532, buff, block_number);
        if (pn532_error != PN532_ERROR_NONE) {
            break;
        }
        printf("%d: ", block_number);
        for (uint8_t i = 0; i < 16; i++) {
            printf("%02x ", buff[i]);
        }
        printf("\r\n");
    }
    if (pn532_error) {
        printf("Error: 0x%02x\r\n", pn532_error);
    }

}