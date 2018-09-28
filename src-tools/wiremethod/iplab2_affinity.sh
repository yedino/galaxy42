#!/bin/bash
echo "For IRQ $1 setting affinity $2" ; echo "$2" > "/proc/irq/$1/smp_affinity"
