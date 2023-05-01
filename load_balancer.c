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

struct server {
	unsigned int hash;
	int label;

	server_memory *server;
};

struct server_metadata {
	unsigned int indexes[REPLICA_NUM];
};

struct load_balancer {
	struct server *hashring;
	size_t hashring_capacity;
	size_t hashring_size;

	hashtable *server_metadatas;
};

unsigned int hash_function_servers(void *a)
{
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int hash_function_key(void *a)
{
	unsigned char *puchar_a = (unsigned char *)a;
	unsigned int hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

unsigned int hash_function_ptr(void *a)
{
	void *ptr = *((void **)a);
	// TODO
	return (unsigned int)ptr;
}

load_balancer *init_load_balancer()
{
	load_balancer *lb = malloc(sizeof(load_balancer));
	DIE(!lb, ""); // TODO

	lb->hashring_capacity = REPLICA_NUM;
	lb->hashring_size = 0;
	lb->hashring = calloc(lb->hashring_capacity, sizeof(struct server));
	DIE(!lb->hashring, ""); // TODO

	lb->server_metadatas =
		ht_create(BUCKET_NO, sizeof(struct server *),
				  sizeof(struct server_metadata), hash_function_ptr);
	return lb;
}

enum order {
	LOWER,
	HIGHER,
	EQUAL,
};

enum order compare_servers(struct server a, struct server b)
{
	if (a.hash != b.hash)
		return a.hash < b.hash ? LOWER : HIGHER;

	if (a.label != b.label)
		return a.label < b.label ? LOWER : HIGHER;
	return EQUAL;
}

size_t search_index(struct server server, struct server *array, size_t len)
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
							  struct server_metadata *new_labels, int labels_no)
{
	struct server_metadata *metadata =
		ht_get_item(main->server_metadatas, &main->hashring[old_pos].server);
	main->hashring[new_pos] = main->hashring[old_pos];

	unsigned int *labels;
	int labels_num;
	if (metadata) {
		labels_num = REPLICA_NUM;
		labels = metadata->indexes;
	} else {
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
	server_memory *new_server = init_server_memory();

	size_t server_count = main->hashring_size;
	main->hashring_size += REPLICA_NUM;
	if (main->hashring_size > main->hashring_capacity) {
		main->hashring_capacity *= REALLOC_FACTOR;
		main->hashring = realloc(main->hashring, sizeof(struct server) *
													 main->hashring_capacity);
		DIE(!main->hashring, ""); // TODO
		memset(main->hashring + main->hashring_size, 0,
			   main->hashring_capacity - main->hashring_size);
	}
	struct server_metadata new_metadata;
	for (int i = 0; i < REPLICA_NUM; ++i) {
		int label = i * 1e5 + server_id;
		unsigned int hash = hash_function_servers(&label);

		struct server new_label = {
			.hash = hash,
			.label = label,
			.server = new_server,
		};

		size_t index = search_index(new_label, main->hashring, server_count);
		for (size_t j = server_count; j > index; --j)
			update_hashring_position(main, j - 1, j, &new_metadata, i);
		main->hashring[index] = new_label;

		size_t next_server = (index + 1) % ++server_count;
		transfer_items(main->hashring[index].server,
					   main->hashring[next_server].server,
					   main->hashring[index].hash);
		new_metadata.indexes[i] = index;
	}
	ht_store_item(main->server_metadatas, &new_server, &new_metadata);
}

void loader_remove_server(load_balancer *main, int server_id)
{
	/* TODO 3 */
	(void)main;
	(void)server_id;
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
	/* TODO 4 */
	(void)main;
	(void)key;
	(void)value;
	(void)server_id;
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
	/* TODO 5 */
	(void)main;
	(void)key;
	(void)server_id;
	return NULL;
}

void free_load_balancer(load_balancer *main)
{
	/* Vector de frecventa in care se retine daca s-a sters serverul deja. */
	char *deleted_servers = calloc(main->hashring_size, sizeof(char));

	for (size_t i = 0; i < main->hashring_size; ++i) {
		if (deleted_servers[i])
			continue;

		server_memory *server_ptr = main->hashring[i].server;
		struct server_metadata *metadata =
			ht_get_item(main->server_metadatas, &server_ptr);
		unsigned int *labels = metadata->indexes;

		/* Marcheaza toate instantele serverului ca fiind sterse. */
		for (int j = 0; j < REPLICA_NUM; ++j)
			deleted_servers[labels[j]] = 1;

		free_server_memory(server_ptr);
	}

	free(deleted_servers);

	ht_destroy(main->server_metadatas);
	free(main->hashring);
	free(main);
}
