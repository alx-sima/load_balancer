/* Copyright 2023 Sima Alexandru (312CA) */
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

#include "hashtable.h"
#include "load_balancer.h"
#include "server.h"
#include "utils.h"

/** De cate ori e replicat fiecare server */
#define REPLICA_NUM 3

/** Pragul de umplere/golire la care se redimensioneaza hashringul */
#define REALLOC_FACTOR 2

/**
 * @brief Un label de server pe hashring.
 */
struct server_entry {
	int id;
	unsigned int hash;
	unsigned int label;

	server_memory *server;
};

struct load_balancer {
	/** vector circular care retine etichetele
		asociate serverelor din load balancer */
	struct server_entry *hashring;
	/** dimensiunea alocata a hashringului */
	size_t hashring_capacity;
	/** numarul de servere existente pe hashring */
	size_t hashring_size;
};

/**
 * @brief Calculeaza labelul replicii `id` a labelului `nth`.
 */
static inline unsigned int get_nth_replica(unsigned int id, int nth)
{
	return nth * 1e5 + id;
}

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
static struct server_entry *find_server(struct server_entry *hashring,
										size_t hashring_size,
										unsigned int target_hash,
										int search_containing)
{
	size_t left = 0;
	size_t right = hashring_size - 1;

	while (left <= right) {
		size_t index = (left + right) / 2;
		unsigned int hash = hash_function_servers(&hashring[index].label);

		if (hash == target_hash)
			return &hashring[index];

		if (hash < target_hash) {
			left = index + 1;
			continue;
		}

		right = index - 1;
		if (search_containing) {
			if (index == 0)
				return &hashring[0];

			unsigned int previous_hash =
				hash_function_servers(&hashring[index - 1].label);

			/* Hash-ul este cuprins intre cel al serverului
			 * curent si hashul serverului anterior, deci
			 * elementul apartine serverului curent. */
			if (previous_hash < target_hash)
				return &hashring[index];
		}
	}

	/* Daca nu a fost gasit un server care sa contina hashul
	 * (valabil pentru `search_containing == 1`),
	 * elementul revine primului server. */
	return search_containing ? &hashring[0] : NULL;
}

/**
 * @brief Compara 2 structuri `server_entry`.
 *
 * @param a	pointer la prima valoare
 * @param b pointer la a 2-a valoare
 *
 * @retval -1	`a` < `b`
 * @retval	1	`a` > `b`
 */
static int compare_servers(const void *a, const void *b)
{
	const struct server_entry *a_cast = a;
	const struct server_entry *b_cast = b;

	if (a_cast->hash != b_cast->hash)
		return a_cast->hash < b_cast->hash ? -1 : 1;
	return a_cast->label < b_cast->label ? -1 : 1;
}

load_balancer *init_load_balancer()
{
	load_balancer *lb = malloc(sizeof(load_balancer));
	DIE(!lb, "failed malloc() of load_balancer");

	lb->hashring_capacity = REPLICA_NUM;
	lb->hashring_size = 0;

	lb->hashring = calloc(lb->hashring_capacity, sizeof(struct server_entry));
	DIE(!lb->hashring, "failed malloc() of load_balancer.hashring");

	return lb;
}

void free_load_balancer(load_balancer *main)
{
	for (size_t i = 0; i < main->hashring_size; ++i) {
		struct server_entry *curr_entry = &main->hashring[i];
		/* Serverul a fost deja sters */
		if (!curr_entry->server)
			continue;

		free_server_memory(curr_entry->server);
		curr_entry->server = NULL;

		/* Seteaza si celelalte replici ca fiind sterse */
		for (int j = 0; j < REPLICA_NUM; ++j) {
			unsigned int replica_label = get_nth_replica(curr_entry->id, j);
			unsigned int replica_hash = hash_function_servers(&replica_label);

			struct server_entry *replica = find_server(
				main->hashring, main->hashring_size, replica_hash, false);
			DIE(!replica, "oops, this shouldn't have happened!");

			replica->server = NULL;
		}
	}

	free(main->hashring);
	free(main);
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
	unsigned int hash = hash_function_key(key);

	struct server_entry *server =
		find_server(main->hashring, main->hashring_size, hash, true);
	*server_id = server->id;
	server_store(server->server, key, value);
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
	unsigned int hash = hash_function_key(key);

	struct server_entry *server =
		find_server(main->hashring, main->hashring_size, hash, true);
	*server_id = server->id;
	return server_retrieve(server->server, key);
}

