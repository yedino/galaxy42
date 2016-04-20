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

set -x

git submodule init

git submodule update 

set +x



