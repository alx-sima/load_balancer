Sima Alexandru (312CA), mai 2023

# Tema 2 (SDA): Load Balancer

## Structura proiectului

- `hashtable`: Implementarea unui tabel de dispersie care poate reține orice
  tipuri de date.
- `list`: Implementarea unei liste simplu înlănțuite care reține perechi
  `(cheie, valoare)` (pentru bucketurile hashtable-ului).
- `load_balancer`: API-ul load_balancerului
- `server`: API-ul serverelor
- `utils`: funcții utilitare

---

## API

### Server

- `init_server_memory`: Instanțiază un nou server.
- `free_server_memory`: Eliberează resursele unui server.
- `server_store`: Adaugă un obiect în memorie.
- `server_remove`: Șterge un obiect din memorie.
- `server_retrieve`: Caută un obiect în memorie după cheie.
- `transfer_items`: Transferă între 2 servere obiectele cu anumite hash-uri.

### Load Balancer

- `init_load_balancer`: Inițializează un load balancer.
- `free_load_balancer`: Eliberează resursele alocate ale unui load balancer.
- `loader_store`: Adaugă un obiect în sistem.
- `loader_retrieve`: Caută un obiect în sistem.
- `loader_add_server`: Adaugă un server în sistem și i se atribuie obiecte
  din serverele vecine.
- `loader_remove_server`: Elimină un server din sistem și redistribuie
  obiectele pe care le stoca.

---

## Implementare

- Load balancerul este o entitate care conține mai multe servere și se asigură
  de distribuirea relativ uniformă a obiectelor între servere. Pentru aceasta se
  folosește algoritmul de _consistent_hashing_:

  - Fiecare server are asociat un id.
  - Pentru fiecare id se generează un număr de labeluri (3 în această
    implementare).
  - Se hash-uiesc labelurile. Funcția de hash este aleasă astfel încât
    etichetele unui server să fie relativ echidistante.
  - Se stochează serverele pe un hashring în ordine crescătoare după hash.
  - Obiectele revin serverului cu replica cu cel mai apropiat hash, mai mare
    decât hashul obiectului.

- Deoarece hashringul este ordonat, s-a utilizat algoritmul de _căutare binară_,
  fiind mai eficient decât căutarea liniară. Se ține totuși cont că vectorul
  este circular, așa că, dacă nu există nicio replică cu un hash mai mare decât
  cel căutat, acesta va reveni primului label.

- Adăugarea, căutarea și ștergerea unui obiect în sistem sunt, așadar, triviale,
  căutându-se serverul căruia îi este repartizat hashul și lucrându-se cu baza
  lui de date.

- Adăugarea unui server în sistem presupune:

  - extinderea hashringului (dacă este necesar);
  - calcularea labelurilor asociate;
  - găsirea poziției unde vor fi inserate în hashring;
  - preluarea obiectelor din baza de date a serverului vecin care devin mai
    aproapiate de labelul inserat;
  - inserarea labelului în hashring;

- Eliminarea unui server din sistem presupune:
  - găsirea vecinilor labelurilor asociate;
  - sortarea acestora dupa hash. Astfel, serverele cu hashuri mai mici vor avea
    prioritate, fiind mai apropiate de obiecte. Un caz special îl reprezintă
    primul label, când conține obiectele cu hash mai mare decât ultimul server.
    În acest caz, acesta va fi ultimul care preia obiecte.
  - transferul obiectelor catre vecini, în ordine;
  - ștergerea replicilor de pe hashring;
  - micșorarea hashringului (dacă este necesar).

---

## Remarci

Pentru că am subestimat cât de mult timp îmi va lua implementarea hashringului,
mi-am propus să nu rezolv tema pe calea obișnuită (într-un IDE cu
autocompletion, linting etc. - cine are nevoie de astea?😄), ci cum au vrut
strămoșii noștri să se rezolve o temă, cu `tmux`, `vim`, `grep`, `entr`, `make`
etc. Din experiența asta am învățat:

- multe configurații și comenzi de vim💪 pe care nu le știam (de ex. `C-w v/s`);
- niște utilitare care să mă ajute (de ex `ctags` sau `entr`);
- `vgdb`;
- `Doxygen`;
- că e greu fără să-ți arate IDE-ul documentația încontinuu;
- să nu subestimez temele la SDA;

... și, desigur, să implementez un hashtable și cum funcționează un load
balancer.
