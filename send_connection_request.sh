#!/bin/sh

i=1
while :
do
    ./tcp_client.exe 
    i=$((i+1))
    if [ $i -gt 1000 ]
    then
        break
    fi
done