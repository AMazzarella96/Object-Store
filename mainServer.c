/*
 * @file mainServer.c
 * @brief Implementazione Server ObjectStore
 * @author Alessandro Mazzarella - Matricola 547541 - Corso B
 * Si dichiara che il contenuto del file "mainServer.c" è opera originale
 * dell'autore
 * NOTA: l'implementazione del signalHandler è stata presa e modificata dalle
 *       soluzioni proposte degli esercizi presentati durante il corso
 */



#define _POSIX_C_SOURCE 200809L
#include<sys/types.h>
#include<sys/un.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<pthread.h>
#include<errno.h>
#include<signal.h>
#include"./utils/utils.h"
#include"./utils/report.h"
#define SOCKNAME "objstore.sock"

//getore segnali
void sighandler();

//variabile per memorizzare il segnale ricevuto
//volatile sig_atomic_t usato per garantire accesso signal safe
volatile sig_atomic_t sigflag = 0;

//variabile per il controllo dell'esecuzione della routine del thread
volatile int running=1;



//thread task
void* th_task(void* v);


/*
Struct per la definizione di un utente
identifica un'associazione username-socket
per la corretta esecuzione delle operazioni
*/
typedef struct el{
    int sck;
    char *name;
    struct el* next;
    struct el* prev;
}usr; 

//Inizializzo la struct per il report
struct info report={0,0,0};

//Funzione per l'inserimento nella lista utenti
void insert(usr **list, char* name, int sck, usr **last);

//Free della lista
void freelist(usr **list);

int main(void){
    unlink(SOCKNAME);
    int sock, clsock, nthreads=0;
    static pthread_t *pool=NULL;
    struct sockaddr_un address;
    
    //lista di struct per tenere traccia di ugni utente nel sistema con la sua socket
    usr *node = NULL;

    //puntatore all'ultimo elemento della lista creato
    //passato a insert per riferimento e aggiornato di volta in volta
    //da passare alla pthread_create
    usr *last = NULL;
    
    //registrazione handler segnali
    sighandler();

    //Stringa per memoriazzare le varie richieste dei client
    char *req = NULL;
    CHECK_PTR((req=calloc(512,sizeof(char))), "req");

    //Array di stringhe per memorizzare la richiesta splittata
    char **param = NULL;

    //Stringa per inviare ripsoste al client
    char *msg=NULL;
    CHECK_PTR((msg = calloc(32,sizeof(char))),"msg");

    //path dei file
    char *path = NULL;
    
    if((sock = socket(AF_UNIX, SOCK_STREAM, 0))<0){
        perror("Error during Server Socket Creation\n");
        exit(EXIT_FAILURE);
    }
    printf("Server Ready\n");
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKNAME, sizeof(address.sun_path));
    bind(sock, (struct sockaddr*) &address, sizeof(address));
    listen(sock, SOMAXCONN);

    //Ciclo di ascolto per REGISTER
    while(!((sigflag==SIGINT) || (sigflag==SIGTERM) || (sigflag==SIGQUIT))){
        
        //Ricevuto USR1 stampo il report e resetto il flag dei segnali
        if(sigflag==SIGUSR1){
            printrep();
            sigflag=0;
        }
        
        if((clsock = accept(sock, NULL, 0))>0){

            //Variabile per contare il numero di parole nella richiesta
            //passata come parametro alla funzione split
            int i=0;

            read(clsock, req, 512);
            printf("Request: %s",req);

            param = split(req,&i);
            printf("Client %s connected\n",param[1]);

            
            if(strcmp(param[0],"REGISTER")==0){
                //creazione della cartella dedicata all'utente
                CHECK_PTR((path = calloc(128,sizeof(char))),"path");
                memset(path,0,128);
                strcpy(path, "./data/");
                strcat(path,param[1]);
                struct stat st = {0};
                if (stat(path, &st) == -1) {
                    if(mkdir(path, 0700)==-1){
                        strcpy(msg, "KO Register failure \n");
                        write(clsock, msg, strlen(msg));
                    }
                }
                
                write(clsock, "OK \n", 5);
                nthreads++;

                insert(&node,param[1],clsock,&last);

                if(pool==NULL){
                    
                    CHECK_PTR((pool = calloc(nthreads,sizeof(pthread_t))),"threadpool");
                }
                else{
                    CHECK_PTR((pool = realloc(pool, nthreads*sizeof(pthread_t))),"threadpool");
                }

    
                if((pthread_create(&pool[nthreads-1], NULL, th_task, (void*) last))!=0){    
                    perror("Error during Thread Creation\n");
                }
                else
                    addusr();
                
                memset(req,0,512);
                for(int j=0;j<i;j++)
                    free(param[j]);
                free(param);
                free(path);
            }
            
        }

        //Dopo ogni batteria di test, quando l'ultimo utente si disconnette
        //faccio la free della lista utenti
        if(getUsrCount()==0){
            freelist(&node);
            node=NULL;
        }
        
    }

    free(msg);
    

    /*-------------------------------------CHIUSURA OBJSTORE-------------------------------------*/
    printf("FASE DI CHIUSURA\n");

    //faccio la join di tutti i thread nel pool
    //in condizioni normali a questo punto dovrebbero essere tutti terminati
    for(int i=0;i<nthreads;i++){
        pthread_join(pool[i],NULL);
    }
    free(req);
    free(pool);

    //Distruggo la mutex in report.h
    dismiss();
    unlink(SOCKNAME);
    return 0;
}


