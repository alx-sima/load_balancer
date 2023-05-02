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

struct server_entry {
	int id;
	unsigned int hash;
	unsigned int label;

	server_memory *server;
};

struct server_info {
	unsigned int indexes[REPLICA_NUM];
	server_memory *server_addr;
};

struct load_balancer {
	struct server_entry *hashring;
	size_t hashring_capacity;
	/** Numarul de servere existente pe hashring. */
	size_t hashring_size;

	/**
	 * !!! DEPRECATED !!!
	 * Un hashtable care tine mapari intre id-ul
	 * unui server si informatii despre acesta.
	 *
	 * Maparea este de tipul: id -> server_info
	 */
	hashtable *servers_info;
};

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

enum order {
	LOWER,
	HIGHER,
	EQUAL,
};

enum order compare_servers(struct server_entry a, struct server_entry b)
{
	if (a.hash != b.hash)
		return a.hash < b.hash ? LOWER : HIGHER;

	if (a.label != b.label)
		return a.label < b.label ? LOWER : HIGHER;
	return EQUAL;
}

/**
 * DEPRECATED: linear search pana iese tema, optimizari dupa
 */
size_t search_index(struct server_entry server, struct server_entry *array,
					size_t len)
{
	size_t left = 0;
	size_t right = len;

	while (left <= right) {
		size_t index = (left + right) / 2;
		if (index >= len)
			return len;

		switch (compare_servers(server, array[index])) {
		case LOWER:
			if (!index)
				return 0;
			right = index - 1;
			break;
		case HIGHER:
			left = index + 1;
			break;
		case EQUAL:
			return index;
		}
	}

	size_t prev_index = (left + right) / 2;
	if (compare_servers(server, array[prev_index]) == LOWER)
		return prev_index;
	return prev_index + 1;
}

void update_hashring_position(load_balancer *main, unsigned int old_pos,
							  unsigned int new_pos,
							  struct server_info *new_labels, int labels_no)
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
		int label = i * 1e5 + server_id;
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
	struct server_info *info = ht_clone_val(main->servers_info, &server_id);
	ht_delete_item(main->servers_info, &server_id);

	for (int i = REPLICA_NUM; i > 0; --i) {
		int index = info->indexes[i - 1];
		unsigned int server_hash = main->hashring[index].hash;

		for (size_t j = index + 1; j < main->hashring_size; ++j)
			update_hashring_position(main, j, j - 1, info, i);

		size_t next_server_index = index % main->hashring_size--;
		unsigned int next_server_id = main->hashring[next_server_index].id;
		struct server_info *next_server_info =
			ht_get_item(main->servers_info, &next_server_id);
		if (!next_server_info) {
			// TODO
			continue;
		}
		transfer_items(next_server_info->server_addr, info->server_addr,
					   server_hash);
	}
	free_server_memory(info->server_addr);
	free(info);
}

struct server_entry *containing_server(struct server_entry *hashring,
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

/**
 * @brief Cauta in `hashring` o instanta de server cu un anumit hash.
 * @todo Se poate optimiza la cautare binara.
 *
 * @param hashring_size Dimensiunea hashringului
 * @param target_hash Hashul cautat
 * @return O referinta la instanta de server
 * @return NULL daca serverul nu exista
 */
struct server_entry *find_server(struct server_entry *hashring,
								 size_t hashring_size, unsigned int target_hash)
{
	for (size_t i = 0; i < hashring_size; ++i) {
		if (hashring[i].hash == target_hash)
			return &hashring[i];
	}

	return NULL;
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
			int replica_label = j * 1e5 + curr_entry->id;
			unsigned int replica_hash = hash_function_servers(&replica_label);

			struct server_entry *replica =
				find_server(main->hashring, main->hashring_size, replica_hash);
			DIE(!replica, "this shouldn't have happened");
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
