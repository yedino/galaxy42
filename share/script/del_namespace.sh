#!/bin/bash
group_name="$1"
group_size="$2"

# *** review *** this program was PARTIALLY reviewed and probably does not have critical flaws

echo "This program will try to delete previously created namespaces of netpriv."
echo "It will create (overwrite!) a log file here called out.log" # <--- log name
echo "Press ENTER to continue or abort (e.g. with Ctrl-C)"
read _

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
		
		(sudo ip link delete "prv${group_name}${i}xH") > out.log # see log name above too
		wc -l out.log
	done
fi
