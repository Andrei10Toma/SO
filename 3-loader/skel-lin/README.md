Nume: Toma Andrei
Grupă: 331CB

# Tema 3 Loader de Executabile

## Organizare

M-am folosit de structurile date din schelet (`so_exec_t` şi `so_seg_t`). Pe
lângă aceste 2 structuri am folosit şi 2 variabile statice:
* fd - file descriptorului executabilului deschis din funcţia `so_execute`
* old_action - folosit pentru a reţine handler-ul default

M-am folosit de câmpul `data` pentru a reţine dacă o pagină din segment este
deja mapată. Am alocat array-ul cu `segment->mem_size / PAGE_SIZE` elemente
de tip char. `data[i]` va fi setat pe 1 dacă pagina i din segment este deja
mapată. În cazul în care are loc un **page fault** pe o pagină care e deja
mapată în segment atunci se va apela handler-ul default (`old_action`).

În funcţia `so_init_loader` setez noul handler de tratare pentru `SIGSEGV`.
Handler-ul va fi functia `segv_handler`.

Consider că tema a fost utilă. Au fost înţelese mai bine conceptele de
virtualizare a memoriei, loader, demand paging, segmente.

Consider că am făcut o implementare destul de bună.

## Implementare

Întregul enunţ al temei este implementat.

***Handler-ul de page fault***

Pentru început se va extrage adresa carea a cauzat **page fault** prin
intermediul câmpului `si_addr`. Se caută secvenţial prin segmente şi când
adresa care a cauzat **page fault** este între adresa de început a
segmentului şi adresa de început la care adăugăm spaţiul ocupat în memorie.
Mai departe, se calculează offsetul faţă de începutul segmentului şi indexul
paginii împărţind offsetul respectiv la dimensiunea unei pagini. Dacă
pagina este deja mapată, se va executa handler-ul default. Dacă pagina nu a
fost mapată încă se mapează prin intermediul `mmap` la adresa paginii
(`found_segment->vaddr + PAGE_SIZE * page_index`) dimensiunea unei pagini
şi se face load din executabil pentru pagina segmentului respectiv. În cazul
în care `mem_size` este mai mare decât `file_size` şi datele disponibile din
fişier au fost încărcate se va zeroiza diferenţa de la adresa de unde se
termină `file_size` până la adresa unde se termină `mem_size`. Într-un final
se setează permisiunile pentru pagină şi se marchează pagina ca fiind 
mapată.

## Cum se compilează și cum se rulează?
* Linux - make (va genera o bibliotecă partajată)

## Bibliografie
* https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-04
* https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-06
* https://ocw.cs.pub.ro/courses/so/teme/tema-3
