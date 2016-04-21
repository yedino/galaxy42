#!/bin/bash
group_name="$1"
group_size="$2"

if [ $# -lt 2 ]
then
echo "---------------------
Not enough arguments!
Give 2 arguments:    
group_name group_size
---------------------"
elif [ $# -gt 2 ]
then
echo "---------------------
Too many arguments!
Give 2 arguments:    
group_name group_size
---------------------"
else
	for i in $(seq 1 $group_size)
	do
		
		(sudo ip link delete "prv${group_name}${i}xH") > out.log
		wc -l out.log
	done
fi
