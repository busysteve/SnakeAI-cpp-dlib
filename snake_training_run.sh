#!/bin/bash
counter=1
for name in $( ls goodnets ); do
./snake-ai 40 40 2000 4 goodnets/$name training_data/training.dat.$counter
let counter=counter+1
sleep 1
done;






