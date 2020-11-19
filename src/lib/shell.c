#include "shell.h"
#include "shell_commands.h"
#include "uart.h"
#include "keyboard.h"
#include "strings.h"
#include "malloc.h"
#include "pi.h"
#include <printf.h>
#include <nfc_shell_commands.h>

#define LINE_LEN 80

static formatted_fn_t shell_printf;
typedef unsigned char uint8_t;

static const size_t COMMAND_SIZE = 5;
static const command_t commands[] = {
    {"help", "<cmd> prints a list of commands or description of cmd", cmd_help},
    {"echo", "<...> echos the user input to the screen", cmd_echo},
    {"reboot", "reboots the Raspberry Pi back to the bootloader", cmd_reboot},
    {"peek", "[address] prints the contents of memory at address", cmd_peek},
    {"poke", "[address] [value] store value into memory at address", cmd_poke},
    {"charge", "[value] charges tag with value", cmd_charge_tag},
    {"read", "[block number] prints block", cmd_read_tag},
    {"pay", "[value] pays tag with value", cmd_pay_tag},
    {"set", "[value] sets tag balance", cmd_set_tag_value},
    {"check", "checks tag balance", cmd_check_tag_balance},
};

static void print_bytes(uint8_t *buf, size_t bufsize)
{
    // Print vertical line numbers
    shell_printf("\n     ");
    int num_length = bufsize < 16 ? bufsize : 16;
    for (int i = 0; i < num_length; i++)
    {
        if (i > 9)
            shell_printf("%d ", i);
        else
            shell_printf(" %d ", i);
    }

    for (int i = 0; i < bufsize; i++)
    {
        if (i % 16 == 0)
            shell_printf("\n%02d : ", i / 16);
        shell_printf("%02x ", buf[i]);
    }
    shell_printf("\n");
}

int cmd_read_tag(int argc, const char *argv[])
{
    // Check that command has no args
    if (argc != 1)
    {
        shell_printf("error: read takes no arguments\n");
        return 1;
    }

    // Print tag
}

/**
 * @fn findCommand
 * ---------------------
 * Iterates through command to find command that matches name. Returns index of command that
 * matches. Returns -1 if no command is found.
 */
