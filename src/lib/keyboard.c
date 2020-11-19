#include "gpio.h"
#include "gpioextra.h"
#include "keyboard.h"
#include "ps2.h"
#include "timer.h"
#include "printf.h"
#include "gpio_interrupts.h"
#include "ringbuffer.h"
#include "malloc.h"

static unsigned int START_BIT = 0;
static unsigned int STOP_BIT = 0;
static unsigned int CLK, DATA;
static int MODIFIERS = 0;
rb_t *rb;

// Global scancode reading variables
static volatile int bit_num = 0;
static volatile bool is_valid_scancode = true;
static volatile unsigned int bit_place = 2;
static volatile unsigned int parity = 0;
static volatile unsigned char scancode = 0;

enum
{
    PS2_CMD_RESET = 0xFF,
    PS2_CODE_ACK = 0xFA,
    PS2_CMD_FLAGS = 0xED,
    PS2_CMD_ENABLE_DATA_REPORTING = 0xF4
};

static void wait_for_falling_clock_edge(void)
{
    while (gpio_read(CLK) == 0)
    {
    }
    while (gpio_read(CLK) == 1)
    {
    }
}

/**
 * @fn reset
 * ---------------------
 * Resets global scancode variables
 */
static void reset()
{
    bit_num = 0;
    parity = 0;
    bit_place = 1;
    scancode = 0;
    is_valid_scancode = true;
}

/**
 * @fn read_bit
 * ---------------------
 * Handler for clock pin that reads bit on falling edge.
 */
static bool read_bit(unsigned int pc)
{
    if (!gpio_check_and_clear_event(CLK))
    {
        return false;
    }

    unsigned int bit = gpio_read(DATA);
    // printf("%d ", bit_num);
    switch (bit_num)
    {
    case 0:
        if (bit)
            is_valid_scancode = false;
        break;
    case 10:
        if (!bit)
            is_valid_scancode = false;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        parity += bit;
        scancode += bit * bit_place;
        bit_place *= 2;
        break;
    case 9:
        if ((parity + bit) % 2 == 0)
            is_valid_scancode = false;
        break;
    }

    if (bit_num == 10)
    {
        if (!rb_full(rb) && is_valid_scancode)
        {
            rb_enqueue(rb, scancode);
        }
        reset();
    }
    else
    {
        bit_num++;
    }

    return true;
}

/** 
 * @fn update_modifiers
 * ---------------------
 * @returns true if MODIFIERS was updated
 * Updates MODIFIERS global variable.
 */
static bool update_modifiers(key_event_t event)
{
    if (event.action.what == KEY_PRESS)
    {
        switch (event.key.ch)
        {
        case PS2_KEY_SHIFT:
            MODIFIERS |= KEYBOARD_MOD_SHIFT;
            break;
        case PS2_KEY_CTRL:
            MODIFIERS |= KEYBOARD_MOD_CTRL;
            break;
        case PS2_KEY_ALT:
            MODIFIERS |= KEYBOARD_MOD_ALT;
            break;
        case PS2_KEY_CAPS_LOCK:
            MODIFIERS ^= KEYBOARD_MOD_CAPS_LOCK;
            break;
        case PS2_KEY_SCROLL_LOCK:
            MODIFIERS ^= KEYBOARD_MOD_SCROLL_LOCK;
            break;
        case PS2_KEY_NUM_LOCK:
            MODIFIERS ^= KEYBOARD_MOD_NUM_LOCK;
            break;
        default:
            return false;
            break;
        }
    }

    else if (event.action.what == KEY_RELEASE)
    {
        switch (event.key.ch)
        {
        case PS2_KEY_SHIFT:
            MODIFIERS &= ~KEYBOARD_MOD_SHIFT;
            break;
        case PS2_KEY_CTRL:
            MODIFIERS &= ~KEYBOARD_MOD_CTRL;
            break;
        case PS2_KEY_ALT:
            MODIFIERS &= ~KEYBOARD_MOD_ALT;
            break;
        case PS2_KEY_CAPS_LOCK:
        case PS2_KEY_SCROLL_LOCK:
        case PS2_KEY_NUM_LOCK:
            break;
        default:
            return false;
            break;
        }
    }

    return true;
}

static unsigned has_odd_parity(unsigned char code)
{
    return code % 2;
}

