# Implementazione di Binary Matrix Multiplication in C ed ottimizzazione con acelleratore dedicato
## Introduzione
Libreria in C per la moltiplicazione di matrici binarie, con un'implementazione di base e una versione ottimizzata per acceleratori dedicati.

La libreia fornisce strutture dati per rappresentare matrici binarie e frammenti di matrice, funzioni per il caricamento e salvataggio di matrici da e verso frammenti, funzioni per la moltiplicazione di matrici binarie ed un set di funzioni e strutture di controllo per l'accesso e l'utilizzo di acceleratori dedicati.

## Struttura del progetto
```
.
├── build 
│   ├── bin [GIT UNTRACKED]
│   │   ├── test
│   │   │   └── <all test binaries>
│   │   └── main.elf
│   └── obj [GIT UNTRACKED]
│       ├── libraries
│       │   └── <all library folders containing object files>
│       └── src
│           └── <all source files compiled to object files>
├── include
│   └── <all include files>
├── libraries
│   └── <all library folders>
│        ├── src
│        │   └── <all source files for the library>
│        └── include
│            └── <all include files for the library>
├── src
│   ├── main.c
│   └── <here are placed Project Sources>
├── test
│   ├── <test 1 folder>
│   │   └── test1.c
│   ├── <test 2 folder>
│   │   └── test2.c
│   └── <test x folder>
│       └── testx.c
├── scripts
│   ├── <all scripts for build, test, etc.>
│   └── <all scripts for the project>
├── scratch [UNTRACKED]
├── linker_script.ld
├── activate 
├── do_build
└── Makefile
```

## Compilazione
Per compilare il progetto, eseguire il comando viene usato `make`, ci sono varie regole per la compilazione:
 - `make` Compila librerie e sorgenti, e linka tutto seguendo il linker_script. Crea la cartella `build/bin` e `build/obj` se non esistono.
 - `make test` Compila le librerie ed i sorgenti presenti nelle varie sotto cartelle di `test`, linkando tutte 
le sotto directory di `test` in eseguibili separati assieme alle librerie. Utile per testare le unità.
 - `make all` Esegue `make` e `make test`, compilando tutto il progetto e i test.
 - `make sim` Come `make all` ma definendo la macro `SIMULATION`, utile per testare le unità in modalità simulazione. 
 - `make dump` Esegue il dump dell'eseguibile principale
 - `make symbols` Esegue il dump dei simboli dell'eseguibile principale
 - `make sections` Esegue il dump delle sezioni dell'eseguibile principale
 - `make clean` Elimina l'esecutable principale.
 - `make cleanall` Elimina l'esecutable principale, le librerie e gli oggetti compilati.

### Script `do_build`
Il file `do_build` è uno script che oltre ad eseguire le direttive make accetta delle opzioni e genera ulteriori file, esportando anche gli eseguibili in vari formati.
Accetta le seguenti opzioni:
 - `-c` Esegue un clean prima di compilare
 - `-s` Esporta alcuni eseguibili nelle directory di simulazione
 - `-t` Compila i test esportandoli in header per la compilazione assieme agli ARM
 - `-f <file>` Esporta l'eseguibile `<file>` in un header nel progetto del loader, nel file `testLibProgram.h`