int findCommand(const char *name)
{
    for (int i = 0; i < COMMAND_SIZE; i++)
    {
        if (strcmp(commands[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

int cmd_echo(int argc, const char *argv[])
{
    for (int i = 1; i < argc; ++i)
        shell_printf("%s ", argv[i]);
    shell_printf("\n");
    return 0;
}

/**
 * @fn getAddress
 * ---------------------
 * @param stringAddress is a string version of address
 * @param name is the name of the function that calls getAddress
 * @returns address as pointer or NULL if address is invalid
 */
void *getAddress(const char *name, const char *stringAddress)
{
    // Check for a readable arg
    char *end;
    unsigned int address = strtonum(stringAddress, (const char **)&end);
    if (strcmp(end, stringAddress + strlen(stringAddress)) != 0)
    {
        shell_printf("error: %s cannot convert '%s'\n", name, stringAddress);
        return (void *)-1;
    }

    // Check for four byte alignment
    if (address % 4 != 0)
    {
        shell_printf("error: address must be 4-byte aligned '%s'\n", stringAddress);
        return (void *)-1;
    }

    // Return address
    return (int *)address;
}

int cmd_poke(int argc, const char *argv[])
{
    // Check for at least 2 arg
    if (argc < 3)
    {
        shell_printf("error: peek requires 2 arguments [address] and [value]\n");
        return 1;
    }

    // Get address
    int *ptr = getAddress("poke", argv[1]);
    if (ptr == (void *)-1)
        return 1;

    // Check if second argument is valid
    char *end;
    int value = strtonum(argv[2], (const char **)&end);
    if (strcmp(end, argv[2] + strlen(argv[2])) != 0)
    {
        shell_printf("error: poke cannot convert '%s'\n", argv[2]);
        return 1;
    }

    *ptr = value;
    return 0;
}

int cmd_peek(int argc, const char *argv[])
{
    // Check for at least 1 arg
    if (argc < 2)
    {
        shell_printf("error: peek requires 1 argument [address]\n");
        return 1;
    }

    // Get address
    int *ptr = getAddress("peek", argv[1]);
    if (ptr == (void *)-1)
        return 1;

    shell_printf("%p: %8x\n", ptr, *ptr);
    return 0;
}

int cmd_help(int argc, const char *argv[])
{
    if (argc > 1)
    {
        int index = findCommand(argv[1]);
        if (index == -1)
        {
            shell_printf("error: no such command '%s'\n", argv[1]);
            return 1;
        }
        else
        {
            shell_printf("%s: %s\n", commands[index].name, commands[index].description);
            return 0;
        }
    }

    for (int i = 0; i < COMMAND_SIZE; i++)
    {
        shell_printf("%s: %s\n", commands[i].name, commands[i].description);
    }
    return 0;
}

void shell_init(formatted_fn_t print_fn)
{
    shell_printf = print_fn;
}

int cmd_reboot(int argc, const char *argv[])
{
    uart_send(EOT);
    pi_reboot();
    return 0;
}

void shell_bell(void)
{
    uart_putchar('\a');
}

static bool isalnum(char c)
{
    return (c > 32 && c < 127);
}

static bool isspace(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\n';
}

static bool canPrint(char ch)
{
    return isspace(ch) || isalnum(ch);
}

void shell_readline(char buf[], size_t bufsize)
{
    int i = 0;
    unsigned char c = 0;

    while (i < bufsize - 1)
    {
        c = keyboard_read_next();

        // Enter operation
        if (c == '\n')
            break;

        // Backspace operation
        else if (c == '\b')
        {
            if (i == 0)
                shell_bell();
            else
            {
                i--;
                shell_printf("%c", '\b');
                shell_printf("%c", ' ');
                shell_printf("%c", '\b');
            }
        }
        else if (i < bufsize - 2 && canPrint(c))
        {
            buf[i] = c;
            shell_printf("%c", c);
            i++;
        }
    }

    shell_printf("\n");
    buf[i] = '\0';
}

/**
 * @fn strndup
 * ---------------------
 * Adapted from heapclient.c from lab4 :). Duplicates string onto heap and returns pointer to it.
 */
char *strndup(const char *src, size_t n)
{
    size_t len = strlen(src) < n ? strlen(src) : n;
    char *p = malloc(len + 1);
    memcpy(p, src, len);
    p[len] = '\0';

    return p;
}

/**
 * @fn tokenize
 * ---------------------
 * @returns number of tokens
 * @param max is the max number of tokens
 * Adapted from heapclient.c from lab4 part 2 :). Tokenizes string. Allocates memory; make sure to
 * free it.
 */
static int tokenize(const char *line, char *array[], int max)
{
    int ntokens = 0;
    const char *cur = line;

    while (ntokens < max)
    {
        while (isspace(*cur))
            cur++; // skip spaces (stop non-space/null)
        if (*cur == '\0')
            break; // no more non-space chars
        const char *start = cur;
        while (*cur != '\0' && !isspace(*cur))
            cur++;                                      // advance to end (stop space/null)
        array[ntokens++] = strndup(start, cur - start); // make heap-copy, add to array
    }
    return ntokens;
}

int shell_evaluate(const char *line)
{
    char *array[strlen(line)];
    size_t ntokens = tokenize(line, array, strlen(line));
    if (ntokens == 0)
        return -1;

    // Grab command or echo error
    char *name = array[0];
    int index = findCommand(name);
    if (index == -1)
    {
        shell_printf("error: no such command '%s'\n", name);
        return -1;
    }

    // Call function
    const char **argv = (const char **)&array;
    int result = commands[index].fn(ntokens, argv);

    // Free tokens
    for (int i = 0; i < ntokens; i++)
    {
        free(array[i]);
    }

    return result;
}

void shell_run(void)
{
    // pri>tf("Welcome to the CS107E shell. Remember to type on your PS/2 keyboard!\n");
    shell_printf("Welcome to the CS107E shell. Remember to type on your PS/2 keyboard!\n");
    while (1)
    {
        char line[LINE_LEN];

        shell_printf("Pi> ");
        shell_readline(line, sizeof(line));
        shell_evaluate(line);
    }
    return;
}