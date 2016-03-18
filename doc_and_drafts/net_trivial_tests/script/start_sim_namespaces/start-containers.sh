group_name="$1"
group_size="$2"

for i in $(seq 1 $group_size)
do
	echo "Creating sandbox $i in group $group_name"
	bash ./step1.sh "${group_name}${i}" &
done

sleep 1

echo "Now the configuration:"

for i in $(seq 1 $group_size)
do
	sudo netpriv9 "${group_name}${i}" "${i}" 61 62
	# sudo netpriv9 48 48 1 2
	# "${group_name}${i}" &
done

