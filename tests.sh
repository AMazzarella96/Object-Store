#!/bin/bash

exec &> testout.log


#Avvio i 50 client per le Store e attendo che terminino
for i in {1..50} 
do
    ./mainClient.o User$i 1 &
done

wait

#Avvio gli altri 50 client di cui 30 RETRIEVE e 20 DELETE
for i in {1..30}
do
    ./mainClient.o User$i 2 &
done

for j in {31..50}
do
    ./mainClient.o User$j 3 &
done

wait

echo "Done"