void* th_task(void* v){

    usr user = *(usr*) v;  
    int received=-1, i=0;
   
    long size=0;
    char **param = NULL, *path = NULL, *msg = NULL, *request=NULL;
    CHECK_PTR((msg=calloc(32,sizeof(char))),"msg");
    request=calloc(101,sizeof(char));
    CHECK_PTR((path=calloc(128,sizeof(char))),"path");

    while(running && (received = read(user.sck, request, 100))>0){
            
            printf("%s [%ld] - Request: %s",user.name,pthread_self(),request);


            //funzione che tokenizza le richieste su " \n"
            //mantiene un puntatore che indica il numero di parole 
            param = split(request,&i);

        
            opt r = strtoenum(param[0]);
            FILE *fd=NULL;
            switch (r){
                case 0: //STORE: param[1]=name, param[2]len \n block
                
                    size = atoi(param[2]);
                    //CHECK_PTR((string = calloc((size+1),sizeof(char))),"string");

                    //costruisco il path del file da salvare
                    strcpy(path,"./data/");
                    strcat(path,user.name);
                    strcat(path,"/");
                    strcat(path,param[1]);

                    if((fd = fopen(path, "w")) == NULL){
                        perror("Error while creating the file");
                    }

                    char *string = calloc(size+1,sizeof(char));
                    long r;
                    if((r = readn(user.sck,string,size))!=size){
                        perror("Bad Readn - STORE");
                        strcpy(msg, "KO Store failed\n");
                        writen(user.sck, msg, strlen(msg));
                    }

                    //Memorizzo il blocco nel file file
                    else if((fwrite(string,sizeof(char), size, fd))!=size){
                        perror("File write Error");
                        strcpy(msg, "KO Store failed\n");
                        writen(user.sck, msg, strlen(msg));
                    }
                    else if((writen(user.sck, "OK \n", 5))!=1){
                        perror("Bad Writen - STORE");
                    }
                    free(string);
                    fclose(fd);
                    fd=NULL;

                    //incremento il numero di oggetti nel sistema e aggiorno la dimensione totale
                    addobj(size);

                 break;    
             case 1: //RETRIEVE: param[1]=name

                    strcpy(path,"./data/");
                    strcat(path,user.name);
                    strcat(path,"/");
                    strcat(path,param[1]);
                    if((fd = fopen(path, "r")) == NULL){
                        perror("Error while opening the file");
                        strcpy(msg,"KO Unknown file \n");
                        writen(user.sck,msg,strlen(msg));
                        break;
                    }
                    char* s = NULL;
                    CHECK_PTR((s=calloc(1024, sizeof(char))),"s");
                    CHECK_PTR((string = calloc(1,sizeof(char))),"string");
                    size=1;
                    //Leggo il file e lo salvo in una stringa
                    while(fread(s,sizeof(char),1023,fd)){
                        size+=strlen(s);
                        string=realloc(string,size*sizeof(char));
                        strcat(string,s);
                        memset(s,0,1024);
                    }

                    //decremento la dimensione per escludere il carattere terminatore dal conteggio
                    //ed inviare la size corretta al client
                    size--;
                    free(s);
                    char *res=NULL, *tmp=NULL;
                    CHECK_PTR((res = calloc(100,sizeof(char))),"res");
                    CHECK_PTR((tmp = calloc(16,sizeof(char))),"tmp");

                    //creazione stringa per il responso
                    strcat(res, "DATA ");
                    sprintf(tmp, "%ld", size);
                    strcat(res, tmp);
                    strcat(res, " \n");
                    
                    long w;
                    if((w = write(user.sck,res,100))==-1)
                        perror("Bad Write - RETRIEVE");

                    else if((w = writen(user.sck,string,size))!=1)
                        perror("Bad Writen - RETRIEVE");
                    
                    free(tmp);

                    free(res);
                    free(string);            
                    fclose(fd);
                    fd=NULL;

                 break;   

                case 2: //DELETE: param[1]=name

                    strcpy(path,"./data/");
                    strcat(path,user.name);
                    strcat(path,"/");
                    strcat(path,param[1]);
                    if((fd=fopen(path,"r"))==NULL){
                        strcpy(msg, "KO Unknown File \n");
                        writen(user.sck, msg, strlen(msg));
                        break;
                    }
                    //uso fseek per calcolare la lunghezza totale del file ai fini del report
                    fseek(fd, 0L, SEEK_END);
                    size = ftell(fd);
                    rewind(fd);
                    fclose(fd);
                    fd=NULL;
                    if(remove(path)!=0){
                           strcpy(msg, "KO Delete Error \n");
                           writen(user.sck, msg, strlen(msg));    
                    }
                    else{
                        write(user.sck , "OK \n", 5);
                        //decremento il numero di oggetti e la dimensione totale
                        rmobj(size);
                    }
                 break;

                case 3: //LEAVE
                    printf("%s[%ld] - Request: %s\n",user.name, pthread_self() ,param[0]);
                    rmusr();
                    memset(msg,0,32);
                    strcpy(msg,"OK \n");
                    writen(user.sck , msg, strlen(msg));
                    close(user.sck);
                    for(int j=0;j<i;j++)
                        free(param[j]);
                    free(param);
                    free(path);
                    free(msg); 
                    free(request);  
                    printf("Client %s disconnected\n", user.name);
                    return NULL;
                    break;          
            }

            memset(path,0,128);
            for(int j=0;j<i;j++)
                free(param[j]);
            free(param);
            i=0;
            memset(msg,0,32);
            memset(request,0,101);
        }

        if(received == 0){
            rmusr();
            printf("Client %s disconnected\n", user.name);
        }
        else if(received < 0){
            perror("Connection Error");
            printf("Client %ld crashed\n",pthread_self());
            rmusr();
        }
        
    //terminazione anomala ma potenzialmente senza errori
    //irrangiungibile in casi normali
    
    //Statement irraggiungibile nel caso un cui il client termini con successo
    free(msg);
    free(path);
    close(user.sck);
    free(request);
   
    return NULL;
}

