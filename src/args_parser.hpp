#pragma once

/**
 * Counts the number of arguments in a string (separated by spaces/tabs)
 * @param str Input string to count arguments from
 * @return Number of arguments found
 */
int count_arguments(const char* str);

/**
 * Parses a string into command-line style arguments
 * @param input_str Input string to parse (e.g., "5 + 3")
 * @param argc_out Output parameter for argument count (including program name)
 * @return Array of argument strings, or NULL on failure
 *         Caller must free memory using free_arguments()
 */
char** parse_arguments_c(const char* input_str, int* argc_out);

/**
 * Frees memory allocated by parse_arguments_c
 * @param argv Array of argument strings returned by parse_arguments_c
 * @param argc Number of arguments in the array
 */
void free_arguments_c(char** argv, int argc);