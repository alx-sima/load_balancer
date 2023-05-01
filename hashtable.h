/* Copyright 2023 Sima Alexandru (312CA) */
#ifndef HASHTABLE_H_
#define HASHTABLE_H_
#include <stddef.h>

#include "list.h"

typedef struct hashtable {
	unsigned int num_buckets;
	list **buckets;

	size_t data_size;
	size_t key_size;

	unsigned int (*hash_func)(void *);
} hashtable;

/**
 * @brief Creeaza si initializaeaza un hashtable.
 *
 * @param num_buckets Numarul de bucketuri ale hashtable-ului
 * @param data_size Dimensiunea valorilor stocate
 * @param key_size Dimensiunea cheii
 * @param hash_func Functia de hash a cheilor
 *
 * @return Hashtable-ul creat
 */
hashtable *create_ht(unsigned int num_buckets, size_t data_size,
					 size_t key_size, unsigned int (*hash_func)(void *));

/**
 * @brief Insereaza o copie a valorii date la cheia specificata
 */
void insert_item_ht(hashtable *ht, void *key, void *value);

/**
 * @brief Sterge din hashtable elementul cu cheia data.
 */
void remove_item_ht(hashtable *ht, void *key);

/**
 * @brief Returneaza informatia stocata la cheia data.
 */
void *get_item_ht(hashtable *ht, void *key);

/**
 * @brief Returneaza si sterge din dictionar prima pereche (cheie, valoare) cu
 * hashul dat.
 *
 * @return dict_entry* prima pereche (cheie, valoare) gasita
 * @return NULL nu exista intrari cu hashul dat.
 */
dict_entry *pop_hash_entry(hashtable *ht, unsigned int hash);

/**
 * @brief Sterge hashtableul si toate resursele alocate de acesta.
 */
void delete_ht(hashtable *ht);

#endif /* HASHTABLE_H_ */
