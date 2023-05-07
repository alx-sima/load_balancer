/* Copyright 2023 Sima Alexandru (312CA) */
#include <limits.h>
#include <stdlib.h>
#include <string.h>

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
	struct server_entry *hashring;
	size_t hashring_capacity;
	size_t hashring_size; /**< Numarul de servere existente pe hashring. */
};

/**
 * @brief Returneaza labelul replicii `id` a labelului `nth`.
 */
static inline unsigned int get_nth_replica(unsigned int id, int nth)
{
	return nth * 1e5 + id;
}

/**
 * @brief Cauta in `hashring` o instanta de server cu un anumit hash.
 * @todo Se poate optimiza la cautare binara.
 *
 * @param hashring_size Dimensiunea hashringului
 * @param target_hash Hashul cautat
 *
 * @return O referinta la instanta de server
 * @return NULL daca serverul nu exista
 */
static struct server_entry *find_server(struct server_entry *hashring,
										size_t hashring_size,
										unsigned int target_hash)
{
	for (size_t i = 0; i < hashring_size; ++i) {
		if (hashring[i].hash == target_hash)
			return &hashring[i];
	}

	return NULL;
}

/**
 * @brief Returneaza serverul care poate contine un hash anume.
 *
 * @param hashring			Vectorul in care se cauta
 * @param hashring_size		Dimensiunea vectorului
 * @param target_hash		Hashul cautat
 */
static struct server_entry *containing_server(struct server_entry *hashring,
											  size_t hashring_size,
											  unsigned int target_hash)
{
	for (unsigned int i = 0; i < hashring_size; ++i) {
		if (target_hash < hashring[i].hash)
			return &hashring[i];
	}

	/* Pentru ca hashringul este circular, daca nu se gaseste un server cu un
	 * hash mai mare, obiectul va ajunge in primul server */
	return &hashring[0];
}

/**
 * @brief Compara 2 structuri `server_entry`.
 *
 * @param a	Pointer la prima valoare
 * @param b Pointer la a 2-a valoare
 *
 * @return	O valoare reprezentand ordinea dintre a si b, compatibila cu qsort
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

			struct server_entry *replica =
				find_server(main->hashring, main->hashring_size, replica_hash);
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
		containing_server(main->hashring, main->hashring_size, hash);
	*server_id = server->id;
	server_store(server->server, key, value);
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
	unsigned int hash = hash_function_key(key);

	struct server_entry *server =
		containing_server(main->hashring, main->hashring_size, hash);
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
			containing_server(main->hashring, server_count, hash);
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
			hash = 0xffffffff;
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
			find_server(main->hashring, main->hashring_size, hash);
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
		find_server(main->hashring, main->hashring_size, hash);

	for (int i = 0; i < REPLICA_NUM; ++i)
		transfer_items(neighbours[i].server, server->server, 0,
					   neighbours[i].hash);
	free_server_memory(server->server);

	for (int i = 0; i < REPLICA_NUM; ++i) {
		unsigned int label = get_nth_replica(server_id, i);
		unsigned int hash = hash_function_servers(&label);

		struct server_entry *server =
			find_server(main->hashring, main->hashring_size, hash);
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
