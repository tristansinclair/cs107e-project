/**
 * @file nfc.c
 * ---------------------
 * @brief Implements nfc.h
 */

#include <nfc.h>

void nfc_init(unsigned int reset_pin, unsigned int nss_pin)
{
    pn532_init(reset_pin, nss_pin);
}

void print_bytes(uint8_t *buf, size_t bufsize)
{
    // Print vertical line numbers
    printf("\n     ");
    int num_length = bufsize < 16 ? bufsize : 16;
    for (int i = 0; i < num_length; i++)
    {
        if (i > 9)
            printf("%d ", i);
        else
            printf(" %d ", i);
    }

    for (int i = 0; i < bufsize; i++)
    {
        if (i % 16 == 0)
            printf("\n%02d : ", i / 16);
        printf("%02x ", buf[i]);
    }
    printf("\n");
}

int pn532_read_passive_target(uint8_t *response, uint8_t card_baud, size_t timeout)
{
    // Send passive read command for 1 card.  Expect at most a 7 byte UUID.
    uint8_t params[] = {0x01, card_baud};
    uint8_t buf[19];
    int length = pn532_send_receive(PN532_COMMAND_INLISTPASSIVETARGET,
                                    buf, sizeof(buf), params, sizeof(params), timeout);

    if (length < 0)
    {
        return PN532_STATUS_ERROR; // No card found
    }

    // Check only 1 card with up to a 7 byte UID is present.
    if (buf[0] != 0x01)
    {
        printf("Response byte: %d\n", buf[0]); // ASK KAI
        printf("More than one card detected!");
        return PN532_STATUS_ERROR;
    }
    if (buf[5] > 7)
    {
        printf("Found card with unexpectedly long UID!");
        return PN532_STATUS_ERROR;
    }
    for (uint8_t i = 0; i < buf[5]; i++)
    {
        response[i] = buf[6 + i];
    }
    return buf[5];
}

int pn532_authenticate_block(uint8_t *uid, size_t uid_length, size_t block_number, size_t key_number, uint8_t *key)
{
    // Build parameters for InDataExchange command to authenticate MiFare card.
    uint8_t response[1] = {0xFF};
    uint8_t params[3 + MIFARE_UID_MAX_LENGTH + MIFARE_KEY_LENGTH];
    params[0] = 0x01;
    params[1] = key_number & 0xFF;
    params[2] = block_number & 0xFF;

    // params[3:3+keylen] = key
    for (int i = 0; i < MIFARE_KEY_LENGTH; i++)
    {
        params[3 + i] = key[i];
    }
    // params[3+keylen:] = uid
    for (int i = 0; i < uid_length; i++)
    {
        params[3 + MIFARE_KEY_LENGTH + i] = uid[i];
    }

    // Send InDataExchange request
    pn532_send_receive(PN532_COMMAND_INDATAEXCHANGE, response, sizeof(response), params, 3 + MIFARE_KEY_LENGTH + uid_length, PN532_DEFAULT_TIMEOUT);
    return response[0];
}

int pn532_read_block(uint8_t *response, size_t block_number)
{
    uint8_t params[] = {0x01, MIFARE_CMD_READ, block_number & 0xFF};
    uint8_t buf[MIFARE_BLOCK_LENGTH + 1];
    // Send InDataExchange request to read block of MiFare data.
    pn532_send_receive(PN532_COMMAND_INDATAEXCHANGE, buf, sizeof(buf),
                       params, sizeof(params), PN532_DEFAULT_TIMEOUT);

    // Check first response is 0x00 to show success.
    if (buf[0] != PN532_ERROR_NONE)
    {
        return buf[0];
    }
    for (int i = 0; i < MIFARE_BLOCK_LENGTH; i++)
    {
        response[i] = buf[i + 1];
    }
    return buf[0];
}

//-------------CALL FUNCTIONS END------------

//--------------VX SUPPORTING-----------------

void run_check_config()
{
    //helper fxn to run SamConfig and print conditions for mode configuration
    if (pn532_config_normal() == PN532_STATUS_OK)
    {
        printf("SamConfig successfully executed. HAT is now in normal mode.");
    }
    else
    {
        printf("Couldn't configure HAT");
        return;
    }
}

int pn532_mifare_classic_write_block(uint8_t *data, size_t block_number)
{
    uint8_t params[MIFARE_BLOCK_LENGTH + 3];
    uint8_t response[1];
    params[0] = 0x01; // Max card numbers
    params[1] = MIFARE_CMD_WRITE;
    params[2] = block_number & 0xFF;

    for (int i = 0; i < MIFARE_BLOCK_LENGTH; i++)
    {
        params[3 + i] = data[i];
    }

    pn532_send_receive(PN532_COMMAND_INDATAEXCHANGE, response, sizeof(response), params, sizeof(params), PN532_DEFAULT_TIMEOUT);
    return response[0];
}

int get_balance(uint8_t *response)
{
    pn532_config_normal();

    uint8_t uid[MIFARE_UID_MAX_LENGTH];
    int32_t uid_len;

    while (1)
    {
        // Check if a card is available to read
        uid_len = pn532_read_passive_target(uid, PN532_MIFARE_ISO14443A, 1000);
        if (uid_len == PN532_STATUS_ERROR)
            printf("Error reading tag.");
        else
            break;
    }

    uint8_t buf[255];
    uint8_t key_a[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // store the default passcode for the tag blocks
    unsigned int pn532_error = PN532_ERROR_NONE;

    pn532_error = pn532_authenticate_block(uid, uid_len, 6, MIFARE_CMD_AUTH_A, key_a);
    pn532_error = pn532_read_block(buf, 6);

    if (pn532_error)
    {
        printf("Error: 0x%02x\r\n", pn532_error);
    }

    memcpy(response, buf, 4);
}

int set_balance()
{
}