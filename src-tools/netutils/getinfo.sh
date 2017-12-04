#!/bin/bash

LC_ALL=C

function lscpu_field() {
	lscpu | egrep "${1}:" | cut -d':' -f2-99 | awk '{out=$1; for(i=1+1;i<=NF;i++){out=out" "$i}; print out}'
}

cpu_arch=$(lscpu_field 'Architecture')
cpu_name=$(lscpu_field 'Model name')
cpu_cpu_n=$(lscpu_field '^CPU\(s\)')
cpu_mhz=$(lscpu_field 'CPU MHz')
cpu_mhz=$(echo "${cpu_mhz}." | cut -d"." -f1) # cut floating point part

cpu_name_short="${cpu_name:0:10}"
[[ "$cpu_name_short" != "$cpu_name" ]] && cpu_name_short="$cpu_name_short..."

cpu_all_1="${cpu_arch} ${cpu_cpu_n}_core@${cpu_mhz}_MHz '${cpu_name_short}'"
cpu_all_2="${cpu_arch} ${cpu_cpu_n}_core@${cpu_mhz}_MHz '${cpu_name}'"

git_rev_full=$(git log --format='%H' -1)
git_rev="${git_rev_full:0:10}"

hostname=$(hostname)

now_date=$(date -u)

echo "cpu:    $cpu_all_1"
echo "cpu:    $cpu_all_2"
echo "gitrev: $git_rev"
echo "now:    $now_date"
echo "host:   $hostname"


