// htab_hash_table_interface.h
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#ifndef PROJEKT2_HTAB_HASH_TABLE_H
#define PROJEKT2_HTAB_HASH_TABLE_H


#include <glob.h>
#include <stdio.h>
#include <stdbool.h>

struct htab_listitem {
    char *key; // ukazatel na dymanicky alokovany retezec
    unsigned long data; // pocet vyskytu retezce
    struct htab_listitem *next; // ukazatel na dalsi zaznam
};

typedef struct
{
    unsigned long size;  // aktualni pocet zaznamu struct htab_listitem [key,data,next]
    unsigned long arr_size; // velikost nasledujiciho pole ukazatelu
    struct htab_listitem *ptr[]; // pole ukazatelu na struct htab_listitem
} htab_t;

/**
 * @brief hash function
 * @param str string to be hashed
 * @return hash value
 */
unsigned int htab_hash_function(const char *str);

/**
 * @brief initialisation
 * @param size size of array of hash table
 * @return pointer to hash table
 */
htab_t *htab_init(unsigned long size);

/**
 * @brief move one hash table to another with different size
 * @param newsize new size of array of hash table
 * @param t2 pointer to old hash table
 * @return pointer to new hash table
 */
htab_t *htab_move(unsigned long newsize, htab_t *t2);

/**
 * @brief
 * @param t pointer to hash table
 * @return count of elemenets in hash table
 */
size_t htab_size(htab_t *t);

/**
 * @brief
 * @param t pointer to hash table
 * @return size of array of hash table
 */
size_t htab_bucket_count(htab_t *t);

/**
 * @brief look for a element with identical hash or add if not exist
 * @param t pointer to hash table
 * @param key string to be found
 * @return pointer to found or added element
 */
struct htab_listitem *htab_lookup_add(htab_t *t, const char *key);

/**
 * @brief look for a element with identical hash
 * @param t pointer to hash table
 * @param key string to be find
 * @return true if found, false if not found
 */
struct htab_listitem *htab_find(htab_t *t, const char *key);

/**
 * @brief call function on every element of hash table
 * @param t pointer to hash table
 * @param func function to be called
 */
void htab_foreach(htab_t *t, void (*func)(char *, unsigned long));

/**
 * @brief remove element from hash table
 * @param t pointer to hash table
 * @param key string to be removed
 * @return true if success, false if not exist
 */
bool htab_remove(htab_t *t, const char* key);

/**
 * @brief removing all elements, table stay empty
 * @param t pointer to hash table
 */
void htab_clear(htab_t *t);

/**
 * @brief destructor
 * @param t pointer to hash table
 */
void htab_free(htab_t *t);



#endif //PROJEKT2_HTAB_HASH_TABLE_H
