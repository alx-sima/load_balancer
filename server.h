/* Copyright 2023 Sima Alexandru (312CA) */
#ifndef SERVER_H_
#define SERVER_H_

/**
 * @class server_memory
 * @brief Server care poate memora, cauta si sterge obiecte.
 */
struct server_memory;
typedef struct server_memory server_memory;

/**
 * @relates server_memory
 * @brief Aloca si initializeaza un server.
 *
 * @return Un nou server initializat
 */
server_memory *init_server_memory();

/**
 * @relates server_memory
 * @brief Elibereaza memoria serverului, stergand toate obiectele continute.
 *
 * @param server Serverul care este eliberat
 */
void free_server_memory(server_memory *server);

/**
 * @relates server_memory
 * @brief Stocheaza pe server o pereche (cheie, valoare).
 *
 * @param server	Serverul pe care se executa operatia
 * @param key		Cheia stocata
 * @param value		Valoarea stocata
 */
void server_store(server_memory *server, char *key, char *value);

/**
 * @relates server_memory
 * @brief Sterge o pereche (cheie, valoare) de pe server.
 *
 * @param server	Serverul pe care se executa operatia
 * @param key		Cheia perechii sterse
 */
void server_remove(server_memory *server, char *key);

/**
 * @relates server_memory
 * @brief Returneaza valoarea stocata la o cheie.
 *
 * @param server	Serverul pe care se cauta cheia
 * @param key		Cheia cautata
 *
 * @return		Valoarea gasita
 * @retval NULL	Valoarea nu exista pe server
 */
char *server_retrieve(server_memory *server, char *key);

/**
 * @relates server_memory
 * @brief Transfera obiectele stocate in `src` care indeplinesc conditia
 * hashului pe serverul `dest`.
 *
 * @param dest		Serverul destinatie
 * @param src		Serverul original
 * @param min_hash	hashul minim pe care il au obiectele transferate
 * @param max_hash	hashul maxim pe care il au obiectele transferate
 */
void transfer_items(server_memory *dest, server_memory *src,
					unsigned int min_hash, unsigned int max_hash);

#endif /* SERVER_H_ */
