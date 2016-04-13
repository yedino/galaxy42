#/bin/bash

projecttop=$( git rev-parse --show-toplevel 2>/dev/null )

cd "$projecttop"

python3 tools/git_pretty_signature.py
