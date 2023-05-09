/* Copyright 2023 Sima Alexandru (312CA) */
#ifndef LIST_H_
#define LIST_H_
#include <stddef.h>

/**
 * @class dict_entry
 * @brief O pereche (cheie, valoare) stocata intr-un hashtable.
 */
typedef struct {
	/** cheia stocata */
	void *key;
	/** informatiile asociate cheii */
	void *data;
} dict_entry;

/**
 * @class list
 * @brief Nod al unei liste simplu inlantuite.
 */
typedef struct list {
	/** informatia stocata (perechi cheie, valoare) */
	dict_entry info;
	/** referinta la urmatorul element din lista */
	struct list *next;
} list;

/**
 * @relates list
 * @brief Aloca un nod nou.
 *
 * @param key 	cheia noului nod
 * @param value valoarea noului nod
 *
 * @return	adresa noului nod
 */
list *list_create_node(void *key, void *value);

/**
 * @relates list
 * @brief Insereaza nodul la inceputul listei.
 *
 * @param[in, out] 	l		lista
 * @param[in] 		node	nodul de inserat
 */
void list_push(list **l, list *node);

/**
 * @relates list
 * @brief Cauta in lista un element cu o cheie data.
 *
 * @param l 			lista in care se cauta
 * @param key 			cheia cautata
 * @param compare_func	functia de comparare a cheilor
 *
 * @return		valoarea asociata cheii
 * @retval NULL	nu exista un element cu acea cheie
 */
void *list_get_item(list *l, void *key,
					int (*compare_func)(void *key1, void *key2));

/**
 * @relates list
 * @brief Extrage un element cu o anumita cheie, eliminandu-l din lista.
 *
 * @param[in, out] 	l				lista
 * @param[in] 		key				cheia elementului cautat
 * @param[in] 		compare_func	functia de comparare a cheilor

 * @return		nodul de lista care contine elementul cautat
 * @retval NULL nu a fost gasit elementul cautat
 */
list *list_extract_item(list **l, void *key,
						int (*compare_func)(void *key1, void *key2));

/**
 * @relates list
 * @brief Imparte lista data in alte 2 liste, in functie de hashurile cheilor
 * elementelor.
 *
 * @param[in]	src			lista originala
 * @param[out]	accepted	lista cu elementele care au hashul cuprins intre
 *							`min_hash` si `max_hash`
 * @param[out]	rejected	lista cu elementele ramase
 * @param[in]	min_hash	valoarea minima a hashului
 * @param[in]	max_hash	valoarea maxima a hashului
 */
void list_split(list *src, list **accepted, list **rejected,
				unsigned int min_hash, unsigned int max_hash);

/**
 * @relates list
 * @brief Elibereaza lista si toate elementele pe care le contine.
 *
 * @param l					lista
 * @param destructor_func	functia de eliberare a datelor stocate
 */
void list_destroy(list *l, void (*destructor_func)(void *key, void *data));

#endif /* LIST_H_ */
