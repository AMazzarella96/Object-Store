/*
 * @file os_lib.h
 * @brief Interfaccia funzioni libreria lato client
 * @author Alessandro Mazzarella - Matricola 547541 - Corso B
 * Si dichiara che il contenuto del file "os_lib.h" è opera originale
 * dell'autore
 */

#include<stdlib.h>
#include<stdio.h>

int os_connect(char *name);
int os_store(char *name, void *block, size_t len);
void *os_retrieve(char *name);
int os_delete(char *name);
int os_disconnect();
