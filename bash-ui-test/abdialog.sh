#!/bin/bash
# This code (in this file) is on BSD 2-clause license, (c) Copyrighted 2016 tigusoft.pl
# abdialog - Any Bash Dialog
abdialog_ver=1

abdialog_program="whiptail" ; abdialog_curses=1 ; abdialog_gui=0 ;

if [ -x $(which dialog) ] ; then abdialog_program="dialog" ; fi

function abdialog() {
	$abdialog_program "$@"
}

function abdialog_exit() {
	clear
	if [[ ! -z "$1" ]] ; then
		exit "$1"
	else
		exit
	fi
}
