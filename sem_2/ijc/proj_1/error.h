// error.h
// Řešení IJC-DU1, příklad a), 14.3.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Přeloženo: gcc 7.3.1

#ifndef ERROR_H
#define ERROR_H
/**
 * @brief funkce vypise na STDERR chybovu hlasku, parametry ma stejne jako funkce printf
 */
void warning_msg(const char *fmt, ...);

/**
 * @brief funkce vypise na STDERR chybovu hlasku a ukonci program, parametry ma stejne jako funkce printf
 */
void error_exit(const char *fmt, ...);

#endif //ERROR_H
