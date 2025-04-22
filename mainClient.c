/*
 * @file mainClient.c
 * @brief Implementazione Client ObjectStore
 * @author Alessandro Mazzarella - Matricola 547541 - Corso B
 * Si dichiara che il contenuto del file "mainClient.c" è opera originale
 * dell'autore
 */


#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>
#include<errno.h>
#include "./utils/utils.h"
#include "./utils/os_lib.h"
#include "./utils/status.h"

int filecompare(char* file, long len);

//struct per controllare l'esistenza di un file
struct stat stats={0,0,0};

int err=0;

int main(int argc, char *argv[]){
    if(argc!=3){
        perror("Wrong args: Name OpNo.");
        return -1;
    }

    int r = atoi(argv[2]);
    if(r<1||r>3){
        perror("Unknown Operation");
        return -1;
    }

    unsigned long size;
    char *s = NULL, *block=NULL, *name=NULL, *tmp=NULL;
    CHECK_PTR((s = calloc(25, sizeof(char))),"s");

    //Username
    strcpy(s,argv[1]);
    inc_op_count();
    if((os_connect(s))==0){
        perror("An error ocurred while connecting to the server");
        inc_fail_count();
        exit(EXIT_FAILURE);
    }
    
    free(s);
    inc_succ_count();
    
    
    switch (r){
        case 1: //STORE
            
            size=100;
            CHECK_PTR((tmp=calloc(10,sizeof(char))),"tmp");
            CHECK_PTR((name=calloc(20,sizeof(char))),"name");
            for(int i=0;i<20;i++){
                inc_op_count();        
                strcpy(name,"prova");
                sprintf(tmp, "%ld", size);
                strcat(name,tmp);
                strcat(name,".txt");
                CHECK_PTR((block = calloc((size+1),sizeof(char))),"block");
                memset(block,0,size+1);

                //Costruisco il blocco per la store
                for(long i=0;i<size;i++){
                    if(i%2==0)
                        block[i]='1';
                    else
                        block[i]='0';
                }
                block[size]='\0';

                if((os_store(name,block,size))!=1){
                    inc_fail_count();
                    err=1;
                    perror("Error in Store");
                }
                else{
                    inc_succ_count();
                }
                memset(tmp,0,10);
                memset(name,0,20);
                free(block);
                size += 5258;   //(100000-100)/19
            }
            free(tmp);           
            free(name);                          
            break;

        case 2: //RETRIEVE 
            size = 100;
            CHECK_PTR((tmp=calloc(10,sizeof(char))),"tmp");
            CHECK_PTR((name=calloc(20,sizeof(char))),"name");
            for(int i=0;i<20;i++){
                inc_op_count();
                strcpy(name,"prova");
                sprintf(tmp, "%ld", size);
                strcat(name,tmp);
                strcat(name,".txt");
                char *file = NULL;
                file = (char*) os_retrieve(name);
                if((file)==NULL){
                    perror("Unexistent File");
                    err=1;
                    inc_fail_count();
                }
                else{
                    //Funzione per controllare che il contenuto del  
                    //file ricevuto sia quello atteso
                    if((filecompare(file, size))==0){
                        inc_succ_count();
                    }
                    else{
                        perror("File mismatch");
                        err=1;
                        inc_fail_count();
                    }
                    free(file);
                }

                size += 5258; 
                memset(name,0,20);
                memset(tmp,0,10);
            }
            free(name);
            free(tmp);
         break;

        case 3: //DELETE
            size = 100;
            CHECK_PTR((tmp=calloc(10,sizeof(char))),"tmp");
            CHECK_PTR((name=calloc(20,sizeof(char))),"name");
            for(int i=0;i<20;i++){
                inc_op_count();
                strcpy(name,"prova");
                sprintf(tmp, "%ld", size);
                strcat(name,tmp);
                strcat(name,".txt");
                if((os_delete(name))==0){
                    inc_fail_count();
                    err=1;
                    perror("Unexistent File");
                }
                else
                    inc_succ_count();  
                size += 5258;
                memset(name,0,20);
                memset(tmp,0,10);                  
            }
            free(name);
            free(tmp);
            break;

    }

    printf("Test %d: ",r);
    if(err){
        printf("Failure\n\n\n");
    }
    else
        printf("Success\n\n\n");

    inc_op_count();
    if((os_disconnect())!=1){
        inc_fail_count();
        perror("Unable to disconnect");
    }
    else
        inc_succ_count();
        
    printstat();
    return 0;
}

int filecompare(char* file, long len){
    //Creo un blocco di dimensione len e lo confronto con file
    char *b=NULL;
    CHECK_PTR((b = calloc(len+1,sizeof(char))),"b");
    for(long i=0;i<len;i++){
        if(i%2==0)
            b[i]='1';
        else
            b[i]='0';       
    }
    char s[len];
    strcpy(s,b);
    free(b);

    return strcmp(file,s);
}
