/*
 * @file os_lib.c
 * @brief Implementazione funzioni libreria lato client
 * @author Alessandro Mazzarella - Matricola 547541 - Corso B
 * Si dichiara che il contenuto del file "os_lib.c" è opera originale
 * dell'autore
 */


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>
#include<errno.h>
#include"utils.h"
#include"os_lib.h"
#include"report.h"
#define SOCKNAME "objstore.sock"

int checkresponse(char *s);
int sock;

int os_connect(char *name){
    
    struct sockaddr_un address; 
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0))<0){
        perror("Error during Client Socket Creation\n");
        exit(EXIT_FAILURE);
    }
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKNAME, sizeof(address.sun_path));
    while(connect(sock,(struct sockaddr*)&address,sizeof(address)) == -1 ){
        if ( errno == ENOENT ){
            sleep(1); /* sock non esiste */
        }
        else {
            perror("Unable to connect");
            exit(EXIT_FAILURE);
        }
    }
   
    char s[512] = "REGISTER ";
    strcat(s, name);
    strcat(s, " \n");
    
   

    int l = write(sock, s, strlen(s)+1);
    if(l!=strlen(s)+1)
        perror("Error in write");
    memset(s,0,512);
    

    l = read(sock, s, 512);

    if(l<0){
        perror("Server unreachable");
        exit(0);
    }


    return(checkresponse(s));
}


int os_store(char *name, void *block, size_t len){
    
    char *s = NULL, *size = NULL;
    CHECK_PTR((size = calloc(10, sizeof(char))),"size");    
    CHECK_PTR((s = calloc(128, sizeof(char))),"s");

    //Costruisco l'header della richiesta in formato STORE name len \n
    strcat(s,"STORE ");
    strcat(s,name);
    strcat(s," ");
    sprintf(size, "%ld", len);
    strcat(s, size);
    strcat(s," \n");


    //Invio l'header
    long w;
    if((w = write(sock,s,100))==-1)
        perror("Bad Write - os_store");

    //Invio il blocco
    if((w = writen(sock,block,len))!=1)
        perror("Bad Writen - os_store");
        
    
    //Read per ricevere il responso del server    
    if((read(sock, s, 32))==-1){
        perror("Error in read");
        return 0;
    }
    int ck = checkresponse(s);
    free(s);
    free(size);
    return(ck);
}


void *os_retrieve(char *name){
    char s[32] = "RETRIEVE ", *buf=NULL, **req=NULL;
    strcat(s,name);
    strcat(s," \n");

    long w;
    //Invio l'header al server
    if((w = writen(sock,s,strlen(s)))!=1){
        perror("Bad Writen - os_retrieve");
        return NULL;
    }
    memset(s,0,32);

    
    char *rq = calloc(101,sizeof(char));
   
    //Mi metto in ascolto per la ripsosta del server
    if((w = read(sock,rq,100))==-1)
        perror("Bad Read - os_retrieve");

    int i=0;
    if((req = split(rq,&i))==NULL){
        perror("Split Error");
        for(int j=0;j<i;j++){
            free(req[j]);
        }
        free(req);
        free(rq);
        return NULL;
    }
    

    if(strcmp(req[0],"KO")==0){
        free(rq);
        for(int j=0;j<i;j++){
            free(req[j]);
        }
        
        free(req);
        return NULL;
    }
    else{
        int len=atoi(req[1]);
        buf=calloc(len+1,sizeof(char));

        if((w=readn(sock,buf,len))!=len)
            perror("Bad Readn - os_retrieve");
    }
    for(int j=0;j<i;j++){
        free(req[j]);
    }
    free(req);
    free(rq);
    return buf;
}



int os_delete(char *name){
    char s[32] = "DELETE ";
    strcat(s,name);
    strcat(s," \n");
    if((writen(sock, s, strlen(s)))!=1){
        perror("Error while writing");
        return 0;
    }
    else{
        memset(s,0,32);
        if((read(sock, s, 32))==-1){
            perror("Error in read");
            exit(EXIT_FAILURE);
        }
        
        return(checkresponse(s));
    }
    
    return 0;
}


int os_disconnect(){
    char s[8] = "LEAVE \n";
    if((writen(sock, s, strlen(s)))!=1)
        return 0;
    memset(s,0,8);
    if((read(sock, s, 8))<0)
        return 0;
    close(sock);
    return 1;
}



//Controllare esito risposte server
int checkresponse(char* s){
    if(s[0]=='O'){
        return 1;
    }
    if(s[0]=='K'){
        return 0;
    }
    else{
        return -1;
    }
}