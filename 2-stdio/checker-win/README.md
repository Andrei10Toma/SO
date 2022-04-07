### Toma Andrei: 331CB:
# Tema 2 Bibliotecă stdio

## Organizare
Pentru implementarea temei mi-am definit structura SO_FILE care conţine
următoarele câmpuri:
- fd - file descriptorul fişierului deschis (de tip `HANDLE`)
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

Tema a fost utilă pentru a înţelege mecanismul de buffering si lucrul cu
apelurile de sistem.

## Implementare
Este implementată doar partea de operaţii cu fişiere. Partea de procese am
implementat-o doar pe Linux.

### **Buffering**
Pentru buffering-ul de citire de fiecare dată cand offset-ul buffer-ului de
citire este egal cu lungimea buffer-ului acesta este invalidat şi umplut din
nou cu informaţie nouă. Când se face o operaţie de citire (`so_fgetc`/
`so_fread`) informaţia este luată direct din buffer.

Pentru buffering-ul de scriere se scrie în el pana cand se umple cu BUFSIZE 
(4096) după care acesta este golit. De asemenea, golirea poate fi forţată
prin apelul lui `so_fflush` şi dacă se face o operaţie de `so_fseek` când
ultima operaţie a fost una de citire.

## Cum se compilează şi cum se rulează?
Windows - nmake.exe

## Bibliografie
- https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-01
- https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-02
- https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-03