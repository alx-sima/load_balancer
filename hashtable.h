/* Copyright 2023 Sima Alexandru (312CA) */
#ifndef HASHTABLE_H_
#define HASHTABLE_H_
#include <stddef.h>

#include "list.h"

/**
 * @class hashtable
 * @brief O tabela de dispersie care poate retine elemente generice.
 */
typedef struct hashtable {
	/** numarul de bucketuri pe care le are hashtableul */
	unsigned int num_buckets;
	/** bucketurile hashtableului */
	list **buckets;

	/** functia de hash a cheilor */
	unsigned int (*hash_func)(void *key);
	/** functia de comparare a cheilor */
	int (*compare_func)(void *key1, void *key2);
	/** functia de eliberare a cheilor si valorilor */
	void (*destructor_func)(void *key, void *data);
} hashtable;

/**
 * @relates hashtable
 * @brief Creeaza si initializaeaza un hashtable.
 *
 * @param num_buckets 		numarul de bucketuri ale hashtable-ului
 * @param hash_func 		functia de hash a cheilor
 * @param compare_func 		functia de comparare a cheilor
 * @param destructor_func	functia de eliberare a cheilor si valorilor

 * @return Hashtable-ul creat
 */
hashtable *ht_create(unsigned int num_buckets,
					 unsigned int (*hash_func)(void *key),
					 int (*compare_func)(void *key1, void *key2),
					 void (*destructor_func)(void *key, void *data));

/**
 * @relates hashtable
 * @brief Insereaza in hashtable o noua pereche (cheie, valoare).
 *
 * @param ht 	hashtable-ul in care se insereaza
 * @param key 	cheia la care se face insertia
 * @param value	valoarea inserata
 */
void ht_store_item(hashtable *ht, void *key, void *value);

/**
 * @relates hashtable
 * @brief Returneaza informatia stocata la cheia data.
 *
 * @param ht 	hashtable-ul din care se cauta
 * @param key 	cheia cautata
 */
void *ht_retrieve_item(hashtable *ht, void *key);

/**
 * @relates hashtable
 * @brief Sterge din hashtable elementul cu cheia data.
 *
 * @param ht 	hashtable-ul din care se sterge
 * @param key 	cheia elementului de sters
 */
void ht_remove_item(hashtable *ht, void *key);

/**
 * @relates hashtable
 * @brief Transfera obiectele stocate dintr-un hashtable in altul, in functie
 * de hashul acsetora.
 *
 * @param dest		hashtable-ul destinatie
 * @param src		hashtable-ul original
 * @param min_hash	hashul minim pe care il au obiectele transferate
 * @param max_hash	hashul maxim pe care il au obiectele transferate
 */
void ht_transfer_items(hashtable *dest, hashtable *src, unsigned int min_hash,
					   unsigned int max_hash);

/**
 * @relates hashtable
 * @brief Sterge hashtable-ul si toate resursele alocate de acesta.
 *
 * @param ht hashtable-ul de eliberat
 */
void ht_destroy(hashtable *ht);

#endif /* HASHTABLE_H_ */
