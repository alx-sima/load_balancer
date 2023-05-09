/* Copyright 2023 Sima Alexandru (312CA) */
#ifndef LIST_H_
#define LIST_H_
#include <stddef.h>

/**
 * @class dict_entry
 * @brief O pereche (cheie, valoare) stocata intr-un hashtable.
 */
typedef struct {
	/** Cheia stocata */
	void *key;
	/** Informatiile asociate cheii */
	void *data;
} dict_entry;

/**
 * @class list
 * @brief Nod al unei liste simplu inlantuite.
 */
typedef struct list {
	/** Informatia stocata (perechi cheie, valoare) */
	dict_entry info;
	/** Referinta la urmatorul element din lista */
	struct list *next;
} list;

/**
 * @relates list
 * @brief Aloca un nod nou.
 *
 * @param key 	Cheia noului nod
 * @param value Valoarea noului nod
 *
 * @return	Adresa noului nod
 */
list *list_create_node(void *key, void *value);

/**
 * @relates list
 * @brief Insereaza nodul la inceputul listei.
 *
 * @param[in, out] 	l		Adresa listei
 * @param[in] 		node	Nodul de inserat
 */
void list_push(list **l, list *node);

/**
 * @relates list
 * @brief Cauta in lista un element cu o cheie data.
 *
 * @param l 			Lista in care se cauta
 * @param key 			Cheia cautata
 * @param compare_func	Functia de comparare a cheilor
 *
 * @return		Valoarea asociata cheii
 * @retval NULL	Nu exista un element cu acea cheie
 */
void *list_get_item(list *l, void *key,
					int (*compare_func)(void *key1, void *key2));

/**
 * @relates list
 * @brief Extrage un element cu o anumita cheie, eliminandu-l din lista.
 *
 * @param[in, out] 	l				Adresa listei
 * @param[in] 		key				Cheia elementului cautat
 * @param[in] 		compare_func	Functia de comparare a cheilor

 * @return		Nodul de lista care contine elementul cautat
 * @retval NULL Nu a fost gasit elementul cautat
 */
list *list_extract_item(list **l, void *key,
						int (*compare_func)(void *key1, void *key2));

/**
 * @relates list
 * @brief Imparte lista data in alte 2 liste, in functie de hashurile cheilor
 * elementelor.
 *
 * @param[in]	src			Lista originala
 * @param[out]	accepted	Lista cu elementele care au hashul cuprins intre
 *							`min_hash` si `max_hash`
 * @param[out]	rejected	Lista cu elementele ramase
 * @param[in]	min_hash	Valoarea minima a hashului
 * @param[in]	max_hash	Valoarea maxima a hashului
 */
void list_split(list *src, list **accepted, list **rejected,
				unsigned int min_hash, unsigned int max_hash);

/**
 * @relates list
 * @brief Elibereaza lista si toate elementele pe care le contine.
 *
 * @param l					Adresa listei
 * @param destructor_func	Functia de eliberare a datelor stocate
 */
void list_destroy(list *l, void (*destructor_func)(void *key, void *data));

#endif /* LIST_H_ */
