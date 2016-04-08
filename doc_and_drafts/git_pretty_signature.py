import sys
import os
import select
import tty
import termios
import argparse
from subprocess import check_output, call, STDOUT

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

    def disable(self):
        self.HEADER = ''
        self.OKBLUE = ''
        self.OKGREEN = ''
        self.WARNING = ''
        self.FAIL = ''
        self.ENDC = ''


def readchar(wait_for_char=True):
    old_settings = termios.tcgetattr(sys.stdin)
    tty.setcbreak(sys.stdin.fileno())
    try:
        if wait_for_char or select.select([sys.stdin, ], [], [], 0.0)[0]:
            char = os.read(sys.stdin.fileno(), 1)
            return char if type(char) is str else char.decode()
    finally:
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_settings)


def print_totag_stats(check_mode=False):
    i = 0
    ch = 't'
    G_totag = 0
    U_totag = 0
    B_totag = 0
    N_totag = 0
    while (ch != 'q'):
        data_com = 'git log -1 --skip='+str(i)+' --pretty=format:"%G?%d"'
        data = check_output(data_com.split())
        sig_stat = data[1:2]
        if sig_stat == 'G':
            G_totag+=1
        elif sig_stat == 'U':
            U_totag+=1
        elif sig_stat == 'B':
            B_totag+=1
        elif sig_stat == 'N':
            N_totag+=1

        if data.find("tag") != -1:
            tag_info = data[2:-1]
            col_str = bcolors.OKGREEN
            col_end = bcolors.ENDC
            # last tag checking
            if sig_stat == 'G':
                G_totag-=1
            if sig_stat == 'U':
                U_totag-=1
                col_tag = bcolors.WARNING
                print(col_tag+"WARNING: Last tag have untrusted sign!"+col_end)
                if check_mode:
                    exit(1)
            if sig_stat == 'B':
                B_totag-=1
                col_tag = bcolors.FAIL
                print(col_tag+"ERROR: Last tag have bad sign!!"+col_end)
                if check_mode:
                    exit(1)
            if sig_stat == 'N':
                N_totag-=1
                col_tag = bcolors.FAIL
                print(col_tag+"ERROR: Last tag is not signed!!"+col_end)
                if check_mode:
                    exit(1)
            col_coms = bcolors.OKGREEN
            # commits checking
            if U_totag:
                col_coms = bcolors.OKBLUE
                print(col_coms+"NOTICE: At least one commit with untrusted sign!"+col_end)
                if check_mode:
                    exit(1)
            if B_totag:
                col_coms = bcolors.FAIL
                print(col_coms+"ERROR: At least one commit with bad sign!"+col_end)
                if check_mode:
                    exit(1)
            if N_totag:
                col_coms = bcolors.WARNING
                print(col_coms+"WARNING: At leas one not signed commit!!"+col_end)
                if check_mode:
                    exit(1)
            info_str = col_coms+"Signatures status after "+col_tag+tag_info+"\n"+col_coms+str(G_totag)+": GOOD\n"+str(U_totag)+": UNTRUSTED\n"+str(B_totag)+": BAD SIGN\n"+str(N_totag)+": NOT SIGNED"+col_end
            print(info_str)
            return
        i+=1

def is_git_directory(path = '.'):
    return call(['git', '-C', path, 'branch'], stderr=STDOUT, stdout=open(os.devnull, 'w')) == 0

if __name__ == "__main__":

    if not is_git_directory():
        print("Exit: This directory does not contain a git repository!")
        exit(1)

    parser = argparse.ArgumentParser(description='Display git log with gpg signatures in pretty colored format')
    parser.add_argument('-c','--check',action="store_true",
                        help='running check mode, exit with 1 status if occurs warning or error')
    args = parser.parse_args()
    if args.check:
        print_totag_stats(True)
    else:
        print_totag_stats()

    i=0
    ch='t'

    while (ch != 'q'):
        data_com = 'git log -1 --skip='+str(i)+' --pretty=format:"%H:%G?%d"'
        data = check_output(data_com.split())

        del_pos = data.find(':')
        com_hash = data[1:del_pos]
        sig_stat = data[del_pos+1:del_pos+2]

        com = "git log -1 "+com_hash+" --pretty=format:\"%C(yellow)%H%C(auto)%d\n"
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