static void write_bit(int nbit, unsigned char code)
{
    switch (nbit)
    {
    case 1:
        gpio_write(DATA, START_BIT);
        break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        gpio_write(DATA, (code >> (nbit - 2)) & 1);
        break;
    case 10:
        gpio_write(DATA, has_odd_parity(code));
        break;
    case 11:
        gpio_write(DATA, STOP_BIT);
        break;
    }
}

static bool ps2_write(unsigned char command)
{
    gpio_set_output(CLK); // swap GPIO from read to write
    gpio_set_output(DATA);
    gpio_write(CLK, 0);  // bring clock line low to inhibit communication
    timer_delay_us(200); // hold time > 100 us
    gpio_write(DATA, 0); // initiate request-to-send, this is START bit
    gpio_write(CLK, 1);
    gpio_set_input(CLK); // release clock line, device will now pulse clock for host to write

    for (int i = 2; i <= 11; i++)
    { // START bit sent above, now send bits 2-11
        wait_for_falling_clock_edge();
        write_bit(i, command);
    }

    gpio_set_input(DATA); // done writing, exit from request-to-send
    wait_for_falling_clock_edge();
    // return (gpio_read(DATA) == 0) && (keyboard_read_scancode() == PS2_CODE_ACK); // device should respond with ack
    return true;
}

void keyboard_init(unsigned int clock_gpio, unsigned int data_gpio)
{
    CLK = clock_gpio;
    DATA = data_gpio;

    // Send acknowledgement to keyboard
    if (ps2_write(PS2_CMD_RESET))
    {
        unsigned char response = keyboard_read_scancode();
        (void)response; // instead ignore self-test result, may correct itself
    }

    // Configure pin interrutps
    gpio_set_input(CLK);
    gpio_set_pullup(CLK);

    gpio_set_input(DATA);
    gpio_set_pullup(DATA);

    gpio_interrupts_enable();
    gpio_enable_event_detection(CLK, GPIO_DETECT_ASYNC_FALLING_EDGE);
    gpio_interrupts_register_handler(CLK, read_bit);

    // Initialize ringbuffer
    rb = rb_new();
}

unsigned char keyboard_read_scancode(void)
{
    // Spin while rb is empty
    while (rb_empty(rb))
        ;

    // Grab scancode
    int *p_elem = malloc(sizeof(int));
    rb_dequeue(rb, p_elem);
    unsigned char scancode = *p_elem;
    free(p_elem);

    return scancode;
}

key_action_t keyboard_read_sequence(void)
{
    key_action_t action;
    char code = keyboard_read_scancode();

    // Extended codes
    if (code == PS2_CODE_EXTENDED)
    {
        code = keyboard_read_scancode();
    }
    // Key release
    if (code == PS2_CODE_RELEASE)
    {
        action.what = KEY_RELEASE;
        code = keyboard_read_scancode();
    }
    // Key press
    else
    {
        action.what = KEY_PRESS;
    }

    action.keycode = code;

    return action;
}

key_event_t keyboard_read_event(void)
{
    bool isModifier = true;
    key_event_t event;

    while (isModifier)
    {
        key_action_t action = keyboard_read_sequence();

        event.action = action;
        event.key = ps2_keys[action.keycode];
        event.modifiers = MODIFIERS;

        isModifier = update_modifiers(event);
    }

    return event;
}

static bool isalpha(char c)
{
    if (c > 96 && c < 123)
    {
        return true;
    }
    else if (c > 64 && c < 91)
    {
        return true;
    }

    return false;
}

static bool isspace(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\n';
}

unsigned char keyboard_read_next(void)
{
    for (;;)
    {
        key_event_t event = keyboard_read_event();
        if (event.action.what == KEY_RELEASE)
            continue;

        // Special characters
        if (event.key.ch > PS2_KEY_SHIFT || isspace(event.key.ch))
        {
            return event.key.ch;
        }

        // Not-special characters
        if ((event.modifiers & KEYBOARD_MOD_SHIFT) != 0)
        {
            return event.key.other_ch;
        }
        else if ((event.modifiers & KEYBOARD_MOD_CAPS_LOCK) != 0 && isalpha(event.key.ch))
        {
            return event.key.other_ch;
        }

        return event.key.ch;
    }
}
