/**
 *  @file pn532.c
 * ---------------------
 * @brief Implements pn532.h
 */

#include <pn532.h>

#define HIGH 1
#define LOW 0
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

char reverse_byte(char byte)
{
    char result = 0;
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
    char data[] = {0x00};
    timer_delay_ms(1000);
    gpio_write(_NSS_PIN, 0);
    timer_delay_ms(2); // T_osc_start
    rpi_spi_rw(data, 1);
    timer_delay_ms(1000);
}

void rpi_spi_rw(char *data, size_t bufsize)
{
    gpio_write(_NSS_PIN, LOW);
    timer_delay_ms(1);
    // #ifndef _SPI_HARDWARE_LSB
    //     for (uint8_t i = 0; i < bufsize; i++)
    //     {
    //         data[i] = reverse_byte(data[i]);
    //     }
    //     wiringPiSPIDataRW(_SPI_CHANNEL, data, bufsize);
    //     for (uint8_t i = 0; i < count; i++)
    //     {
    //         data[i] = reverse_byte(data[i]);
    //     }
    // #else
    // #endif
    unsigned char rx[bufsize];
    spi_transfer((unsigned char *)data, rx, bufsize);
    memcpy(data, rx, bufsize);

    timer_delay_ms(1);
    gpio_write(_NSS_PIN, HIGH);
}

void pn532_read_data(char *data, size_t bufsize)
{
    // char frame[bufsize + 1];
    // frame[0] = _SPI_DATAREAD;
    // // timer_delay_ms
    // rpi_spi_rw(frame, frame, bufsize + 1);
    // memcpy(data, frame, bufsize);
    // unsigned char frame[bufsize];
    // frame[0] = _SPI_DATAREAD;
    // rpi_spi_rw(frame, bufsize);
}

void pn532_write_data(char *data, size_t bufsize)
{
    // char frame[bufsize + 1];
    // frame[0] = _SPI_DATAWRITE;
    // memcpy(frame + 1, data, bufsize);
    // rpi_spi_rw(frame, bufsize + 1);
    // rpi_spi_rw(frame, frame, bufsize + 1);
    // char frame[bufsize + 1];
    // frame[0] = _SPI_DATAWRITE;
    // memcpy(frame + 1, data, bufsize);
    //rpi_spi_rw(frame, bufsize + 1);
    // rpi_spi_rw(data, frame, bufsize + 1);
}