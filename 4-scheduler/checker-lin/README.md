Nume: Toma Andrei
Grupă: 331CB

# Tema 4 Planificator de threaduri

## Organizare

Pentru implementare m-am folosit de 3 structuri (`TScheduler`, `TThread` si
`TNode`).

Structura `TThread`:
* handler - functia care va fi apelata de thread cand acesta este pus in
starea `RUNNING`;
* priority - prioritatea threadului;
* time_quantum_left - cuanta ramasa de timp;
* thread_id - id-ul threadului generat la apelul `pthread_create`;
* planned_semaphore - semafor pentru ca functia `so_fork` sa returneze id-ul
threadului cand acesta a intrat in starea `READY` sau `RUNNING`;
* running_semaphore - semafor ca thread-ul sa astepte pana intra in starea
`RUNNING`.

Structura `TNode` (lista simplu inlantuita folosita pentru implementarea
cozii de prioritati):
* thread - informatia din nodul curent;
* next - referinta catre urmatorul element.

Structura `TScheduler`:
* time_quantum - cuanta de timp care va fi atribuita thread-ului la crearea
sa, dar si cand acesta este preemptat;
* io - numarul maxim de evenimente io care vor fi facute pe scheduler;
* thread_counter - numarul de threaduri care mai au de executat (se afla in
starea `READY` sau `RUNNING`);
* ready_threads_queue - coada de prioritati folosita pentru threadurile
aflate in starea `READY`, acestea vor fi puse in coada dupa prioritate;
* waiting_threads_queue - o lista de liste care va functiona similar cu o
tabela hash. Pentru fiecare intrare `io` posibila va fi alocata o lista;
* terminated_threads - coada care va contine threadurile care si-au
terminat executia, in aceasta coada nu se va tine cont de prioritati;
* running_thread - threadul care ruleaza in momentul curent;
* stop_sem - folosit pentru a semnala faptul ca toate threadurile si-au
terminat executia si ca se pot face dezalocarile in `so_end`.

Ca implementare generala se aloca o structura pentru un thread nou si dupa
se apeleaza o functie auxiliara `start_thread` care va planifica threadul.
Dupa ce acesta este planificat se va semnala semaforul care asteapta ca
threadul sa fie planificat din `so_fork`. Dupa aceea va astepta la un
semafor care ii va spune cand sa intre si sa execute in handler.

Tema a fost utila in ceea ce tine a intelege mai bine primitivele de
sincronziare, in special semafoarele.

## Implementare

Enuntul temei este implementat cu exceptia functioanlitatii de IO cu
prioritati.
Am intampinat dificultati cu sincronizarea (multe deadlock-uri).

***so_exec()***
La apelul functiei `so_exec` doar va trece timp pe procesor. Cuanta de timp
ramasa pentru thread este decrementata si cand se va ajunge la 0 threadul
este preemptat si threadul cu cea mai mare prioritate din `READY` va fi pus
ca noul running thread. Cand noul thread este pus sa ruleze se va face `post` 
semaforul sau de running si pe semaforul threadului care a fost preemptat se
va face `wait`.

***Evenimete IO***
Pentru evenimente IO se va folosi lista de liste `waiting_threads_queue`.
Cand un thread apeleaza `so_wait` pe un eveniment cu id-ul `io` thread-ul
din `RUNNING` va fi adaugat in coada `waiting_threads_queue[io]` si se va
face wait pe semaforul de `RUNNING` al threadului curent.

Cand se face `so_signal` pe un eveniment cu id-ul `io` thread-urile adaugate
in coada respectiva evenimentului pana in momentul respectiv se vor scoate
din coada si se vor adauga in coada `READY` pentru a urma sa fie executate.

## Cum se compilează și cum se rulează?
* Linux - make (va genera o bibliotecă partajată)

## Bibliografie
* https://ocw.cs.pub.ro/courses/so/teme/tema-4
* https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-08
