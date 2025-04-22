# Sistemi Operativi Project developed during 3rd year of computer science bachelor's degree (2019)

The student is expected to implement an object store implemented as a client-server system, and intended to support requests to store and retrieve blocks of data from a large number of applications. The connection between clients and object store is through sockets on local domain. 
In particular, it will be necessary to implement:
- the server part (object store) as a stand-alone executable
- a library intended to be embedded in clients that interfaces with the object store using the protocol defined below
- an example client that uses the library to test the operation of the system

# Object-Store
The object store is an executable whose purpose is to receive requests from clients to store, retrieve, delete named blocks of data, called “objects.” The object store:
- manages a separate storage space for each client
- object names are guaranteed to be unique within a client's storage space
- client names are guaranteed to be all distinct

# Files & Structure

Il progetto si compone di due file principali: mainServer.c, mainClient.c, e di una directory di utility
contenente i file os_lib.c/h, relativi alla libreria per le funzioni lato client, utils.c/h, in cui è definita
una macro di controllo e un insieme di funzioni di supporto per client e server, e due file per la
gestione dei report delle operazioni: report.h (lato server), status.h (lato client).
- mainServer.c: Il file rappresenta l’implementazione del Server di objectStore; al suo interno è definito il
ciclo di ascolto per le richieste di connessione da parte dei client, che si occupa della creazione
delle cartelle per ogni utente, e la routine dei thread per servire le richieste. Per la gestione dei
diversi client è stata definita una lista bidirezionale di struct usr definita come una coppia
<username, socket>, così da identificare univocamente le diverse richieste da parte dei client.
Vengono forniti due metodi per la gestione della lista, quali insert e freelist. Insert provvede
ad inserire in coda il nuovo blocco usr creato, inoltre aggiorna un puntatore last che punta
all’ultimo elemento nella lista, il quale viene utilizzato come parametro della pthread_create.
Freelist invece, si occupa della deallocazione della lista a test ultimati. La routine del thread
è implementata mediante uno switch case su un intero che rappresenta la richiesta; a tal
proposito si avvale di una enum opt e una funzione strtoenum per la conversione da stringa a
valore numerico. Infine, è stato definito un meccanismo per la gestione dei segnali per evitare
di lasciare il sistema in situazioni di inconsistenza; definito in un apposito paragrafo.

- `mainClient.c`
Il file rappresenta l’implementazione del Client di ObjectStore, in cui all’avvio, dopo essersi
accertati che i parametri passati per argomento sono corretti, viene effettuata una chiamata
alla funzione os_connect() per instaurare la connessione con il server e successivamente si
entra all’interno di uno switch case per distinguere le 3 operazioni: STORE, RETRIEVE e
DELETE. Nel caso particolare della retrieve, è stata definita una funzione
filecompare(file, size) che ha il compito di verificare che il contenuto del file ricevuto sia
quello atteso, in cui viene generato sul momento un file di dimensione size e viene effettuata
una string compare tra il file della retrieve e quest’ultimo. Completata l’operazione richiesta,
in uscita viene chiamata la funzione os_disconnect() la quale invia una richiesta di
disconnessione al server.
- os_lib.c
File contenente l’implementazione delle funzioni utilizzate dal client per inviare le richieste
al server secondo il protocollo di comunicazione definito. Al suo interno è definita una
variabile globale int sock relativa alla socket del client, settata dal metodo os_connect() in
fase di connessione; utilizzata dalle altre funzioni per l’invio delle richieste. Per lo scambio di
messaggi via socket tra client e server sono state utilizzate, oltre a read e write, due funzioni
ausiliarie: readn e writen (in utils.c), per evitare letture o scritture parziali durante l’invio o la
ricezione di blocchi di dimensioni elevate (funzioni definite all’interno di utils.c). Un’ulteriore
funzione di supporto è checkresponse, che si occupa di controllare se il messaggio ricevuto
dal server sia un messaggio di “OK” o “KO”.
2
- utils.c/h
(.h) Interfaccia contenente una macro per il controllo dei puntatori allocati (CHECK_PTR),
una enum che rappresenta le diverse operazioni e una struct che associa al valore enum una
stringa, utilizzata dalla funzione strtoenum per il passaggio da stiringa a enum.
(.c) File contenente l’implementazione delle funzioni di utility utilizzate da client e server,
quali readn e writen per sopperire ai problemi di letture/scritture parziali, split che ha il
compito di splittare le richieste parola per parola restituendo un array di stringhe e il numero
di parole splittate, e infine la strtoenum citata sopra.
- report.h
Interfaccia lato server in cui è stata definita una struct info ai fini del report sullo stato del
sistema e le relative funzioni per l’update dei parametri e la stampa del report. Si compone
dei seguenti parametri:
• users: indica il numero di utenti online;
• objcount: indica il numero totale di file presenti nel sistema;
• totsize: indica la dimensione totale dei file presenti nel sistema.
Essendo funzioni chiamate dai diversi thread, è stata dichiarata una mutex per l’accesso in
mutua esclusione alla struct, salvo per addusr() e printrep() che vengono chiamate soltanto
dal main thread del server.
- status.h
Interfaccia lato client in cui è stata definita una struct stat ai fini del report sullo stato delle
operazioni effettuate dei client con le relative funzioni per l’update e la stampa delle
statistiche. Si compone dei seguenti parametri:
• op_count: conto totale delle operazioni eseguite;
• op_success: numero di operazioni andate a buon fine;
• op_failed: numero di perazioni fallite.
Essendo propria di ogni client non è stato necessario alcun meccanismo di muta esclusione.
