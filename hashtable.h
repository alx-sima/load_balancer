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
	/** Numarul de bucketuri pe care le are hashtableul */
	unsigned int num_buckets;
	/** Bucketurile hashtableului */
	list **buckets;

	/** Functia de hash a cheilor */
	unsigned int (*hash_func)(void *key);
	/** Functia de comparare a cheilor */
	int (*compare_func)(void *key1, void *key2);
	/** Functia de eliberare a cheilor si valorilor */
	void (*destructor_func)(void *key, void *data);
} hashtable;

/**
 * @relates hashtable
 * @brief Calculeaza hashul cheii, folosind functia corespunzatoare
 * hashtable-ului.
 *
 * @param ht 	Hashtable-ul in care se calculeaza hashul
 * @param key 	Cheia pentru care se calculeaza hashul
 */
unsigned int ht_compute_hash(hashtable *ht, void *key);

/**
 * @relates hashtable
 * @brief Creeaza si initializaeaza un hashtable.
 *
 * @param num_buckets 		Numarul de bucketuri ale hashtable-ului
 * @param hash_func 		Functia de hash a cheilor
 * @param compare_func 		Functia de comparare a cheilor
 * @param destructor_func	Functia de eliberare a cheilor si valorilor

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
 * @param ht 	Hashtable-ul in care se insereaza
 * @param key 	Cheia la care se face insertia
 * @param value	Valoarea inserata
 */
void ht_store_item(hashtable *ht, void *key, void *value);

/**
 * @relates hashtable
 * @brief Sterge din hashtable elementul cu cheia data.
 *
 * @param ht 	Hashtable-ul din care se sterge
 * @param key 	Cheia elementului de sters
 */
void ht_delete_item(hashtable *ht, void *key);

/**
 * @relates hashtable
 * @brief Returneaza informatia stocata la cheia data.
 *
 * @param ht 	Hashtable-ul din care se cauta
 * @param key 	Cheia cautata
 */
void *ht_get_item(hashtable *ht, void *key);

/**
 * @relates hashtable
 * @brief Alcatuieste o lista inlantuita cu perechile (cheie, valoare) care au
 * hashul mai mic decat o valoare data, stergandu-le din hashtable.
 *
 * @param ht 		Hashtable-ul din care se extrag perechile
 * @param max_hash 	Valoarea maxima a hashului
 *
 * @return Inceputul listei simplu inlantuite cu perechi.
 */
list *ht_chain_entries(hashtable *ht, unsigned int max_hash);

/**
 * @relates hashtable
 * @brief Sterge hashtableul si toate resursele alocate de acesta.
 *
 * @param ht Hashtable-ul de eliberat
 */
void ht_destroy(hashtable *ht);

/**
 * @relates hashtable
 * @brief Transfera obiectele stocate dintr-un hashtable in altul, in functie
 * de hashul acsetora.
 *
 * @param dest		Hashtableul destinatie
 * @param src		Hashtableul original
 * @param min_hash	hashul minim pe care il au obiectele transferate
 * @param max_hash	hashul maxim pe care il au obiectele transferate
 */
void ht_transfer_items(hashtable *dest, hashtable *src, unsigned int min_hash,
					   unsigned int max_hash);

#endif /* HASHTABLE_H_ */
