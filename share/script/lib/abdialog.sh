#!/bin/bash
# This code (in this file) is on BSD 2-clause license, (c) Copyrighted 2016 TiguSoft.pl
# abdialog - Any Bash Dialog
abdialog_ver=1

abdialog_curses=1 ; abdialog_gui=0 ;

abdialog_program="whiptail"
if [ -x "$(which dialog)" ] ; then abdialog_program="dialog" ; fi

if [[ "$FORCE_DIALOG" == "whiptail" ]] ; then abdialog_program="whiptail" ; abdialog_curses=1 ; abdialog_gui=0 ; fi
if [[ "$FORCE_DIALOG" == "dialog" ]] ; then abdialog_program="dialog" ; abdialog_curses=1 ; abdialog_gui=0 ; fi

export abdialog_ver
export abdialog_curses
export abdialog_gui

function abdialog() {
	local argtab=()
	for arg in "$@" ; do
		local argfixed="${arg//$'\n'/\\n}";
		argtab+=("$argfixed")
	done
	$abdialog_program "${argtab[@]}"
}

function abdialog_exit() {
	clear
	if [[ ! -z "$1" ]] ; then
		exit "$1"
	else
		exit
	fi
}
