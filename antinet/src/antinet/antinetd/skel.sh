
echo "Info: This script creates a new file (.cpp + .h) from skell{.cpp,.h} doing a copy and trivial replacement"

name="$1"
re='^[a-zA-Z0-9\_-]{1,50}$'
if ! [[ "${name}" =~ ${re} ]]; then echo "Error: Invalid name (${name}). Please give the new class name, e.g. c_foo, see this script for allowed names"; exit 1; fi

nameup="$( echo "$1" | tr '[:upper:]' '[:lower:]' )"

echo "Info: Creating new cpp/hpp file from skel: ${name} (${nameup})"

for ext in cpp hpp
do
	cp -n "c_skel.${ext}" "${name}.${ext}" || { echo "Can not copy (${ext})" ; exit 1 ; }
	sed -i -e "s/c_skel/${name}/g" "${name}.${ext}"
	sed -i -e "s/C_SKEL/${nameup}/g" "${name}.${ext}"
done





