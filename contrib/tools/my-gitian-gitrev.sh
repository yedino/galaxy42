#!/bin/bash
git log --format="%H" -1 || { echo "(error: can not detect gitrev. pwd=$PWD)" ; exit 1; }
