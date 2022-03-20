# *Tema 1 SO* (Preprocesor C)
## *Toma Andrei 331CB*
<br>

## *Overview*
Pentru implementarea temei am folosit 2 structuri de date: vector cu un
array care isi face resize cand este atinsa capacitatea maxima (similar cu
cel din c++) si un hashmap. Pentru inceput sunt pasate argumentele si dupa
aceea codul este preprocesat linie cu linie.
<br>
Vectorul l-am folosit pentru a retine directoarele in care trebuie cautate
fisierele incluse cu directiva `#include`. Am ales sa folosec vector pentru
ca pot fi mai multe directoare date prin argumentul `-I`.
<br>
Hashmapul l-am folosit pentru a retine maparile cheie-valoare definite prin
directiva `#define` sau prin intermediul parametrului `-D`.

## *Detalii de implementare hashtable*
Pe tabela hash se vor putea efectua urmatoarele operatii:
- `creare` - este alocata memorie pentru hashmap (o capacitate initiala de
383 de mapari). Am ales un numar prim pentru a reduce numarul de coliziuni;
- `get` - ia valoarea asociata cheii din hashmap;
- `put` - pune maparea cheie valoare in hashmap pe baza hash-ului;
- `destroy` - dezaloca memoria alocata pentru hashmap;
- `remove` - sterge cheia si valoarea asociata acesteia din hashmap;
- `hash` - am folosit algoritmul FNV-1a, deoarece am citit pe net despre el si am
vazut ca are o rata de coliziune mica.

## *Detalii de implementare vector*
Pe vector se vor putea efectua urmatoarele aparitii:
- `creare` - este alocata memorie pentru vector (am ales o capacitate initiala de 5),
iar de fiecare data cand vectorul ajunge la capacitatea maxima este realocat cu
capacitate dupla;
- `append` - este adaugat elementul la finalul vectorului, iar in cazul in care
vectorul este plin se face realocarea;
- `destroy` - este dezalocata memoria pentru vector.

## *Detalii de implementare preprocesor*
Pentru inceput se face interpretarea parametrilor din linia de comanda. In cazul in
care a fost citit `-I` va fi extrasa calea data din parametru si introdusa in
vectorul de directoare incluse. In cazul in care se citeste `-D` se va adauga
maparea cheie-valoare in hashmap. Daca este definita doar cheia atunci se va adauga
in hashmap cheia care are asociata o valoare dummy. In cazul in care nu este nici
`-I` si nici `-D` primul fisier va fi interpretat ca input, iar al doilea fisier ca
output. Daca sunt mai de 2 fisiere atunci se va returna cod de eroare 12.

Mai departe va incepe preprocesarea propriu zisa a codului. In primul rand se vor
ignora liniile care sunt goale si dupa va incepe partea de preprocesare in functie
de ce este citit la inceput de linie. Aproape pentru fiecare directiva se va verifica
daca trebuie sa fie preprocesate liniile prin intermediul flag-urilor de if branch.
- `#include` - se extrage calea data dupa directiva `#include` si se cauta mai intai
in fisierul curent in care se afla codul sursa si daca nu a fost gasit acolo se
parcurg secvential in vectorul de directoare incluse. Daca a fost gasit in unul
dintre cazurile enumerate precedent atunci intra in recursivitate si este preprocesat
codul din fisierul inclus.
- `#define` - se parseaza define-ul extragandu-se cheia si valoarea. Valoarea este
parsata inca o data in cazul in care se afla inca un define in interiorul sau. Daca
se gaseste un define in interior acesta este inlocuit cu valoarea salvata in hashmap.
In cazul in care **ultimul** caracter citit de pe linie este '\' atunci se va activa
flag-ul de multi line define. Cat timp flag-ul de multi line define este activat se
concateneaza la valoare si cand la final nu se mai citeste '\' se pune in hasmap
cheia si valoarea.
- `#ifdef` || `#ifndef` - se ia cheia data din conditie si daca este
(ne)definita in hashmap atunci se va pasa branchul respectiv, altfel se va duce pe
branch-ul `else`.
- `#if` - se ia conditia se evalueaza si daca este diferita de 0 atunci se va pasa
branch-ul if, altfel se va cauta pe branch-urile elif si cand se gaseste o conditie
care e adevarata si se opresete parsarea branch-ului if.
- `#endif` - se reseteaza toate flagurile pentru branch-ul if.
- `#else` - in cazul in care nu a fost nicio conditie evaluata la true se intra pe
acest branch si este preprocesat.
- `#undef` - sterge cheia data dupa undef din hashmap.

Daca nu a fost gasita niciuna dintre directivele precizate mai sus atunci se
preproceseaza linia de cod normal. Se inlocuisec literalii care se afla definite in
hasmap. Literalii respectivi nu vor trebui sa faca parte dintr-un string. La final,
se elibereaza toata memoria.
