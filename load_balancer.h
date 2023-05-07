/* Copyright 2023 Sima Alexandru (312CA) */
#ifndef LOAD_BALANCER_H_
#define LOAD_BALANCER_H_

#include "server.h"

/**
 * @class load_balancer
 * @brief Entitate care distribuie uniform obiecte
 * pe mai multe servere, folosind consistent hashing.
 */
struct load_balancer;
typedef struct load_balancer load_balancer;

/**
 * @relates load_balancer
 * @brief Aloca si initializeaza un load balancer.
 *
 * @return Referinta la nodul alocat
 */
load_balancer *init_load_balancer();

/**
 * @relates load_balancer
 * @brief Elibereaza load balancerul si toate serverele de pe acesta.
 *
 * @param main Load balancerul care este eliberat
 */
void free_load_balancer(load_balancer *main);

/**
 * @relates load_balancer
 * @brief Stocheaza o valoare la o anumita cheie in sistem.
 *
 * @param main				Load balancerul in care se stocheaza
 * @param key				Cheia la care se stocheaza
 * @param value				Valoarea stocata
 * @param server_id[out]	Id-ul serverului pe care a fost stocata valoarea
 */
void loader_store(load_balancer *main, char *key, char *value, int *server_id);

/**
 * @relates load_balancer
 * @brief Intoarce valoarea stocata pe hashring.
 *
 * @param main				Load balancerul pe care se cauta cheia
 * @param key				Cheia cautata
 * @param server_id[out]	Serverul pe care se afla cheia cautata
 *
 * @return		Valoarea stocata la cheia `key`
 * @retval NULL Cheia nu exista in sistem
 */
char *loader_retrieve(load_balancer *main, char *key, int *server_id);

/**
 * @relates load_balancer
 * @brief Adauga un nou server in load balancer, redistribuind elementele
 * serverelor vecine pe hashring.
 *
 * @param main		Load balancerul
 * @param server_id	Id-ul serverului de adaugat
 */
void loader_add_server(load_balancer *main, int server_id);

/**
 * @relates load_balancer
 * @brief Sterge un server din load balancer, redistribuindu-i elementele pe
 * hashring.
 *
 * @param main		Load balancerul
 * @param server_id	ID-ul serverului care trebuie sters
 */
void loader_remove_server(load_balancer *main, int server_id);

#endif /* LOAD_BALANCER_H_ */
