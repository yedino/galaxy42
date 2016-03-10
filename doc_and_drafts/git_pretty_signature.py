import sys
import os
import select
import tty
import termios

def readchar(wait_for_char=True):
    old_settings = termios.tcgetattr(sys.stdin)
    tty.setcbreak(sys.stdin.fileno())
    try:
        if wait_for_char or select.select([sys.stdin, ], [], [], 0.0)[0]:
            char = os.read(sys.stdin.fileno(), 1)
            return char if type(char) is str else char.decode()
    finally:
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_settings)

from subprocess import check_output

i=0
ch='t'

while (ch != 'q'):
    data_com = 'git log -1 --skip='+str(i)+' --pretty=format:"%H:%G?"'
    data = check_output(data_com.split())

    del_pos = data.find(':')
    com_hash = data[1:del_pos]
    sig_stat = data[del_pos+1:-1]

    com = "git log -1 "+com_hash+" --pretty=format:\"%C(yellow)%H\n"
    sig_info_com = "git log "+com_hash+" -1 --pretty=format:\"%GG\""
    sig_info = check_output(sig_info_com.split())
    sig_info = repr(sig_info).replace('"','\\"')[2:-2]
    sig_info = sig_info.replace('\\n','\n')
    if sig_stat == 'G':
        com += "%C(green)SIGNATURE - GOOD\n%C(green)"
        com += sig_info.replace('\n','\n%C(green)')[1:-1]
    elif sig_stat == 'U':
        com += "%C(cyan)SIGNATURE - UNTRUSTED\n%C(cyan)"
        com += sig_info.replace('\n','\n%C(cyan)')[1:-1]
    elif sig_stat == 'B':
        com += "%C(magneta)SIGNATURE - BAD SIGN\n%C(magneta)"
        com += sig_info.replace('\n','\n%C(magneta)')[1:-1]
    elif sig_stat == 'N':
        com += "%C(red)! WARNING !\n%C(red)SIGNATURE - NOT SIGNED\n%C(red)! WARNING !\n"
        com += sig_info.replace('\n','\n%C(red)')[1:-1] # empty
    com += "%C(white)Author: %aN <%aE>\n%C(white)Date: %aD\n\n    %B\n\""
    os.system(com)
    sys.stdout.write(':')
    sys.stdout.flush()
    ch = readchar()
    sys.stdout.write('\b')
    i+=1

