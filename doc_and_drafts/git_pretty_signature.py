import sys
import os
import select
import tty
import termios
import argparse
from collections import deque
from subprocess import check_output, call, CalledProcessError, STDOUT
import locale

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


##Returns the terminal size WxH
#Found on http://stackoverflow.com/a/566752/2646228
def getTerminalSize():
    import os
    env = os.environ
    def ioctl_GWINSZ(fd):
        try:
            import fcntl, termios, struct, os
            cr = struct.unpack('hh', fcntl.ioctl(fd, termios.TIOCGWINSZ,
        '1234'))
        except:
            return
        return cr
    cr = ioctl_GWINSZ(0) or ioctl_GWINSZ(1) or ioctl_GWINSZ(2)
    if not cr:
        try:
            fd = os.open(os.ctermid(), os.O_RDONLY)
            cr = ioctl_GWINSZ(fd)
            os.close(fd)
        except:
            pass
    if not cr:
        cr = (env.get('LINES', 25), env.get('COLUMNS', 80))
    return int(cr[1]), int(cr[0])


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
    encoding = locale.getdefaultlocale()[1]
    i = 0
    ch = 't'
    G_totag = 0
    U_totag = 0
    B_totag = 0
    N_totag = 0
    while (ch != 'q'):
        data_com = 'git log -1 --skip='+str(i)+' --pretty=format:"%G?%d"'

        try:
            data = check_output(data_com.split()).decode(encoding)
        except CalledProcessError as e:
            print("exit(1)")
            exit(1)

        sig_stat = data[1:2]
        if sig_stat == 'G':
            G_totag+=1
        elif sig_stat == 'U':
            U_totag+=1
        elif sig_stat == 'B':
            B_totag+=1
        elif sig_stat == 'N':
            N_totag+=1

        if data.find("tag") != -1 or not data:
            tag_info = data[2:-1]
            col_tag = bcolors.OKGREEN
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
            info_head=""
            if not data:
                info_head = col_coms+"Signatures status for all commits:\n"
            else:
                info_head = col_coms+"Signatures status after "+col_tag+tag_info+"\n"
            info_str = info_head+col_coms+str(G_totag)+": GOOD\n"+str(U_totag)+": UNTRUSTED\n"+str(B_totag)+": BAD SIGN\n"+str(N_totag)+": NOT SIGNED"+col_end
            print(info_str)
            return
        i+=1

def is_git_directory(path = '.'):
    return call(['git', '-C', path, 'branch'], stderr=STDOUT, stdout=open(os.devnull, 'w')) == 0

def buffor_popleft_n(buffor, lines):
    for line in range(lines):
        print(buffor.popleft())

def buffor_popleft_all(buffor):
    while buffor:
        print(buffor.popleft())

def buffor_popleft_singly(buffor):
    ch = 't'
    while buffor:
        print(buffor.popleft())
        sys.stdout.write(':')
        sys.stdout.flush()
        ch = readchar()
        sys.stdout.write('\b')
        sys.stdout.write("\033[K")
        sys.stdout.flush()
        if ch == 'q':
            return 'q'

def wait_for_quit():
    ch = 't'
    while ch != 'q':
        sys.stdout.write('\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b')
        sys.stdout.write("\033[K") # Clear to the end of line
        # sys.stdout.write("\033[F") # Cursor up one line
        sys.stdout.write("(END) type q for quit")
        sys.stdout.flush()
        ch = readchar()
    sys.stdout.write('\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b')
    sys.stdout.write("\033[K")


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

    encoding = locale.getdefaultlocale()[1]
    i=0
    ch='t'

    teminal_height = 20
    if (sys.version_info > (3, 0)):
        terminal_height = os.get_terminal_size().lines
    else:
        terminal_height = getTerminalSize()[1]

    terminal_height -= 9 # for status // TODO Load status to buffer too
    first_lines_printed = False
    buffor = deque([])
    while (ch != 'q'):
        data_com = 'git log -1 --skip='+str(i)+' --pretty=format:"%H:%G?%d"'

        try:
            data = check_output(data_com.split()).decode(encoding)
        except CalledProcessError as e:
            print("exit(1)")
            exit(1)

        if not first_lines_printed and len(buffor) >= terminal_height:
            buffor_popleft_n(buffor,terminal_height)
            first_lines_printed = True
        if first_lines_printed and buffor and buffor_popleft_singly(buffor) == 'q':
            break

        if not data:
            if len(buffor) <= terminal_height and not first_lines_printed:
                print("printed?:" + str(first_lines_printed))
                buffor_popleft_all(buffor)
            elif len(buffor) >= terminal_height and not first_lines_printed:
                buffor_popleft_n(buffor,terminal_height)

            if buffor_popleft_singly(buffor) != 'q':
                wait_for_quit()

            break     # end of commits success exit

        del_pos = data.find(':')
        com_hash = data[1:del_pos]
        sig_stat = data[del_pos+1:del_pos+2]

        com = "git||log||-1||"+com_hash+"||--pretty=format:\"%C(yellow)%H%C(auto)%d\n"
        sig_info_com = "git log "+com_hash+" -1 --pretty=format:\"%GG\""

        if (sys.version_info > (3, 0)):
            sig_info = check_output(sig_info_com.split()).decode(encoding)
        else:
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

        try:
            #print(com.split("||"))
            if (sys.version_info > (3, 0)):
                data = check_output(com.split("||")).decode(encoding)
                data = data[1:-2]
            else:
                data = check_output(com.split("||"))
                data = data[1:-2]

            data_lines = data.split('\n')
            for line in data_lines:
                buffor.append(line)
        except CalledProcessError as e:
            print("exit(1)")
            exit(1)

        #sys.stdout.write(':')
        #sys.stdout.flush()
        #ch = readchar()
        #sys.stdout.write('\b')
        i+=1

    #print("end buffor_size: "+str(len(buffor)));
    #for line in buffor:
    #    print(line)
