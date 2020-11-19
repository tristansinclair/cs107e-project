/**
 * @file nfc_shell_commands.h
 * ---------------------
 * @brief Defines shell commands to interact with nfc tags
 */
#ifndef _NFC_SHELL_COMMANDS_H
#define _NFC_SHELL_COMMANDS_H

/**
 * @fn cmd_read_tag
 * ---------------------
 * @description: Reads and prints tag block specified by second arg. Reads and prints whole tag if 
 * there is no second argument.
 */
int cmd_read_tag(int argc, const char *argv[]);

/**
 * @fn cmd_charge_tag
 * ---------------------
 * @description: Charges tag amount specified by second argument and updates tag balance.
 */
int cmd_charge_tag(int argc, const char *argv[]);

/**
 * @fn cmd_set_tag_value
 * ---------------------
 * @description: Sets tag value to second argument.
 */
int cmd_set_tag_value(int argc, const char *argv[]);

/**
 * @fn cmd_check_tag_balance
 * ---------------------
 * @description: Prints tag balance.
 */
int cmd_check_tag_balance(int argc, const char *argv[]);

/**
 * @fn cmd_pay_tag
 * ---------------------
 * @description: Adds amount specified by second argument to tag balance.
 */
int cmd_pay_tag(int argc, const char *argv[]);

#endif // _NFC_SHELL_COMMANDS_H