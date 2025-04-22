#!/bin/bash

#numero totale di Test
let tot=$(grep -c "Test" testout.log);


printf "\nTotale test lanciati: $tot\n\n"

for i in {1..3}
do
    #Conto quanti testo di ogni tipo sono stati effettuati
    let n=$(grep -c "Test $i" testout.log);

    #Quanti a buon fine
    let ok=$(grep -c "Test $i: Success" testout.log);

    #E quanti falliti
    let ko=$(grep -c "Test $i: Failure" testout.log);
    
    echo "Test $i lanciati: $n [Passati: $ok][Falliti: $ko]";

done

printf "\n"

wait


#Mando il segnale usr1 al server
kill -s USR1 $(pidof mainServer.o)
#Mando il segnale TERM al server
kill -s TERM $(pidof mainServer.o)