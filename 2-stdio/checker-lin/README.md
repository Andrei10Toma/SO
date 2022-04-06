### Toma Andrei: 331CB:
# Tema 2 Bibliotecă stdio

## Organizare
Pentru implementarea temei mi-am definit structura SO_FILE care conţine
următoarele câmpuri:
- fd - file descriptorul fişierului deschis
- cursor - indica poziţia actuala in fişier
- last_operation - are tipul `Operation` care este un enum care conţine
valorile `READ` şi `WRITE`; în acest câmp voi reţine ultima operaţie
efectuată pe fişier (citire sau scriere)
- eof - determină dacă s-a ajuns la finalul fişierului
- child_pid - folosit pentru procesele deschise cu `so_popen` pentru a ştii
cand se dă `so_pclose` ce proces să fie aşteptat
- read_buffer - folosit la buffering-ul pentru citire
- read_buffer_length - lungimea buffer-ului de citire
- read_buffer_offset - poziţia actuala in buffer-ul de citire
- read_error - setat daca are loc o eroare în timpul citirii
- write_buffer - folosit la buffering-ul pentru scriere
- write_buffer_offset - poziţia actuala in buffer-ul de citire
- write_error - setat daca are loc o eroare în timpul scrierii

Tema a fost utilă pentru a înţelege mecanismul de buffering, lucrul cu
apelurile de sistem şi procese şi comunicarea acestora prin pipe.

## Implementare
Întregul enunţ al temei este implementat.

### **Buffering**
Pentru buffering-ul de citire de fiecare dată cand offset-ul buffer-ului de
citire este egal cu lungimea buffer-ului acesta este invalidat şi umplut din
nou cu informaţie nouă. Când se face o operaţie de citire (`so_fgetc`/
`so_fread`) informaţia este luată direct din buffer.

Pentru buffering-ul de scriere se scrie în el pana cand se umple cu BUFSIZE 
(4096) după care acesta este golit. De asemenea, golirea poate fi forţată
prin apelul lui `so_fflush` şi dacă se face o operaţie de `so_fseek` când
ultima operaţie a fost una de citire.

### **Rularea de procese**
În primul rând, este creat un pipe prin intermediul căruia procesul nou creat
o să comunice cu procesul părinte. Mai departe se apelează `fork()` şi este
creat procesul copil. Acesta din urmă va închide capătul nefolosit al 
pipe-ului şi va redirecta stdout-ul/stdin-ul vor fi redirectate către capătul
neînchis al pipe-ului. Astfel, după aceea în procesul părinte se va crea un
`SO_FILE` care va avea ca file descriptor capătul neînchis al pipe-ului. De
asemenea, va seta pid-ul copilului, deoarece când se apelează `so_pclose` să
fie aşteptată terminarea execuţiei procesului copil.

## Cum se compilează şi cum se rulează?
Linux - make

## Bibliografie
- https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-01
- https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-02
- https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-03