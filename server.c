/* Copyright 2023 Sima Alexandru (312CA) */
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "server.h"
#include "utils.h"

// FIXME
#define KEY_LENGTH 128
#define VALUE_LENGTH 65536

struct server_memory {
	hashtable *database;
};

server_memory *init_server_memory()
{
	struct server_memory *server = malloc(sizeof(struct server_memory));
	DIE(!server, "failed server malloc()\n");

	server->database = create_ht(10, KEY_LENGTH, VALUE_LENGTH, NULL);
	DIE(!server->database, "failed server database malloc()\n");
	return server;
}

void server_store(server_memory *server, char *key, char *value)
{
	insert_item_ht(server->database, key, value);
}

char *server_retrieve(server_memory *server, char *key)
{
	return get_item_ht(server->database, key);
}

void server_remove(server_memory *server, char *key)
{
	remove_item_ht(server->database, key);
}

void free_server_memory(server_memory *server)
{
	delete_ht(server->database);
	free(server);
}

void transfer_items(server_memory *dest, server_memory *src,
					unsigned int max_hash)
{
	for (unsigned int i = 0; i < max_hash && i < src->database->num_buckets;
		 ++i) {
		dict_entry *entry;
		while ((entry = pop_hash_entry(src->database, i))) {
			insert_item_ht(dest->database, entry->key, entry->data);
		}
	}
}