//Procedura che setta il signal flag e in caso di segnale di terminazione
//resetta la variabile running 
void catcher(int sig){
    write(1,"\nSegnale catturato\n", 20);
    if(sig!=SIGUSR1 && (getUsrCount()==0)){
        running=0;
    }
    sigflag=sig;
}

void sighandler(){
    //Inizializzo la variabile di gestione dei segnali
    sigflag = 0;
    //Struct per personalizzare la gestione dei Segnali
    struct sigaction action;
    //Inizializzazione e installazione signal handler
    memset (&action, 0, sizeof(action));   
    action.sa_handler = catcher;

    if(sigaction(SIGINT,  &action, NULL)==-1) 
        perror("sigaction SIGINT");
    if(sigaction(SIGTERM, &action, NULL)==-1) 
        perror("sigaction SIGTERM");
    if(sigaction(SIGQUIT, &action, NULL)==-1)
        perror("sigaction SIGQUIT");
    if(sigaction(SIGUSR1, &action, NULL)==-1)
        perror("sigaction SIGUSR1");
}




void insert(usr **list, char* name, int sck, usr **last){
   
    if(name==NULL || sck<0)
       return;

    if((*list)==NULL){
        CHECK_PTR(((*list) = calloc(1,sizeof(usr))),"node");
        (*list)->next=NULL;
        (*list)->prev=NULL;
        (*list)->name=NULL;
        (*list)->sck=0;
        CHECK_PTR(((*list)->name = calloc(20,sizeof(char))),"list->name");
        memcpy((*list)->name,name,strlen(name));
        (*list)->sck = sck;
        (*last)=(*list);
    }
    else{
        usr *cor = (*list);
        usr *new = NULL;
        CHECK_PTR((new = calloc(1,sizeof(usr))),"new");
        CHECK_PTR((new->name = calloc(20,sizeof(char))),"new->name");
        memcpy(new->name,name,strlen(name));
        new->next=NULL;
        new->prev=NULL;
        new->sck = sck;
        while(cor->next!=NULL)
            cor = cor->next;
        cor->next=new;
        new->prev=cor;
        (*last)=new;
    }
}
 
void freelist(usr **list){
    if((*list)!=NULL){
        while((*list)->next!=NULL){
           (*list)=(*list)->next;
           free((*list)->prev->name);
           free((*list)->prev);
        }
        free((*list)->name);
        free((*list));
    }
}