/* Copyright 2023 Sima Alexandru (312CA) */
#ifndef LIST_H_
#define LIST_H_
#include <stddef.h>

typedef struct {
	void *key;
	void *data;
} dict_entry;

/**
 * @class list
 * @brief Nod al unei liste simplu inlantuite.
 */
typedef struct list {
	dict_entry info;   /**< Informatia stocata (perechi cheie, valoare) */
	struct list *next; /**< Referinta la urmatorul element din lista */
} list;

/**
 * @relates list
 * @brief Aloca un nod nou.
 *
 * @return	Adresa noului nod.
 */
list *list_create_node(void *key, void *value, size_t key_size,
					   size_t data_size);

/**
 * @relates list
 * @brief Insereaza nodul `node` la inceputul listei.
 */
void list_push(list **l, list *node);

/**
 * @relates list
 * @brief Cauta in lista un element cu o cheie data.
 *
 * @return		Referinta la *valoarea* asociata cheii.
 * @retval NULL	Nu exista un element cu acea cheie.
 */
void *list_get_item(list *l, void *key, size_t key_size);

/**
 * @relates list
 * @brief Extrage un element cu o anumita cheie, eliminandu-l din lista.
 *
 * @return		Nodul de lista care contine elementul cautat
 * @retval NULL Nu a fost gasit elementul cautat
 */
list *list_extract_item(list **l, void *key, size_t key_size);

/**
 * @relates list
 * @brief Imparte lista data in alte 2 liste, in functie de hashurile cheilor
 * elementelor.
 *
 * @param[in]	src			Lista originala
 * @param[out]	accepted	Lista cu elementele care au hashul cuprins intre
 *							`min_hash` si `max_hash`.
 * @param[out]	rejected	Lista cu elementele ramase.
 */
void list_split(list *src, list **accepted, list **rejected,
				unsigned int min_hash, unsigned int max_hash);

/**
 * @relates list
 * @brief Elibereaza lista si toate elementele pe care le contine.
 */
void list_destroy(list *l);

#endif /* LIST_H_ */
