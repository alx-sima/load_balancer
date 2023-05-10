/* Copyright 2023 Sima Alexandru (312CA) */
#ifndef HASHRING_H_
#define HASHRING_H_
#include <stdbool.h>
#include <stddef.h>

#include "server.h"

/**
 * @class hashring_entry
 * @brief Un label de pe hashring, continand
 * informatii despre serverul de care apartine.
 */
typedef struct {
	/** id-ul serverului */
	int id;
	/** hash-ul labelului */
	unsigned int hash;
	/** labelul intrarii */
	unsigned int label;

	/** serverul la care se face referinta */
	server_memory *server;
} hashring_entry;

/**
 * @brief Compara 2 structuri `server_entry`.
 *
 * @param a	pointer la prima valoare
 * @param b pointer la a 2-a valoare
 *
 * @retval -1	`a` < `b`
 * @retval	1	`a` > `b`
 */
int compare_servers(const void *a, const void *b);

/**
 * @brief Cauta un server cu un anumit hash/caruia ii
 * este repartizat un anumit hash.
 *
 * @param hashring				hashringul pe care se executa operatia
 * @param hashring_size			dimensiunea hashringului
 * @param target_hash			hashul cautat
 * @param search_containing		daca este setat, nu se cauta un hash al unui
 *								server, ci serverul care poate stoca acel hash
 *
 * @return		serverul gasit
 * @retval NULL	u exista un server cu acel hash
 */
hashring_entry *find_server(hashring_entry *hashring, size_t hashring_size,
							unsigned int target_hash, bool search_containing);

#endif /* HASHRING_H_ */
