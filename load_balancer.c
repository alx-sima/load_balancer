/* Copyright 2023 Sima Alexandru (312CA) */
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "load_balancer.h"
#include "server.h"
#include "utils.h"

#define BUCKET_NO 123
#define REPLICA_NUM 3
#define REALLOC_FACTOR 2

struct server_entry {
	int id;
	unsigned int hash;
	unsigned int label;
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
			right = index - 1;
			break;
		case HIGHER:
			left = index + 1;
			break;
		case EQUAL:
			return index;
		}
	}

	return right;
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
		/** Serverul nu este in baza de date, deci este o instanta a celui creat
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
	// server_memory *new_server = init_server_memory();
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
		// TODO: S-ar putea sa nu fie necesar
		memset(main->hashring + main->hashring_size, 0,
			   main->hashring_capacity - main->hashring_size);
	}
	for (int i = 0; i < REPLICA_NUM; ++i) {
		int label = i * 1e5 + server_id;
		unsigned int hash = hash_function_servers(&label);

		struct server_entry new_label = {
			.id = server_id,
			.hash = hash,
			.label = label,
		};

		size_t index = search_index(new_label, main->hashring, server_count);
		for (size_t j = server_count; j > index; --j)
			update_hashring_position(main, j - 1, j, &new_server_info, i);
		main->hashring[index] = new_label;

		size_t next_server = (index + 1) % ++server_count;
		struct server_info *next_server_info =
			ht_get_item(main->servers_info, &next_server);

		/* Daca `main` era gol inaintea apelarii functiei, nu vor exista alte
		 * servere, asa ca nu exista obiecte de transferat. */
		if (next_server_info) {
			transfer_items(new_server_info.server_addr,
						   next_server_info->server_addr, hash);
		}
		new_server_info.indexes[i] = index;
	}

	ht_store_item(main->servers_info, &server_id, &new_server_info);
}

void loader_remove_server(load_balancer *main, int server_id)
{
	struct server_info *info = ht_get_item(main->servers_info, &server_id);
	free_server_memory(info->server_addr);

	for (int i = 0; i < REPLICA_NUM; ++i) {
		int index = info->indexes[i];
		for (size_t j = index + 1; j < main->hashring_size; ++j) 
			update_hashring_position(main, j, j - 1, NULL, 0);
		// TODO redistribuie itemele
		--main->hashring_size;
	}
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
	unsigned int hash = hash_function_key(key);

	// TODO: cautare binara
	for (unsigned int i = 0; i < main->hashring_size; ++i) {
		if (hash < main->hashring[i].hash) {
			*server_id = main->hashring[i].id;
			struct server_info *metadata =
				ht_get_item(main->servers_info, server_id);
			struct server_memory *addr = metadata->server_addr;
			server_store(addr, key, value);
			return;
		}
	}
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
	unsigned int hash = hash_function_key(key);

	// TODO: cautare binara
	for (unsigned int i = 0; i < main->hashring_size; ++i) {
		if (hash < main->hashring[i].hash) {
			*server_id = main->hashring[i].id;
			struct server_info *metadata =
				ht_get_item(main->servers_info, server_id);
			struct server_memory *addr = metadata->server_addr;
			return server_retrieve(addr, key);
		}
	}

	return NULL;
}

void free_load_balancer(load_balancer *main)
{
	/* Vector de frecventa in care se retine daca s-a sters serverul deja. */
	char *deleted_servers = calloc(main->hashring_size, sizeof(char));

	for (size_t i = 0; i < main->hashring_size; ++i) {
		if (deleted_servers[i])
			continue;

		int id = main->hashring[i].id;
		const struct server_info *metadata =
			ht_get_item(main->servers_info, &id);
		const unsigned int *labels = metadata->indexes;

		/* Marcheaza toate instantele serverului ca fiind sterse. */
		for (int j = 0; j < REPLICA_NUM; ++j)
			deleted_servers[labels[j]] = 1;

		free_server_memory(metadata->server_addr);
	}

	free(deleted_servers);

	ht_destroy(main->servers_info);
	free(main->hashring);
	free(main);
}
