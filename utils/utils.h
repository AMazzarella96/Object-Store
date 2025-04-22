/*
 * @file utils.h
 * @brief Interfaccia funzioni utility client/server
 * @author Alessandro Mazzarella - Matricola 547541 - Corso B
 * Si dichiara che il contenuto del file "utils.h" è opera originale
 * dell'autore, salvo specificato diversamente
 */


#include<stdlib.h>
#include<stdio.h>
#ifndef UTILS_H_
#define UTILS_H_
#define CHECK_PTR(ptr,string)\
    if(!ptr){\
        perror("Allocation Error\n");\
        perror(string);\
        EXIT_FAILURE;\
    }
#endif

//Soluzione trovata online su https://stackoverflow.com/questions/16844728/converting-from-string-to-enum-in-c

typedef enum{
    STORE = 0,
    RETRIEVE = 1,
    DELETE = 2,
    LEAVE = 3
} opt;

const static struct {
    opt      val;
    const char *str;
} conversion [] = {
    {STORE, "STORE"},
    {RETRIEVE, "RETRIEVE"},
    {DELETE, "DELETE"},
    {LEAVE, "LEAVE"}
};

opt strtoenum (const char *str);
//fine.

int writen(long fd, void *buf, size_t size);
int readn(int fd, void *vptr, size_t n);
char **split(char *str, int *i);


