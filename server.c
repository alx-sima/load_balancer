/* Copyright 2023 Sima Alexandru (312CA) */
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "server.h"
#include "utils.h"

#define BUCKET_NO 421
#define KEY_LENGTH 128	   // FIXME
#define VALUE_LENGTH 65536 // FIXME

struct server_memory {
	hashtable *database;
};

server_memory *init_server_memory()
{
	struct server_memory *server = malloc(sizeof(struct server_memory));
	DIE(!server, "failed malloc() of server_memory");

	server->database =
		ht_create(BUCKET_NO, KEY_LENGTH, VALUE_LENGTH, hash_function_key);
	DIE(!server->database, "failed malloc() of server_memory.database");
	return server;
}

void server_store(server_memory *server, char *key, char *value)
{
	ht_store_item(server->database, key, value);
}

char *server_retrieve(server_memory *server, char *key)
{
	return ht_get_item(server->database, key);
}

void server_remove(server_memory *server, char *key)
{
	ht_delete_item(server->database, key);
}

void free_server_memory(server_memory *server)
{
	ht_destroy(server->database);
	free(server);
}

void transfer_items(server_memory *dest, server_memory *src,
					unsigned int min_hash, unsigned int max_hash)
{
	ht_transfer_items(dest->database, src->database, min_hash, max_hash);
	return;
	list *chain = ht_chain_entries(src->database, max_hash);
	while (chain) {
		ht_store_item(dest->database, chain->info.key, chain->info.data);
		list *old_chain = chain;
		chain = chain->next;

		free(old_chain->info.data);
		free(old_chain->info.key);
		free(old_chain);
	}
	return;
}
