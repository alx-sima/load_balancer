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
 * @brief Calculeaza hashul cheii, folosind functia corespunzatoare
 * hashtable-ului.
 */
unsigned int ht_compute_hash(hashtable *ht, void *key);

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
hashtable *ht_create(unsigned int num_buckets, size_t data_size,
					 size_t key_size, unsigned int (*hash_func)(void *));

/**
 * @brief Insereaza o copie a valorii date la cheia specificata
 */
void ht_store_item(hashtable *ht, void *key, void *value);

/**
 * @brief Sterge din hashtable elementul cu cheia data.
 */
void ht_delete_item(hashtable *ht, void *key);

/**
 * @brief Returneaza informatia stocata la cheia data.
 */
void *ht_get_item(hashtable *ht, void *key);

void *ht_clone_val(hashtable *ht, void *key);

list *ht_pop_entry(hashtable *ht);

/**
 * @brief Alcatuieste o lista inlantuita cu perechile (cheie, valoare) care au
 * hashul mai mic decat o valoare data, stergandu-le din hashtable.
 *
 * @return Inceputul listei simplu inlantuite cu perechi.
 */
list *ht_chain_entries(hashtable *ht, unsigned int max_hash);

/**
 * @brief Returneaza si sterge din dictionar prima pereche (cheie, valoare) cu
 * hashul dat.
 *
 * @return dict_entry* prima pereche (cheie, valoare) gasita
 * @return NULL nu exista intrari cu hashul dat.
 */
dict_entry *ht_pop_hash_entry(hashtable *ht, unsigned int hash);

/**
 * @brief Sterge hashtableul si toate resursele alocate de acesta.
 */
void ht_destroy(hashtable *ht);

void ht_transfer_items(hashtable *dest, hashtable *src, unsigned int max_hash);
#endif /* HASHTABLE_H_ */
