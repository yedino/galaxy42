echo "This script finshes the download of source code and (some) dependencies."

echo "Downloading from git submodules"
echo ""
echo "READ THIS:"
echo ""
echo "If this step fails, then you should first clone given project from "
echo "it's upstream and push it into your repository that you use here"
echo "For example you need to get (in other directory) with:"
echo "git clone https://github.com/google/googletest"
echo "and then create repository next to place where you have this repository"
echo "E.g. if you have THIS project on git.you.com/antinet.git then "
echo "create and push the google tests into git.you.com/googletest.git"
echo ""
echo "---------------------------------------------------------------"
echo "CLONE THIS to your repository:"
echo ""
echo " * https://github.com/google/googletest"
echo " * https://github.com/NTRUOpenSourceProject/ntru-crypto"
echo ""
echo "---------------------------------------------------------------"

set -x

git submodule init || { echo "Error: submodules download failed (init);" ; exit 1 ; }

git submodule update || { echo "Error: submodules download failed (update)." ; exit 1 ; }

set +x



