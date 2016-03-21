echo "Running the tunserver in devel mode, devel_num=$devel_num"
set -x
./tunserver.elf --devel --develnum $devel_num
