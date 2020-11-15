/**
 *  @file pn532.c
 * ---------------------
 * @brief Implements pn532.h
 */

#include <pn532.h>
#include <gpio.h>
#include <timer.h>

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
    digitalWrite(_RESET_PIN, 1);
    timer_delay_ms(100);
    digitalWrite(_RESET_PIN, 0);
    timer_delay_ms(500);
    digitalWrite(_RESET_PIN, 1);
    timer_delay_ms(100);
}

void pn532_wakeup()
{
    // Send any special commands/data to wake up PN532
    char data[] = {0x00};
    char rx[1];
    timer_delay_ms(1000);
    digitalWrite(_NSS_PIN, 0);
    timer_delay_ms(2); // T_osc_start
    spi_transfer(data, rx, 1);
    timer_delay_ms(1000);
}

void pn532_read_data(char *data, size_t bufsize)
{
    // char frame[bufsize + 1];
    // frame[0] = _SPI_DATAREAD;
    // // delay
    // spi_transfer(frame, frame, bufsize + 1);
    // memcpy(data, frame, bufsize);
    char frame[bufsize];
    frame[0] = _SPI_DATAREAD;
    spi_transfer(frame, data, bufsize);
}

void pn532_write_data(char *data, size_t bufsize)
{
    // char frame[bufsize + 1];
    // frame[0] = _SPI_DATAWRITE;
    // memcpy(frame + 1, data, bufsize);
    // rpi_spi_rw(frame, bufsize + 1);
    // spi_transfer(frame, frame, bufsize + 1);
    char frame[bufsize + 1];
    frame[0] = _SPI_DATAWRITE;
    memcpy(frame + 1, data, bufsize);
    rpi_spi_rw(frame, bufsize + 1);
    spi_transfer(data, frame, bufsize + 1);
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