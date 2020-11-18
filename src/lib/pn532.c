/**
 *  @file pn532.c
 * ---------------------
 * @brief Implements pn532.h
 */

#include <pn532.h>

#include <stdint.h> //use standard integer library


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


//-------------SUPPORTING FUNCTIONS START----------------

void pn532_init(unsigned int reset_pin, unsigned int nss_pin) //initialize the HAT to send/receive over SPI
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

//-------------SUPPORTING FUNCTIONS END------------------




//-------------FRAME WRITING FUNCTIONS START ------------

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

// Send a command frame over SPI and receive the response; this mimics the function of 'PN532_CallFunction' from the Waveshare 'pn532.c'
// Returns number of bytes received back from the HAT, or PN532_STATUS_ERROR if something went wrong
int pn532_send_receive(byte_t command, byte_t *response, size_t response_length, byte_t *params, size_t params_length, unsigned int timeout)
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
    // Then return the number of bytes read
    return frame_len - 2;
}

//-------------FRAME WRITING FUNCTIONS END------------



//-------------CALL FUNCTIONS START------------

int pn532_get_firmware_version(byte_t *version)
{
    if(pn532_send_receive(PN532_COMMAND_GETFIRMWAREVERSION, version, 4, NULL, 0, 500) == PN532_STATUS_ERROR) {
        printf("pn532_get_firmware_version failed to detect the PN532");
        return PN532_STATUS_ERROR;
    }
    return PN532_STATUS_OK;
}

int pn532_SamConfig() { //Configure the PN532 to read MiFare cards using normal mode
    // Send SAM configuration command with configuration for:
    // - 0x01, normal mode
    // - 0x14, timeout 50ms * 20 = 1 second
    // - 0x01, use IRQ pin
    // Note that no other verification is necessary as call_function will
    // check the command was executed as expected.
    uint8_t params[] = {0x01, 0x14, 0x01};
    pn532_send_receive(PN532_COMMAND_SAMCONFIGURATION,
                       NULL, 0, params, sizeof(params), PN532_DEFAULT_TIMEOUT);
    return PN532_STATUS_OK;
}


int pn532_ReadPassiveTarget( //Wait for a MiFare card to be available and return its UID when found.
    uint8_t* response,
    uint8_t card_baud,
    uint32_t timeout
) {
    // Send passive read command for 1 card.  Expect at most a 7 byte UUID.
    uint8_t params[] = {0x01, card_baud};
    uint8_t buff[19];
    int length = pn532_send_receive(PN532_COMMAND_INLISTPASSIVETARGET,
                        buff, sizeof(buff), params, sizeof(params), timeout);

    if (length < 0) {
        return PN532_STATUS_ERROR; // No card found
    }
    // Check only 1 card with up to a 7 byte UID is present.
    if (buff[0] != 0x01) {
        printf("Response byte: %d\n", buff[0]); // ASK KAI
        printf("More than one card detected!");
        return PN532_STATUS_ERROR;
    }
    if (buff[5] > 7) {
        printf("Found card with unexpectedly long UID!");
        return PN532_STATUS_ERROR;
    }
    for (uint8_t i = 0; i < buff[5]; i++) {
        response[i] = buff[6 + i];
    }
    return buff[5];
}








int pn532_authenticateBlock( //Unlock a tag memory block
    uint8_t* uid,
    uint8_t uid_length,
    uint16_t block_number,
    uint16_t key_number,
    uint8_t* key
) {
    // Build parameters for InDataExchange command to authenticate MiFare card.
    uint8_t response[1] = {0xFF};
    uint8_t params[3 + MIFARE_UID_MAX_LENGTH + MIFARE_KEY_LENGTH];
    params[0] = 0x01;
    params[1] = key_number & 0xFF;
    params[2] = block_number & 0xFF;
    // params[3:3+keylen] = key
    for (uint8_t i = 0; i < MIFARE_KEY_LENGTH; i++) {
        params[3 + i] = key[i];
    }
    // params[3+keylen:] = uid
    for (uint8_t i = 0; i < uid_length; i++) {
        params[3 + MIFARE_KEY_LENGTH + i] = uid[i];
    }
    // Send InDataExchange request
    pn532_send_receive(PN532_COMMAND_INDATAEXCHANGE, response, sizeof(response),
                       params, 3 + MIFARE_KEY_LENGTH + uid_length, PN532_DEFAULT_TIMEOUT);
    return response[0];
}


int pn532_readBlock(uint8_t* response, uint16_t block_number) {
    uint8_t params[] = {0x01, MIFARE_CMD_READ, block_number & 0xFF};
    uint8_t buff[MIFARE_BLOCK_LENGTH + 1];
    // Send InDataExchange request to read block of MiFare data.
    pn532_send_receive(PN532_COMMAND_INDATAEXCHANGE, buff, sizeof(buff),
                       params, sizeof(params), PN532_DEFAULT_TIMEOUT);
    // Check first response is 0x00 to show success.
    if (buff[0] != PN532_ERROR_NONE) {
        return buff[0];
    }
    for (uint8_t i = 0; i < MIFARE_BLOCK_LENGTH; i++) {
        response[i] = buff[i + 1];
    }
    return buff[0];
}



//-------------CALL FUNCTIONS END------------



//--------------VX SUPPORTING-----------------

void run_check_config() { //helper fxn to run SamConfig and print conditions for mode configuration
    if(pn532_SamConfig() == PN532_STATUS_OK) {
        printf("SamConfig successfully executed. HAT is now in normal mode.");
    }
    else {
        printf("Couldn't configure HAT");
        return;
    }
}


int tag_dataDump() { //wrapper function to dump the data contents of a tag. Returns success/failure

    int32_t uid_len = 0; //length of UID returned
    uint8_t uid[MIFARE_UID_MAX_LENGTH]; //holds the UID received from the HAT

    run_check_config();
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

*/