void loader_add_server(load_balancer *main, int server_id)
{
	server_memory *server = init_server_memory();

	size_t server_count = main->hashring_size;
	main->hashring_size += REPLICA_NUM;
	if (main->hashring_size > main->hashring_capacity) {
		main->hashring_capacity *= REALLOC_FACTOR;
		main->hashring = realloc(main->hashring, sizeof(struct server_entry) *
													 main->hashring_capacity);
		DIE(!main->hashring,
			"failed realloc() (extending) of load_balancer.hashring");
	}
	for (int i = 0; i < REPLICA_NUM; ++i) {
		unsigned int label = get_nth_replica(server_id, i);
		unsigned int hash = hash_function_servers(&label);

		struct server_entry new_label = {
			.id = server_id,
			.hash = hash,
			.label = label,
			.server = server,
		};

		if (server_count == 0) {
			main->hashring[0] = new_label;
			++server_count;
			continue;
		}

		struct server_entry *neighbor =
			find_server(main->hashring, server_count, hash, true);
		size_t index = neighbor - main->hashring;
		unsigned int min_hash;

		if (index == 0) {
			/* Chiar daca noul label preia obiecte de la primul
			 * server, acesta e de fapt ultimul pe hashring */
			if (main->hashring[0].hash < hash) {
				min_hash = main->hashring[server_count - 1].hash;
				main->hashring[server_count++] = new_label;
				transfer_items(server, neighbor->server, min_hash, hash);
				continue;
			}

			transfer_items(server, neighbor->server, 0, hash);
			/* Devenind primul element de pe hashring, acesta va prelua toate
			 * elementele care se aflau in primul server pentru ca aveau hashul
			 * mai mare decat ultimul server. */
			min_hash = main->hashring[server_count - 1].hash;
			hash = UINT_MAX;
		} else {
			min_hash = main->hashring[index - 1].hash;
		}

		transfer_items(server, neighbor->server, min_hash, hash);

		for (size_t j = server_count++; j > index; --j)
			main->hashring[j] = main->hashring[j - 1];

		main->hashring[index] = new_label;
	}
}

void loader_remove_server(load_balancer *main, int server_id)
{
	struct server_entry neighbours[REPLICA_NUM];
	for (int i = 0; i < REPLICA_NUM; ++i) {
		unsigned int label = get_nth_replica(server_id, i);
		unsigned int hash = hash_function_servers(&label);

		struct server_entry *server_replica =
			find_server(main->hashring, main->hashring_size, hash, false);
		DIE(!server_replica, "oops, this shouldn't have happened!");

		size_t neighbour_index = server_replica - main->hashring + 1;
		while (neighbour_index < main->hashring_size &&
			   main->hashring[neighbour_index].id == server_id)
			++neighbour_index;

		/* Daca replica e ultimul element din hashring, elementele care raman
		 * vor fi preluate de primul server diferit, indiferent de hash. */
		if (neighbour_index == main->hashring_size) {
			neighbour_index = 0;
			while (main->hashring[neighbour_index].id == server_id)
				++neighbour_index;

			neighbours[i] = main->hashring[neighbour_index];
			neighbours[i].hash = UINT_MAX;
		} else {
			neighbours[i] = main->hashring[neighbour_index];
		}
	}

	qsort(neighbours, REPLICA_NUM, sizeof(struct server_entry),
		  compare_servers);

	unsigned int hash = hash_function_servers(&server_id);
	struct server_entry *server =
		find_server(main->hashring, main->hashring_size, hash, false);

	for (int i = 0; i < REPLICA_NUM; ++i)
		transfer_items(neighbours[i].server, server->server, 0,
					   neighbours[i].hash);

	free_server_memory(server->server);

	for (int i = 0; i < REPLICA_NUM; ++i) {
		unsigned int label = get_nth_replica(server_id, i);
		unsigned int hash = hash_function_servers(&label);

		struct server_entry *server =
			find_server(main->hashring, main->hashring_size, hash, false);
		size_t index = server - main->hashring;

		--main->hashring_size;
		for (size_t j = index; j < main->hashring_size; ++j)
			main->hashring[j] = main->hashring[j + 1];
	}

	if (main->hashring_size < main->hashring_capacity / REALLOC_FACTOR) {
		main->hashring_capacity /= REALLOC_FACTOR;
		main->hashring = realloc(main->hashring, sizeof(struct server_entry) *
													 main->hashring_capacity);
		DIE(!main->hashring,
			"failed realloc() (shrinking) of load_balancer.hashring");
	}
}
