import sys
import os
import argparse
from collections import deque
from subprocess import check_output, call, Popen, PIPE, CalledProcessError, STDOUT
import locale
import subprocess

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


def totag_stats(pager, encoding, check_mode = False):
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
            pager.stdin.write('exit(1)'.encode(encoding))
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

        col_end = bcolors.ENDC
        col_tag = bcolors.OKGREEN

        tag_pos = data.find("tag")
        if tag_pos != -1 or not data:
            # getting tag from %d ref names
            if tag_pos != -1:
                tag_info = data[tag_pos:-1]
                tag_end_pos = tag_info.find(',')
                tag_info = tag_info[:tag_end_pos]

                # last tag checking
                if sig_stat == 'G':
                    G_totag-=1
                if sig_stat == 'U':
                    U_totag-=1
                    col_tag = bcolors.WARNING
                    string = col_tag+"WARNING: Last tag have untrusted sign!"+col_end+'\n'
                    pager.stdin.write(string.encode(encoding))
                    if check_mode:
                        exit(1)
                if sig_stat == 'B':
                    B_totag-=1
                    col_tag = bcolors.FAIL
                    string = col_tag+"ERROR: Last tag have bad sign!!"+col_end+'\n'
                    pager.stdin.write(string.encode(encoding))
                    if check_mode:
                        exit(1)
                if sig_stat == 'N':
                    N_totag-=1
                    col_tag = bcolors.FAIL
                    string = col_tag+"ERROR: Last tag is not signed!!"+col_end+'\n'
                    pager.stdin.write(string.encode(encoding))
                    if check_mode:
                        exit(1)

            col_coms = bcolors.OKGREEN
            # commits checking
            if U_totag:
                col_coms = bcolors.OKBLUE
                string = col_coms+"NOTICE: At least one commit with untrusted sign!"+col_end+'\n'
                pager.stdin.write(string.encode(encoding))
                if check_mode:
                    exit(1)
            if B_totag:
                col_coms = bcolors.FAIL
                string = col_coms+"ERROR: At least one commit with bad sign!"+col_end+'\n'
                pager.stdin.write(string.encode(encoding))
                if check_mode:
                    exit(1)
            if N_totag:
                col_coms = bcolors.WARNING
                string = col_coms+"WARNING: At leas one not signed commit!!"+col_end+'\n'
                pager.stdin.write(string.encode(encoding))
                if check_mode:
                    exit(1)

            info_head=""
            info_ft=""
            if tag_pos != -1:
                info_head = col_coms+"Signatures status after "+col_tag+tag_info+":\n"
                if col_tag == bcolors.OKGREEN and (col_coms == bcolors.OKGREEN or col_coms == bcolors.OKBLUE):
                    info_ft = col_coms+"Everything SINCE LAST TAG "+tag_info[5:]+" seems correctly signed\n"
                else:
                    info_ft = col_coms+"Occur at least one problem with signatures SINCE LAST TAG "+tag_info[5:]+'\n'
            else:
                info_head = col_coms+"Signatures status for all commits:\n"
                info_ft = bcolors.OKBLUE+"NOTICE: No tag found in this repository\n"

            info_str = info_head                                    \
                        +col_coms+str(G_totag)+": GOOD\n"           \
                        +col_coms+str(U_totag)+": UNTRUSTED\n"      \
                        +col_coms+str(B_totag)+": BAD SIGN\n"       \
                        +col_coms+str(N_totag)+": NOT SIGNED\n"     \
                        +col_coms+info_ft+col_end+'\n'
            pager.stdin.write(info_str.encode(encoding))
            return
        i+=1

def is_git_directory(path = '.'):
    return call(['git', '-C', path, 'branch'], stderr=STDOUT, stdout=open(os.devnull, 'w')) == 0

if __name__ == "__main__":

    if not is_git_directory():
        print("Exit: This directory does not contain a git repository!")
        exit(1)

    try:
        encoding = locale.getdefaultlocale()[1]
        pager = Popen(['less', '-F', '-R', '-S', '-X', '-K'], stdin=PIPE, stdout=sys.stdout)

        parser = argparse.ArgumentParser(description='Display git log with gpg signatures in pretty colored format')
        parser.add_argument('-c','--check',action="store_true",
                            help='running check mode, exit with 1 status if occurs warning or error')

        args = parser.parse_args()
        if args.check:
            totag_stats(pager, encoding, True)
        else:
            totag_stats(pager, encoding)

        i=0
        while (True):
            data_com = 'git log -1 --skip='+str(i)+' --pretty=format:"%H:%G?%d"'

            try:
                data = check_output(data_com.split()).decode(encoding)
            except CalledProcessError as e:
                pager.stdin.write('exit(1)'.encode(encoding))
                exit(1)


            if not data:
                break     # end of commits success exit

            del_pos = data.find(':')
            com_hash = data[1:del_pos]
            sig_stat = data[del_pos+1:del_pos+2]

            com = "git||log||-1||"+com_hash+"||--pretty=format:\"%C(yellow)%H%C(auto)%d\n"
            sig_info = ""
            ref_info = ""
            sig_info_com = "git log "+com_hash+" -1 --pretty=format:\"%GG\""
            ref_info_com = "git log "+com_hash+" -1 --pretty=format:\"%d\""

            if (sys.version_info > (3, 0)):
                sig_info = check_output(sig_info_com.split()).decode(encoding)
                ref_info = check_output(ref_info_com.split()).decode(encoding)
            else:
                sig_info = check_output(sig_info_com.split())
                ref_info = check_output(ref_info_com.split())

            tag_pos = ref_info.find("tag")
            # getting tag from %d ref names
            if tag_pos != -1:
                tag_info = ref_info[tag_pos:-1]
                tag_end_pos = tag_info.find(',')
                tag_info = tag_info[:tag_end_pos]
                info_len = len(com_hash) + len(ref_info) - len(tag_info) - 15
                com += "%C(yellow reverse)=== "+tag_info+" is here " + '='*info_len+'\n'

            sig_info = repr(sig_info).replace('"','\\"')[2:-2]
            sig_info = sig_info.replace('\\n','\n')
            sig_info = sig_info.replace('\\\"','\"')
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
                if (sys.version_info > (3, 0)):
                    data = check_output(com.split("||")).decode(encoding)
                    data = data[1:-1]
                else:
                    data = check_output(com.split("||"))
                    data = data[1:-1]

                pager.stdin.write(data.encode(encoding))

            except CalledProcessError as e:
                pager.stdin.write('exit(1)'.encode(encoding))
                exit(1)

            i+=1

        pager.stdin.close()
        pager.wait()

    except IOError as e:
        if e.errno == 32:  # catch q quit from less
            pass
        else:
            print("Unexpected error: " + os.strerror(e.errno))
            raise

    except KeyboardInterrupt:
        # less should handle this
        pass
