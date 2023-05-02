/* Copyright 2023 Sima Alexandru (312CA) */
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "load_balancer.h"
#include "server.h"
#include "utils.h"

#define BUCKET_NO 123

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

/**
 * @todo DEPRECATED Va fi scos odata cu load_balancer->servers_info
 */
struct server_info {
	unsigned int indexes[REPLICA_NUM];
	server_memory *server_addr;
};

struct load_balancer {
	struct server_entry *hashring;
	size_t hashring_capacity;
	size_t hashring_size; /**< Numarul de servere existente pe hashring. */

	/**
	 * !!! DEPRECATED !!!
	 * Un hashtable care tine mapari intre id-ul
	 * unui server si informatii despre acesta.
	 *
	 * Maparea este de tipul: id -> server_info
	 */
	hashtable *servers_info;
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
 * @brief Compara 2 struct server_entry* (@todo struct server_entry).
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

/**
 * @todo DEPRECATED: linear search pana iese tema, optimizari dupa
 */
static size_t search_index(struct server_entry server,
						   struct server_entry *array, size_t len)
{
	size_t left = 0;
	size_t right = len;

	while (left <= right) {
		size_t index = (left + right) / 2;
		if (index >= len)
			return len;

		int order = compare_servers(&server, &array[index]);
		if (order == 0)
			return index;

		if (order < 0) {
			if (!index)
				return 0;
			right = index - 1;
		} else {
			left = index + 1;
		}
	}

	size_t prev_index = (left + right) / 2;
	if (compare_servers(&server, &array[prev_index]) < 0)
		return prev_index;
	return prev_index + 1;
}

/**
 * @todo verifica daca e folosit dupa loader_add_server refactor
 */
static void update_hashring_position(load_balancer *main, unsigned int old_pos,
									 unsigned int new_pos,
									 struct server_info *new_labels,
									 int labels_no)
{
	struct server_info *old_server_info =
		ht_get_item(main->servers_info, &main->hashring[old_pos].id);
	main->hashring[new_pos] = main->hashring[old_pos];

	unsigned int *labels;
	int labels_num;
	if (old_server_info) {
		labels_num = REPLICA_NUM;
		labels = old_server_info->indexes;
	} else {
		/* Serverul nu este in baza de date, deci este o instanta a celui creat
		 * acum, asa ca se schimba labelurile din `new_labels` */
		labels_num = labels_no;
		labels = new_labels->indexes;
	}

	for (int i = 0; i < labels_num; ++i) {
		if (labels[i] == old_pos)
			labels[i] = new_pos;
	}
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

load_balancer *init_load_balancer()
{
	load_balancer *lb = malloc(sizeof(load_balancer));
	DIE(!lb, "failed malloc() of load_balancer");

	lb->hashring_capacity = REPLICA_NUM;
	lb->hashring_size = 0;
	lb->hashring = calloc(lb->hashring_capacity, sizeof(struct server_entry));
	DIE(!lb->hashring, "failed malloc() of load_balancer.hashring");

	lb->servers_info =
		ht_create(BUCKET_NO, sizeof(unsigned int), sizeof(struct server_info),
				  hash_function_servers);
	return lb;
}

void free_load_balancer(load_balancer *main)
{
	for (size_t i = 0; i < main->hashring_size; ++i) {
		struct server_entry *curr_entry = &main->hashring[i];
		fprintf(stderr, "index %lu: ptr %p\n", i, curr_entry->server);
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
			fprintf(stderr,
					"index %lu, replica %d, replica_index %lu: ptr %p\n", i, j,
					replica - main->hashring, replica->server);

			replica->server = NULL;
		}
	}

	ht_destroy(main->servers_info); /* TODO: temporary fix */
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
	struct server_info new_server_info = {
		.server_addr = init_server_memory(),
	};

	size_t server_count = main->hashring_size;
	main->hashring_size += REPLICA_NUM;
	if (main->hashring_size > main->hashring_capacity) {
		main->hashring_capacity *= REALLOC_FACTOR;
		main->hashring = realloc(main->hashring, sizeof(struct server_entry) *
													 main->hashring_capacity);
		DIE(!main->hashring, "failed realloc() of load_balancer.hashring");
	}
	for (int i = 0; i < REPLICA_NUM; ++i) {
		unsigned int label = get_nth_replica(server_id, i);
		unsigned int hash = hash_function_servers(&label);

		struct server_entry new_label = {
			.id = server_id,
			.hash = hash,
			.label = label,
			.server = new_server_info.server_addr,
		};

		size_t index = search_index(new_label, main->hashring, server_count);
		for (size_t j = server_count; j > index; --j)
			update_hashring_position(main, j - 1, j, &new_server_info, i);
		main->hashring[index] = new_label;

		size_t next_server_index = (index + 1) % ++server_count;
		unsigned int next_server_id = main->hashring[next_server_index].id;
		struct server_info *next_server_info =
			ht_get_item(main->servers_info, &next_server_id);

		/* Daca `main` era gol inaintea apelarii functiei, nu vor exista alte
		 * servere, asa ca nu exista obiecte de transferat. */
		if (next_server_info) {
			transfer_items(new_server_info.server_addr,
						   next_server_info->server_addr, hash);
		}
		new_server_info.indexes[i] = index;
	}

	ht_store_item(main->servers_info, &server_id, &new_server_info);
	for (size_t i = 0; i < main->hashring_size; ++i) {
		fprintf(stderr, "%u ", main->hashring[i].hash);
	}
	fprintf(stderr, "\n");
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

		size_t index = server_replica - main->hashring;
		while (index < main->hashring_size - 1 &&
			   main->hashring[index + 1].id == server_id)
			++index;

		/* Daca replica e ultimul element din hashring, elementele care raman
		 * vor fi preluate de primul server, indiferent de hash. */
		if (index == main->hashring_size - 1) {
			memcpy(&neighbours[i], &main->hashring[0],
				   sizeof(struct server_entry));
			neighbours[i].hash = 0xffffffff; /* TODO: constanta */
		} else {
			memcpy(&neighbours[i], &main->hashring[index + 1],
				   sizeof(struct server_entry));
		}
	}

	qsort(neighbours, REPLICA_NUM, sizeof(struct server_entry),
		  compare_servers);

	unsigned int hash = hash_function_servers(&server_id);
	struct server_entry *server =
		find_server(main->hashring, main->hashring_size, hash);

	list *transferred_item;
	while ((transferred_item = server_pop_entry(server->server))) {
		unsigned int item_hash = hash_function_key(transferred_item->info->key);
		for (int i = 0; i < REPLICA_NUM; ++i) {
			if (item_hash < neighbours[i].hash) {
				server_store(neighbours[i].server, transferred_item->info->key,
							 transferred_item->info->data);
				break;
			}
		}
		free(transferred_item->info->key);
		free(transferred_item->info->data);
		free(transferred_item->info);
		free(transferred_item);
	}

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
}
