/*
 * @file utils.c
 * @brief Implementazione funzioni ausiliarie per client e server
 * @author Alessandro Mazzarella - Matricola 547541 - Corso B
 * Si dichiara che il contenuto del file "utils.c" è opera originale
 * dell'autore, salvo specificato diversamente, e.g. riga 36
 */


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/un.h>
#include<errno.h>
#include"utils.h"


//Spezza le richieste parola per parola
//Restituisce un Array di stringhe
char **split(char *req, int *i){
    char **param=NULL,*token,*svptr;
    token =__strtok_r(req," \n",&svptr);
    while (token){
        CHECK_PTR((param = realloc(param,((*i)+1)*sizeof(char*))),"param");
        CHECK_PTR((param[(*i)]=calloc((strlen(token)+1),sizeof(char))),"param[i]");
        memset(param[(*i)],0,strlen(token)+1);
        memcpy(param[(*i)++],token,strlen(token));
        token =__strtok_r(NULL," \n",&svptr);
    }
    return param;
}


//funzioni per sopperire a letture/scritture parziali da parte di read e write
//soluzioni trovate online
int writen(long fd, void *buf, size_t n)
{
	size_t nleft, nwritten;
	const char	*ptr;

	ptr = buf;	
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0)
			return(nwritten);		/* error */

		nleft -= nwritten;
		ptr += nwritten;
	}
	return 1;
}



int readn(int fd, void *vptr, size_t n){
     size_t  nleft;
     ssize_t nread;
     char   *ptr;     ptr = vptr;
     nleft = n;
     while (nleft > 0) {
         if ( (nread = read(fd, ptr, nleft)) < 0) {
             if (errno == EINTR)
                 nread = 0;      /* and call read() again */
             else
                 return (-1);
         } else if (nread == 0)
             break;              /* EOF */
         nleft -= nread;
         ptr += nread;
     }
     return (n - nleft);         /* return >= 0 */
 }
 


//soluzione presa da StackOverflow
//restituisce un intero a partire da una stringa(ENUM)
opt strtoenum (const char *str){
    for (int i = 0;  i<sizeof (conversion) / sizeof (conversion[0]);  i++){
        if (!strcmp (str, conversion[i].str))
            return conversion[i].val;    
    }
    return 4;
}